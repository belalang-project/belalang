#include "belalang/BIR/Passes.h"
#include "mlir/Transforms/Passes.h"

namespace belalang {
namespace bir {

void buildBIRLoweringPipeline(mlir::OpPassManager &pm) {
  buildBIRLoweringPipeline(pm, BIRLoweringPipelineOptions());
}

void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options) {
  pm.addPass(createBelalangLowerFuncExprPass());
  pm.addPass(createBelalangLowerToRuntimeCallsPass());
  pm.addPass(createBelalangOptimizeStructLayoutPass());
  pm.addPass(createBelalangLowerDeclToMemoryPass());
  pm.addPass(createBelalangFlattenCFGPass());
  if (options.enableMem2Reg) {
    pm.addPass(mlir::createMem2Reg());
  }
  if (options.enableDCE) {
    pm.addPass(mlir::createTrivialDeadCodeEliminationPass());
    pm.addPass(mlir::createSymbolDCEPass());
  }
  pm.addPass(createBelalangInsertStackMapsPass());
  pm.addPass(createBelalangVerifyLoweredFormPass());
}

void registerBIRPipelines() {
  mlir::PassPipelineRegistration<BIRLoweringPipelineOptions>(
      "bir-lowering-pipeline", "Default lowering pipeline for BIR dialect.",
      [](mlir::OpPassManager &pm, const BIRLoweringPipelineOptions &options) {
        bir::buildBIRLoweringPipeline(pm, options);
      });
}

} // namespace bir
} // namespace belalang
