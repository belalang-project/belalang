#ifndef BELALANG_GCIR_TRANSFORMS_PASSES_TD_
#define BELALANG_GCIR_TRANSFORMS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def GCIRPrepareGCSafepointsPass : Pass<"gcir-prepare-gc-safepoints"> {
  let dependentDialects = ["::belalang::gc::GCIRDialect"];
}

#endif // BELALANG_GCIR_TRANSFORMS_PASSES_TD_
