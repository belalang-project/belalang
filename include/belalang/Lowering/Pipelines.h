#ifndef BELALANG_LOWERING_PIPELINES_H_
#define BELALANG_LOWERING_PIPELINES_H_

#include "mlir/Pass/Pass.h"

namespace belalang {
namespace lowering {

struct BIRLoweringPipelineOptions
    : public mlir::PassPipelineOptions<BIRLoweringPipelineOptions> {
  mlir::detail::PassOptions::Option<bool> enableDCE{
      *this,
      "enable-dce",
      llvm::cl::desc("Enables dead code elimination."),
      llvm::cl::init(true),
  };

  mlir::detail::PassOptions::Option<bool> enableMem2Reg{
      *this,
      "enable-mem2reg",
      llvm::cl::desc("Enables mem2reg."),
      llvm::cl::init(true),
  };

  mlir::detail::PassOptions::Option<bool> onlyBIR{
      *this,
      "only-bir",
      llvm::cl::desc("Stop after lowering to BIR."),
      llvm::cl::init(false),
  };
};

void buildBIRLoweringPipeline(mlir::OpPassManager &pm);
void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options);

void registerBIRPipelines();

} // namespace lowering
} // namespace belalang

#endif // BELALANG_LOWERING_PIPELINES_H_
