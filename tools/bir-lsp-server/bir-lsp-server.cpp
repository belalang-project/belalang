#include "belalang/BIR/IR/BIR.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Tools/mlir-lsp-server/MlirLspServerMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<belalang::bir::BIRDialect, mlir::gc::GCDialect,
                  mlir::cf::ControlFlowDialect, mlir::func::FuncDialect,
                  mlir::LLVM::LLVMDialect, mlir::arith::ArithDialect>();

  return mlir::failed(mlir::MlirLspServerMain(argc, argv, registry));
}
