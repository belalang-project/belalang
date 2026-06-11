#ifndef BELALANG_PASSES_TD_
#define BELALANG_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangConstantsPass : Pass<"constantize"> {
  let summary = "lower belalang.constant to arith.constant";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::arith::ArithDialect"];
}

def BelalangRuntimizePass : Pass<"runtimize"> {
  let summary = "lower belalang ops to calls to runtime functions";
  let dependentDialects = ["::belalang::bir::BIRDialect"];
}

def BelalangBIRToLLVMPass : Pass<"bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_PASSES_TD_
