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
  if (options.onlyBIR)
    return;
  pm.addPass(bir::createBelalangBIRToGCPass());
  pm.addPass(bir::createBelalangBIRToLLVMPass());
  pm.addPass(mlir::createGCIRPrepareGCSafepointsPass());
  pm.addPass(mlir::createGCLowerAllocationsPass());
  pm.addPass(mlir::createGCToLLVMPass());
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
