#include "belalang/BIR/CodeGen/BIRGen.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"

#include <memory>

namespace belalang {
namespace bir {
namespace codegen {

#include "belalang/BIR/CodeGen/Bindings.cpp.inc"

BIRGen::BIRGen() : builder(&context), loc(builder.getUnknownLoc()) {
  // Load dialects.
  mlir::DialectRegistry registry;
  registry.insert<bir::BIRDialect, mlir::LLVM::LLVMDialect>();
  context.appendDialectRegistry(registry);
  context.getOrLoadDialect<bir::BIRDialect>();

  // Create the module.
  module = mlir::ModuleOp::create(loc);
  builder.setInsertionPointToStart(module.getBody());

  // Create the main function.
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
  } else if (ty == TypeKind::Bool) {
    return bir::BoolType::get(&context);
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

std::unique_ptr<BIRValue> build_binop_cmp_impl(mlir::OpBuilder &builder,
                                               mlir::Location loc,
                                               bir::CmpOpKind kind,
                                               const BIRValue &lhs,
                                               const BIRValue &rhs) {
  auto op =
      bir::CmpOp::create(builder, loc, kind, lhs.getValue(), rhs.getValue());
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
  case BinOpKind::Lt:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::lt, lhs, rhs);
  case BinOpKind::Le:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::le, lhs, rhs);
  case BinOpKind::Gt:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::gt, lhs, rhs);
  case BinOpKind::Ge:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::ge, lhs, rhs);
  case BinOpKind::Eq:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::eq, lhs, rhs);
  case BinOpKind::Ne:
    return build_binop_cmp_impl(builder, loc, bir::CmpOpKind::ne, lhs, rhs);
  default:
    return nullptr;
  }
}

std::unique_ptr<BIRValue> BIRGen::build_var_declare(const BIRValue &v,
                                                    rust::Str name) {
  auto nakedType = v.getValue().getType();
  auto refType = bir::RefType::get(&context, nakedType);
  auto op = bir::DeclareOp::create(
      builder, loc, refType, llvm::StringRef(name.data(), name.size()));
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_var_declare_ty(TypeKind v,
                                                       rust::Str name) {
  mlir::Type ty = mapType(v);

  auto refType = bir::RefType::get(&context, ty);
  auto op = bir::DeclareOp::create(
      builder, loc, refType, llvm::StringRef(name.data(), name.size()));
  return std::make_unique<BIRValue>(op.getResult());
}

void BIRGen::build_var_store(const BIRValue &v, const BIRValue &ref) {
  auto src = v.getValue();
  auto dest = ref.getValue();
  bir::StoreOp::create(builder, loc, src, dest);
}

std::unique_ptr<BIRValue> BIRGen::build_var_load(const BIRValue &refValue) {
  auto refType = dyn_cast<bir::RefType>(refValue.getValue().getType());

  // TODO: error or some kind
  if (!refType)
    return nullptr;

  auto resultType = refType.getReferent();

  auto op =
      bir::VarLoadOp::create(builder, loc, resultType, refValue.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRFunctionGuard::get_arg(size_t index) const {
  auto &block = bodyRegion->front();
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

  auto guard = std::make_unique<BIRFunctionGuard>(builder, op.getResult(),
                                                  &op.getBody());

  std::vector<mlir::Location> locs(inputs.size(), loc);
  builder.createBlock(&op.getBody(), {}, inputs, locs);
  return guard;
}

BIRIfGuard::~BIRIfGuard() {
  // TODO: handle errors, for now, it won't error
  assert(bir::IfOp::ensureRegionTerm(builder, *thenRegion).succeeded());
  assert(bir::IfOp::ensureRegionTerm(builder, *elseRegion).succeeded());
}

void BIRIfGuard::start_then() {
  thenRegion->push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&thenRegion->front());
}

void BIRIfGuard::start_else() {
  elseRegion->push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&elseRegion->front());
}

std::unique_ptr<BIRValue> BIRIfGuard::get_value() const {
  if (resultValue)
    return std::make_unique<BIRValue>(resultValue);
  return nullptr;
}

std::unique_ptr<BIRIfGuard> BIRGen::build_if_expr(const BIRValue &cond) {
  auto op = bir::IfOp::create(builder, loc, mlir::TypeRange{}, cond.getValue());
  mlir::Value result = op.getNumResults() > 0 ? op.getResult() : mlir::Value();
  return std::make_unique<BIRIfGuard>(builder, &op.getThenRegion(),
                                      &op.getElseRegion(), result);
}

std::unique_ptr<BIRIfGuard> BIRGen::build_if_expr_ty(const BIRValue &cond,
                                                     TypeKind resultTy) {
  mlir::Type ty = mapType(resultTy);
  auto op =
      bir::IfOp::create(builder, loc, mlir::TypeRange{ty}, cond.getValue());
  mlir::Value result = op.getNumResults() > 0 ? op.getResult() : mlir::Value();
  return std::make_unique<BIRIfGuard>(builder, &op.getThenRegion(),
                                      &op.getElseRegion(), result);
}

void BIRWhileGuard::start_cond() {
  condRegion->push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&condRegion->front());
}

void BIRWhileGuard::start_body() {
  bodyRegion->push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&bodyRegion->front());
}

std::unique_ptr<BIRWhileGuard> BIRGen::build_while_stmt() {
  auto op = bir::WhileOp::create(builder, loc);
  return std::make_unique<BIRWhileGuard>(builder, &op.getCond(), &op.getBody());
}

void BIRScopeGuard::start_body() {
  scopeRegion->push_back(new mlir::Block());
  builder.setInsertionPointToEnd(&scopeRegion->front());
}

std::unique_ptr<BIRScopeGuard> BIRGen::build_block_expr() {
  auto op = bir::ScopeOp::create(builder, loc, mlir::Type{});
  return std::make_unique<BIRScopeGuard>(builder, &op.getScopeRegion(), mlir::Value());
}

std::unique_ptr<BIRScopeGuard> BIRGen::build_block_expr_ty(TypeKind resultTy) {
  mlir::Type ty = mapType(resultTy);
  auto op = bir::ScopeOp::create(builder, loc, ty);
  mlir::Value result = op.getNumResults() > 0 ? op.getResult(0) : mlir::Value();
  return std::make_unique<BIRScopeGuard>(builder, &op.getScopeRegion(), result);
}

std::unique_ptr<BIRValue> BIRScopeGuard::get_value() const {
  if (resultValue)
    return std::make_unique<BIRValue>(resultValue);
  return nullptr;
}

void BIRGen::build_condition(const BIRValue &cond) {
  bir::ConditionOp::create(builder, loc, cond.getValue());
}

void BIRGen::build_continue() { bir::ContinueOp::create(builder, loc); }

void BIRGen::build_break() { bir::BreakOp::create(builder, loc); }

void BIRGen::build_yield(const BIRValue &val) {
  bir::YieldOp::create(builder, loc, val.getValue());
}

void BIRGen::build_empty_yield() { bir::YieldOp::create(builder, loc); }

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

bool BIRGen::run_lowering_pipeline() {
  mlir::PassManager pm(&context);
  bir::buildBIRLoweringPipeline(pm);
  return mlir::succeeded(pm.run(module));
}

std::unique_ptr<BIRGen> create_birgen() { return std::make_unique<BIRGen>(); }

} // namespace codegen
} // namespace bir
} // namespace belalang
