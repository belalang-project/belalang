#ifndef MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
#define MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCToLLVMPass : Pass<"convert-gc-to-llvm"> {
  let summary = "lower GC dialect operations to LLVM";

  let dependentDialects = ["::mlir::gc::GCDialect",
                           "::mlir::arith::ArithDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
