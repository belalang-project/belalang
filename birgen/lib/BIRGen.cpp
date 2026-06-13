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
#include "llvm/Support/raw_ostream.h"

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

  auto main = bir::FuncOp::create(builder, loc, "main",
                                  mlir::FunctionType::get(&context, {}, {}));
  mlir::Block *entry = main.addEntryBlock();
  builder.setInsertionPointToStart(entry);
}

std::unique_ptr<BIRValue> BIRGen::build_constant_int(int64_t val) {
  auto type = builder.getType<bir::IntType>();
  auto op = bir::ConstantOp::create(builder, loc, type,
                                    builder.getI32IntegerAttr(val));
  return std::make_unique<BIRValue>(op.getResult());
}

std::unique_ptr<BIRValue> BIRGen::build_constant_float(double val) {
  auto type = builder.getType<bir::FloatType>();
  auto op =
      bir::ConstantOp::create(builder, loc, type, builder.getF64FloatAttr(val));
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

void BIRGen::build_print(const BIRValue &val) {
  bir::PrintOp::create(builder, loc, val.getValue());
}

void BIRGen::build_return(const BIRValue &val) {
  bir::ReturnOp::create(builder, loc, val.getValue());
}

void BIRGen::build_empty_return() {
  bir::ReturnOp::create(builder, loc, {});
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
  // TODO: change this with actual optimizers.
  pm.addPass(bir::createBelalangRuntimizePass());
  return mlir::succeeded(pm.run(module));
}

bool BIRGen::lower_to_llvm_dialect() {
  mlir::PassManager pm(&context);
  pm.addPass(bir::createBelalangBIRToLLVMPass());
  return mlir::succeeded(pm.run(module));
}

std::unique_ptr<BIRGen> create_birgen() { return std::make_unique<BIRGen>(); }

rust::String BIRGen::translateToLLVMIR() {
  llvm::LLVMContext llvmContext;
  auto llvmModule = mlir::translateModuleToLLVMIR(module, llvmContext);
  if (!llvmModule)
    return rust::String();

  std::string s;
  llvm::raw_string_ostream os(s);
  llvmModule->print(os, nullptr);
  return rust::String(os.str());
}

} // namespace birgen
} // namespace belalang
