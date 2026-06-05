#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangConstantsPass : Pass<"constantize"> {
  let summary = "lower belalang.constant to arith.constant";
  let dependentDialects = ["::bir::BIRDialect", "::mlir::arith::ArithDialect"];
}

#endif // BELALANG_PASSES_TD_
