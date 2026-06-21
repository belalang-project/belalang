#include "belalang/BIR/Passes.h"

namespace belalang {
namespace bir {

void buildBIRLoweringPipeline(mlir::OpPassManager &pm) {
  buildBIRLoweringPipeline(pm, BIRLoweringPipelineOptions());
}

void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options) {
  pm.addPass(createBelalangLowerFuncExprPass());
  pm.addPass(createBelalangRuntimizePass());
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
