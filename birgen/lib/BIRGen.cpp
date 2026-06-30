#include "belalang/BIRGen/BIRGen.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include <optional>

namespace belalang {
namespace birgen {

BIRGen::BIRGen() : builder(&context), loc(builder.getUnknownLoc()) {
  mlir::DialectRegistry registry;
  registry.insert<bir::BIRDialect, mlir::LLVM::LLVMDialect>();
  mlir::registerLLVMDialectTranslation(registry);
  mlir::registerBuiltinDialectTranslation(registry);
  context.appendDialectRegistry(registry);

  context.getOrLoadDialect<bir::BIRDialect>();
  module = mlir::ModuleOp::create(loc);
  builder.setInsertionPointToStart(module.getBody());

  auto retTy = bir::IntType::get(&context);
  auto main = bir::FuncOp::create(
      builder, loc, "main", mlir::FunctionType::get(&context, {}, {retTy}));
  mlir::Block *entry = main.addEntryBlock();
  builder.setInsertionPointToStart(entry);
}

mlir::Type BIRGen::mapType(TypeKind ty) {
  if (ty == TypeKind::String) {
    return bir::StringType::get(&context);
  } else if (ty == TypeKind::Int) {
    return bir::IntType::get(&context);
  } else if (ty == TypeKind::Float) {
    return bir::FloatType::get(&context);
  } else {
    return {};
  }
}

std::unique_ptr<BIRValue> BIRGen::build_constant_int(int64_t val) {
  auto type = builder.getType<bir::IntType>();
  llvm::APInt value(64, val);
  auto attr = bir::IntegerAttr::get(&context, type, value);
  auto op = bir::ConstantOp::create(builder, loc, type, attr);
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_constant_float(double val) {
  auto type = builder.getType<bir::FloatType>();
  llvm::APFloat value(val);
  auto attr = bir::FloatAttr::get(&context, type, value);
  auto op = bir::ConstantOp::create(builder, loc, type, attr);
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_constant_string(rust::Str val) {
  auto type = builder.getType<bir::StringType>();
  llvm::StringRef value(val.data(), val.size());
  auto attr = bir::StringAttr::get(&context, type, value);
  auto op = bir::ConstantOp::create(builder, loc, type, attr);
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_constant_bool(bool val) {
  auto type = builder.getType<bir::BoolType>();
  auto attr = bir::BoolAttr::get(&context, type, val);
  auto op = bir::ConstantOp::create(builder, loc, type, attr);
  return std::make_unique<BIRValue>(op.getResult());
}

template <typename Op>
std::unique_ptr<BIRValue>
build_binop_impl(mlir::OpBuilder &builder, mlir::Location loc,
                 const BIRValue &lhs, const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op = Op::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue>
BIRGen::build_binop(BinOpKind kind, const BIRValue &lhs, const BIRValue &rhs) {
  switch (kind) {
  case BinOpKind::Add:
    return build_binop_impl<bir::AddOp>(builder, loc, lhs, rhs);
  case BinOpKind::Sub:
    return build_binop_impl<bir::SubOp>(builder, loc, lhs, rhs);
  case BinOpKind::Mul:
    return build_binop_impl<bir::MulOp>(builder, loc, lhs, rhs);
  case BinOpKind::Div:
    return build_binop_impl<bir::DivOp>(builder, loc, lhs, rhs);
  case BinOpKind::Mod:
    return build_binop_impl<bir::ModOp>(builder, loc, lhs, rhs);
  default:
    return nullptr;
  }
}

std::unique_ptr<BIRValue> BIRGen::build_var_declare(const BIRValue &v,
                                                    rust::Str name) {
  auto nakedType = v.getValue().getType();
  auto refType = bir::RefType::get(&context, nakedType);
  auto op = bir::VarDeclareOp::create(
      builder, loc, refType, llvm::StringRef(name.data(), name.size()));
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_var_declare_ty(TypeKind v,
                                                       rust::Str name) {
  mlir::Type ty = mapType(v);

  auto refType = bir::RefType::get(&context, ty);
  auto op = bir::VarDeclareOp::create(
      builder, loc, refType, llvm::StringRef(name.data(), name.size()));
  return std::make_unique<BIRValue>(op.getResult());
}

void BIRGen::build_var_store(const BIRValue &v, const BIRValue &ref) {
  auto src = v.getValue();
  auto dest = ref.getValue();
  bir::VarStoreOp::create(builder, loc, src, dest);
}

std::unique_ptr<BIRValue> BIRGen::build_var_load(const BIRValue &refValue) {
  auto refType = dyn_cast<bir::RefType>(refValue.getValue().getType());

  // TODO: error or some kind
  if (!refType)
    return nullptr;

  auto resultType = refType.getEl();

  auto op =
      bir::VarLoadOp::create(builder, loc, resultType, refValue.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRFunctionGuard::get_arg(size_t index) const {
  auto op = fnValue.getDefiningOp();
  auto &region = op->getRegion(0);
  auto &block = region.front();
  auto arg = block.getArgument(index);
  return std::make_unique<BIRValue>(arg);
}

std::unique_ptr<BIRFunctionGuard>
BIRGen::build_fn_expr(TypeKind resultTy, rust::Slice<const TypeKind> paramTys) {
  std::vector<mlir::Type> inputs;
  for (auto ty : paramTys) {
    inputs.push_back(mapType(ty));
  }
  auto fnTy = mlir::FunctionType::get(&context, inputs, {mapType(resultTy)});
  auto op = bir::FuncExprOp::create(builder, loc, fnTy);

  auto guard = std::make_unique<BIRFunctionGuard>(builder, op.getResult());

  std::vector<mlir::Location> locs(inputs.size(), loc);
  builder.createBlock(&op.getBody(), {}, inputs, locs);
  return guard;
}

void BIRIfGuard::start_then() {
  auto op = mlir::cast<bir::IfOp>(ifOp);
  auto &region = op.getThenRegion();
  region.push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&region.front());
}

void BIRIfGuard::start_else() {
  auto op = mlir::cast<bir::IfOp>(ifOp);
  auto &region = op.getElseRegion();
  region.push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&region.front());
}

std::unique_ptr<BIRValue> BIRIfGuard::get_value() const {
  auto op = mlir::cast<bir::IfOp>(ifOp);
  if (op.getNumResults() > 0)
    return std::make_unique<BIRValue>(op.getResult());
  return nullptr;
}

std::unique_ptr<BIRIfGuard> BIRGen::build_if_expr(const BIRValue &cond) {
  auto op = bir::IfOp::create(builder, loc, mlir::TypeRange{}, cond.getValue());
  return std::make_unique<BIRIfGuard>(builder, op.getOperation());
}

void BIRGen::build_yield(const BIRValue &val) {
  bir::YieldOp::create(builder, loc, val.getValue());
}

void BIRGen::build_empty_yield() {
  bir::YieldOp::create(builder, loc);
}

void BIRGen::build_print(const BIRValue &val) {
  bir::PrintOp::create(builder, loc, val.getValue());
}

void BIRGen::start_call(const BIRValue &callee) {
  current_callee = callee.getValue();
  current_args.clear();
}

void BIRGen::add_call_arg(const BIRValue &arg) {
  current_args.push_back(arg.getValue());
}

std::unique_ptr<BIRValue> BIRGen::finish_call() {
  auto fnType = mlir::cast<mlir::FunctionType>(current_callee.getType());
  auto resultTypes = fnType.getResults();

  std::vector<mlir::Value> operands;
  operands.push_back(current_callee);
  operands.insert(operands.end(), current_args.begin(), current_args.end());

  auto op = bir::CallIndirectOp::create(builder, loc, resultTypes, operands);
  if (resultTypes.empty())
    return nullptr;
  return std::make_unique<BIRValue>(op.getResult(0));
}

void BIRGen::build_return(const BIRValue &val) {
  bir::ReturnOp::create(builder, loc, val.getValue());
}

void BIRGen::build_empty_return() { bir::ReturnOp::create(builder, loc, {}); }

void BIRGen::build_main_return() {
  auto typ = bir::IntType::get(&context);
  auto val = llvm::APInt(64, 0); // return 0
  auto atr = bir::IntegerAttr::get(&context, typ, val);
  auto ret = bir::ConstantOp::create(builder, loc, typ, atr);
  bir::ReturnOp::create(builder, loc, {ret});
}

void BIRGen::dump() const { const_cast<mlir::ModuleOp &>(module).dump(); }

rust::String BIRGen::dump_to_string() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  const_cast<mlir::ModuleOp &>(module).print(os);
  return rust::String(os.str());
}

bool BIRGen::optimize() {
  mlir::PassManager pm(&context);
  bir::buildBIRLoweringPipeline(pm);
  return mlir::succeeded(pm.run(module));
}

std::unique_ptr<BIRGen> create_birgen() { return std::make_unique<BIRGen>(); }

// -----------------------------------------------------------------------------
// LLVMGen
// -----------------------------------------------------------------------------

std::unique_ptr<LLVMGen> create_llvmgen(BIRGen &gen) {
  return std::make_unique<LLVMGen>(&gen.module);
}

LLVMGen::LLVMGen(mlir::ModuleOp *op) {
  mlir::PassManager pm(op->getContext());
  pm.addPass(bir::createBelalangBIRToLLVMPass());
  assert(mlir::succeeded(pm.run(*op)));

  auto llvmModule = mlir::translateModuleToLLVMIR(*op, context);
  assert(llvmModule);

  auto triple = Triple(sys::getDefaultTargetTriple());
  llvmModule->setTargetTriple(triple);

  this->module = std::move(llvmModule);
}

rust::String LLVMGen::compile_object_file(rust::String out) const {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  auto triple = Triple(sys::getDefaultTargetTriple());
  module->setTargetTriple(triple);

  std::string error;
  auto target = TargetRegistry::lookupTarget(triple, error);
  if (!target)
    return rust::String("failed to lookup target");

  auto cpu = "generic";
  auto features = "";
  TargetOptions opt;
  auto rm = llvm::Reloc::PIC_;

  auto tm = target->createTargetMachine(triple, cpu, features, opt, rm);
  module->setDataLayout(tm->createDataLayout());

  std::error_code ec;
  raw_fd_ostream dest(llvm::StringRef(out.data(), out.size()), ec);
  if (ec)
    return rust::String("failed to create destination");

  legacy::PassManager pm;
  if (tm->addPassesToEmitFile(pm, dest, nullptr, CodeGenFileType::ObjectFile))
    return rust::String("failed to add passes");

  pm.run(*module);

  dest.flush();
  return rust::String();
}

rust::String LLVMGen::dump_to_string() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  module->print(os, nullptr);
  return rust::String(os.str());
}

} // namespace birgen
} // namespace belalang
