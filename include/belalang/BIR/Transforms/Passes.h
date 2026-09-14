#ifndef BELALANG_TRANSFORMS_PASSES_H_
#define BELALANG_TRANSFORMS_PASSES_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
class DataFlowSolver;
} // namespace mlir

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Transforms/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/BIR/Transforms/Passes.h.inc"

void populateBelalangFlattenCFGPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangLowerDeclToMemoryPatterns(
    mlir::RewritePatternSet &patterns, mlir::DataFlowSolver &solver);
void populateBelalangLowerFuncExprPatterns(mlir::RewritePatternSet &patterns);

} // namespace bir
} // namespace belalang

#endif // BELALANG_TRANSFORMS_PASSES_H_
