#include "belalang/BIR/IR/Builder.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/Pass/PassManager.h"
#include "llvm/Support/raw_ostream.h"

namespace belalang {
namespace bir {

BIRBuilder::BIRBuilder() : builder(&context), loc(builder.getUnknownLoc()) {
  context.getOrLoadDialect<bir::BIRDialect>();
  module = mlir::ModuleOp::create(loc);
  builder.setInsertionPointToStart(module.getBody());
}

std::unique_ptr<BIRValue> BIRBuilder::build_constant_int(int64_t val) {
  auto type = builder.getType<bir::IntType>();
  auto op = bir::ConstantOp::create(builder, loc, type,
                                    builder.getI32IntegerAttr(val));
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_constant_float(double val) {
  auto type = builder.getType<bir::FloatType>();
  auto op =
      bir::ConstantOp::create(builder, loc, type, builder.getF64FloatAttr(val));
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_add(const BIRValue &lhs,
                                                const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::AddOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_sub(const BIRValue &lhs,
                                                const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::SubOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_mul(const BIRValue &lhs,
                                                const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::MulOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_div(const BIRValue &lhs,
                                                const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::DivOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRBuilder::build_mod(const BIRValue &lhs,
                                                const BIRValue &rhs) {
  auto type = lhs.getValue().getType();
  auto op =
      bir::ModOp::create(builder, loc, type, lhs.getValue(), rhs.getValue());
  return std::make_unique<BIRValue>(op.getResult());
}

void BIRBuilder::build_print(const BIRValue &val) {
  bir::PrintOp::create(builder, loc, val.getValue());
}

void BIRBuilder::dump() const { const_cast<mlir::ModuleOp &>(module).dump(); }

rust::String BIRBuilder::dump_to_string() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  const_cast<mlir::ModuleOp &>(module).print(os);
  return rust::String(os.str());
}

bool BIRBuilder::optimize() {
  mlir::PassManager pm(&context);
  // TODO: change this with actual optimizers.
  pm.addPass(createBelalangRuntimizePass());
  return mlir::succeeded(pm.run(module));
}

std::unique_ptr<BIRBuilder> create_builder() {
  return std::make_unique<BIRBuilder>();
}

} // namespace bir
} // namespace belalang
