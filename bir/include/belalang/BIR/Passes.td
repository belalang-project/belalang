#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangRuntimizePass : Pass<"runtimize"> {
  let summary = "lower belalang ops to calls to runtime functions";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangLowerFuncExprPass : Pass<"bir-lower-func-expr"> {
  let summary = "replace func_expr with constant";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangBIRToLLVMPass : Pass<"bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_PASSES_TD_
