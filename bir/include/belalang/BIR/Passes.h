#ifndef BELALANG_PASSES_H_
#define BELALANG_PASSES_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/BIR/Passes.h.inc"

void populateBelalangFlattenCFGPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangLowerDeclToMemoryPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangLowerFuncExprPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangLowerToRuntimeCallsPatterns(mlir::RewritePatternSet &patterns);
void populateBelalangBIRToLLVMPatterns(mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter &typeConverter);

// -----------------------------------------------------------------------------
// Pipelines
// -----------------------------------------------------------------------------

struct BIRLoweringPipelineOptions
    : public mlir::PassPipelineOptions<BIRLoweringPipelineOptions> {};

void buildBIRLoweringPipeline(mlir::OpPassManager &pm);
void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options);

void registerBIRPipelines();

} // namespace bir
} // namespace belalang

#endif // BELALANG_PASSES_H_
