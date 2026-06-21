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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"

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
  auto main = bir::FuncOp::create(builder, loc, "main",
                                  mlir::FunctionType::get(&context, {}, {retTy}));
  mlir::Block *entry = main.addEntryBlock();
  builder.setInsertionPointToStart(entry);
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

std::unique_ptr<BIRValue> BIRGen::build_constant_string(rust::String val) {
  auto type = builder.getType<bir::StringType>();
  llvm::StringRef value(val.data());
  auto attr = bir::StringAttr::get(&context, type, value);
  auto op = bir::ConstantOp::create(builder, loc, type, attr);
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_add(const BIRValue &lhs,
                                            const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::AddOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_sub(const BIRValue &lhs,
                                            const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::SubOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_mul(const BIRValue &lhs,
                                            const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::MulOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_div(const BIRValue &lhs,
                                            const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::DivOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_mod(const BIRValue &lhs,
                                            const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::ModOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_var_declare(const BIRValue &v, rust::String name) {
  auto nakedType = v.getValue().getType();
  auto refType = bir::RefType::get(&context, nakedType);
  auto op = bir::VarDeclareOp::create(builder, loc, refType, name.c_str());
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

  auto op = bir::VarLoadOp::create(builder, loc, resultType, refValue.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

void BIRGen::build_print(const BIRValue &val) {
  bir::PrintOp::create(builder, loc, val.getValue());
}

void BIRGen::build_return(const BIRValue &val) {
  bir::ReturnOp::create(builder, loc, val.getValue());
}

void BIRGen::build_empty_return() {
  bir::ReturnOp::create(builder, loc, {});
}

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

std::unique_ptr<LLVMGen> BIRGen::llvmgen() {
  return std::make_unique<LLVMGen>(&module);
}

std::unique_ptr<BIRGen> create_birgen() { return std::make_unique<BIRGen>(); }

// -----------------------------------------------------------------------------
// LLVMGen
// -----------------------------------------------------------------------------

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
  raw_fd_ostream dest(out.data(), ec);
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
