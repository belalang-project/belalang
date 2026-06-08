#ifndef BELALANG_PASSES_H_
#define BELALANG_PASSES_H_

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
class RewritePatternSet;
} // namespace mlir

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/BIR/Passes.h.inc"

void populateBelalangConstantsPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangRuntimizePatterns(mlir::RewritePatternSet &patterns);

} // namespace bir
} // namespace belalang

#endif // BELALANG_PASSES_H_
