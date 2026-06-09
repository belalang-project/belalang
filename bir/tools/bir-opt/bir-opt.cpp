#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;

  registry.insert<mlir::arith::ArithDialect, mlir::func::FuncDialect,
                  belalang::bir::BIRDialect>();

  belalang::bir::registerPasses();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "Belalang IR analysis and optimization tool\n", registry));
}
