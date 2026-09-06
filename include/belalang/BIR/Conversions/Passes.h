#ifndef BELALANG_CONVERSIONS_PASSES_H_
#define BELALANG_CONVERSIONS_PASSES_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Conversions/Passes.h.inc"

#define GEN_PASS_REGISTRATION_BELALANGBIRTOLLVMPASS
#include "belalang/BIR/Conversions/Passes.h.inc"

void populateBelalangBIRToLLVMPatterns(mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter &typeConverter);

} // namespace bir
} // namespace belalang

#endif // BELALANG_CONVERSIONS_PASSES_H_
