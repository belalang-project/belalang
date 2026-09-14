#ifndef BELALANG_CONVERSIONS_PASSES_TD_
#define BELALANG_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangBIRToLLVMPass : Pass<"convert-bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect",
                           "::mlir::gc::GCDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

def BelalangBIRToGCPass : Pass<"convert-bir-to-gc"> {
  let summary = "converts BIR ops to GC ops";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::gc::GCDialect"];
}

#endif // BELALANG_CONVERSIONS_PASSES_TD_
