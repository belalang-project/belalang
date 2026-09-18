#ifndef BELALANG_LOWERING_PIPELINES_H_
#define BELALANG_LOWERING_PIPELINES_H_

#include "mlir/Pass/Pass.h"
#include "llvm/Support/CommandLine.h"

namespace belalang {
namespace lowering {

enum class LoweringTarget {
  BIRLowered,
  GC,
  GCLowered,
  GCLLVM,
  LLVM,
};

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

  // clang-format off
  mlir::detail::PassOptions::Option<LoweringTarget> target{
      *this,
      "target",
      llvm::cl::desc("Set the target dialect for BIR lowering."),
      llvm::cl::init(LoweringTarget::LLVM),
      llvm::cl::values(
          clEnumValN(LoweringTarget::BIRLowered,
                     "bir-lowered",
                     "Lowered BIR dialect."),
          clEnumValN(LoweringTarget::GC,
                     "gc",
                     "GC dialect."),
          clEnumValN(LoweringTarget::GCLowered,
                     "gc-lowered",
                     "Lowered GC dialect."),
          clEnumValN(LoweringTarget::GCLLVM,
                     "gc-llvm",
                     "LLVM dialect lowered through the GC dialect."),
          clEnumValN(LoweringTarget::LLVM,
                     "llvm",
                     "LLVM dialect.")),
  };
  // clang-format on
};

void buildBIRLoweringPipeline(mlir::OpPassManager &pm);
void buildBIRLoweringPipeline(mlir::OpPassManager &pm,
                              const BIRLoweringPipelineOptions &options);

void registerBIRPipelines();

} // namespace lowering
} // namespace belalang

#endif // BELALANG_LOWERING_PIPELINES_H_
