#ifndef MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_
#define MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCIRPrepareGCSafepointsPass : Pass<"gcir-prepare-gc-safepoints"> {
  let dependentDialects = ["::mlir::gc::GCDialect"];
}

def GCLowerAllocationsPass : Pass<"gc-lower-allocations"> {
  let summary = "lower GC allocations to GC calls";

  let options = [
    Option<"allocator", "alloc-function", "std::string", "\"malloc\"",
           "allocator function to call for gc.alloc">,
  ];

  let dependentDialects = ["::mlir::gc::GCDialect"];
}

#endif // MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_
