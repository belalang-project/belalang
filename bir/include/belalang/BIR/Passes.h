#ifndef BELALANG_PASSES_H_
#define BELALANG_PASSES_H_

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/BIR/Passes.h.inc"

void populateBelalangConstantsPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangRuntimizePatterns(mlir::RewritePatternSet &patterns);
void populateBelalangBIRToLLVMPatterns(mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter typeConverter);

} // namespace bir
} // namespace belalang

#endif // BELALANG_PASSES_H_
