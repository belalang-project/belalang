#ifndef MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_
#define MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCIRPrepareGCSafepointsPass : Pass<"gcir-prepare-gc-safepoints"> {
  let dependentDialects = ["::mlir::gc::GCDialect"];
}

#endif // MLIR_DIALECT_GC_TRANSFORMS_PASSES_TD_
