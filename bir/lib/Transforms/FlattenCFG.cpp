#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGFLATTENCFGPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;
using namespace belalang::bir;

} // namespace

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

void belalang::bir::populateBelalangFlattenCFGPatterns(
    mlir::RewritePatternSet &patterns) {}

struct BelalangFlattenCFGPass
    : public impl::BelalangFlattenCFGPassBase<BelalangFlattenCFGPass> {
  using impl::BelalangFlattenCFGPassBase<
      BelalangFlattenCFGPass>::BelalangFlattenCFGPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangFlattenCFGPatterns(patterns);

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangFlattenCFGPass() {
  return std::make_unique<BelalangFlattenCFGPass>();
}
