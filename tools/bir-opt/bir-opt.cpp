#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Transforms/Passes.h"
#include "belalang/BIR/Conversions/Passes.h"
#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/Transforms/Passes.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;

  registry.insert<belalang::bir::BIRDialect, mlir::gc::GCDialect,
                  mlir::cf::ControlFlowDialect, mlir::func::FuncDialect,
                  mlir::LLVM::LLVMDialect>();

  belalang::bir::registerPasses();
  belalang::bir::registerBelalangBIRToLLVMPass();
  belalang::bir::registerBIRPipelines();
  mlir::registerPasses();
  mlir::registerTransformsPasses();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "Belalang IR analysis and optimization tool\n", registry));
}
