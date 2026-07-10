#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGLOWERDECLTOMEMORYPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {

using namespace belalang;
using namespace belalang::bir;

struct DeclareOpLowering final : public OpRewritePattern<bir::DeclareOp> {
  using OpRewritePattern<bir::DeclareOp>::OpRewritePattern;

  LogicalResult
  matchAndRewrite(bir::DeclareOp op,
                  mlir::PatternRewriter &rewriter) const override {
    auto refType = mlir::cast<bir::RefType>(op.getType());
    auto referentTy = refType.getReferent();

    int64_t elSize;
    if (mlir::isa<bir::IntType>(referentTy))
      elSize = 8;
    else if (mlir::isa<bir::FloatType>(referentTy))
      elSize = 8;
    else if (mlir::isa<bir::StringType>(referentTy))
      elSize = 16;
    else if (mlir::isa<bir::BoolType>(referentTy))
      elSize = 8;
    else if (mlir::isa<mlir::FunctionType>(referentTy))
      elSize = 8;
    else
      return failure();

    mlir::Type ty = op.getResult().getType();
    rewriter.replaceOpWithNewOp<bir::AllocHeapOp>(op, ty, elSize);
    return success();
  };
};

} // namespace

void belalang::bir::populateBelalangLowerDeclToMemoryPatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<DeclareOpLowering>(patterns.getContext());
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangLowerDeclToMemoryPass
    : public impl::BelalangLowerDeclToMemoryPassBase<
          BelalangLowerDeclToMemoryPass> {
  using impl::BelalangLowerDeclToMemoryPassBase<
      BelalangLowerDeclToMemoryPass>::BelalangLowerDeclToMemoryPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangLowerDeclToMemoryPatterns(patterns);

    if (mlir::applyPatternsGreedily(getOperation(), std::move(patterns))
            .failed()) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass>
belalang::bir::createBelalangLowerDeclToMemoryPass() {
  return std::make_unique<BelalangLowerDeclToMemoryPass>();
}
