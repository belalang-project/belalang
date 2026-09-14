#ifndef MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
#define MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCToLLVMPass : Pass<"convert-gcir-to-llvm"> {
  let summary = "lower GCIR operations to LLVM";

  let dependentDialects = ["::mlir::gc::GCDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
