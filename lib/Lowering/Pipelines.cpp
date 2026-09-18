#include "belalang/BIR/Conversions/Passes.h"
#include "belalang/BIR/Transforms/Passes.h"
#include "belalang/Lowering/Pipelines.h"
#include "mlir/Dialect/GC/Passes.h"
#include "mlir/Transforms/Passes.h"

namespace belalang {
namespace lowering {

void buildBIRLoweringPipeline(mlir::OpPassManager &pm) {
  buildBIRLoweringPipeline(pm, BIRLoweringPipelineOptions());
}

void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options) {
  pm.addPass(bir::createBelalangLowerFuncExprPass());
  pm.addPass(bir::createBelalangOptimizeStructLayoutPass());
  pm.addPass(bir::createBelalangLowerDeclToMemoryPass());
  pm.addPass(bir::createBelalangFlattenCFGPass());
  if (options.enableMem2Reg) {
    pm.addPass(mlir::createMem2Reg());
  }
  pm.addPass(mlir::createCSEPass());
  if (options.enableDCE) {
    pm.addPass(mlir::createTrivialDeadCodeEliminationPass());
    pm.addPass(mlir::createSymbolDCEPass());
  }
  pm.addPass(bir::createBelalangPrepareGCAllocationsPass());
  pm.addPass(bir::createBelalangVerifyLoweredFormPass());
  if (options.target == LoweringTarget::BIRLowered)
    return;
  // This branch is currently only enabled for GC lowering targets.
  // This is because the GC lowering path is not yet supported.
  //
  // TODO: Make the GC lowering path the default and not gated.
  if (options.target == LoweringTarget::GC ||
      options.target == LoweringTarget::GCLowered) {
    pm.addPass(bir::createBelalangBIRToGCPass());
    if (options.target == LoweringTarget::GC)
      return;
    pm.addPass(mlir::createGCIRPrepareGCSafepointsPass());
    pm.addPass(mlir::createGCLowerAllocationsPass());
    if (options.target == LoweringTarget::GCLowered)
      return;
  }
  pm.addPass(bir::createBelalangBIRToLLVMPass());
}

void registerBIRPipelines() {
  mlir::PassPipelineRegistration<BIRLoweringPipelineOptions>(
      "bir-lowering-pipeline", "Default lowering pipeline for BIR dialect.",
      [](mlir::OpPassManager &pm, const BIRLoweringPipelineOptions &options) {
        buildBIRLoweringPipeline(pm, options);
      });
}

} // namespace lowering
} // namespace belalang
