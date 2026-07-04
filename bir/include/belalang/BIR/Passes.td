#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangLowerPrintToRuntimePass : Pass<"bir-lower-print-to-runtime"> {
  let summary = "lower bir print ops to calls to runtime functions";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangFlattenCFGPass : Pass<"bir-flatten-cfg"> {
  let summary = "flatten bir CFGs";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect"];
}

def BelalangLowerFuncExprPass : Pass<"bir-lower-func-expr"> {
  let summary = "replace func_expr with constant";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangPrepareRuntimePass : Pass<"bir-prepare-runtime"> {
  let summary = "Lower ops to calls to runtime library";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangBIRToLLVMPass : Pass<"convert-bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_PASSES_TD_
