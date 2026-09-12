#ifndef MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
#define MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCToLLVMPass : Pass<"convert-gcir-to-llvm"> {
  let summary = "lower gcir allocations to LLVM";

  let options = [
    Option<"allocator", "alloc-function", "std::string", "\"malloc\"",
           "allocator function with signature ptr(i64)">
  ];

  let dependentDialects = ["::mlir::gc::GCDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // MLIR_DIALECT_GC_CONVERSIONS_PASSES_TD_
