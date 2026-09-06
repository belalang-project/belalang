#ifndef BELALANG_CONVERSIONS_PASSES_TD_
#define BELALANG_CONVERSIONS_PASSES_TD_

include "mlir/Pass/PassBase.td"

def BelalangBIRToLLVMPass : Pass<"convert-bir-to-llvm"> {
  let summary = "lower bir instructions to LLVM";
  let dependentDialects = ["::belalang::bir::BIRDialect",
                           "::mlir::cf::ControlFlowDialect",
                           "::mlir::LLVM::LLVMDialect"];
}

#endif // BELALANG_CONVERSIONS_PASSES_TD_
