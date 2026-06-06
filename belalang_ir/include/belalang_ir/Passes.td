#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangConstantsPass : Pass<"constantize"> {
  let summary = "lower belalang.constant to arith.constant";
  let dependentDialects = ["::bir::BIRDialect", "::mlir::arith::ArithDialect"];
}

def BelalangRuntimizePass : Pass<"runtimize"> {
  let summary = "lower belalang ops to calls to runtime functions";
  let dependentDialects = ["::bir::BIRDialect"];
}

#endif // BELALANG_PASSES_TD_
