#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangLowerToRuntimeCallsPass : Pass<"bir-lower-to-runtime-calls"> {
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

def BelalangLowerDeclToMemoryPass : Pass<"bir-lower-decl-to-memory"> {
  let summary = "lowers declare ops to memory";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangPrepareGCAllocationsPass : Pass<"bir-prepare-gc-allocations"> {
  let summary = "attach live roots and relocated results to heap allocations";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangVerifyLoweredFormPass : Pass<"bir-verify-lowered-form"> {
  let summary = "verify BIR contains only ops expected after lowering";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect"];
}

def BelalangOptimizeStructLayoutPass : Pass<"bir-optimize-struct-layout"> {
  let summary = "compute compact physical layouts for BIR structs";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

def BelalangBIRToLLVMPass : Pass<"convert-bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_PASSES_TD_
