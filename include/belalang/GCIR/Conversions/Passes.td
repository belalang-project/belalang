#ifndef BELALANG_GCIR_CONVERSIONS_PASSES_TD_
#define BELALANG_GCIR_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangGCIRToLLVMPass : Pass<"convert-gcir-to-llvm"> {
  let summary = "lower gcir allocations to LLVM";

  let options = [
    Option<"allocator", "alloc-function", "std::string", "\"malloc\"",
           "allocator function with signature ptr(i64)">
  ];

  let dependentDialects = ["::belalang::gc::GCIRDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_GCIR_CONVERSIONS_PASSES_TD_
