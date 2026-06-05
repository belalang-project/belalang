#include "belalang_ir/Passes.h"
#include "belalang_ir/IR/Dialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace bir {

#define GEN_PASS_DEF_BELALANGCONSTANTSPASS
#include "belalang_ir/Passes.h.inc"

namespace {
struct ConstantOpLowering : public mlir::OpRewritePattern<ConstantOp> {
  using OpRewritePattern<ConstantOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(ConstantOp op,
                  mlir::PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<mlir::arith::ConstantOp>(op, op.getValue());
    return mlir::success();
  }
};

struct BelalangConstantsPass
    : public impl::BelalangConstantsPassBase<BelalangConstantsPass> {
  using impl::BelalangConstantsPassBase<
      BelalangConstantsPass>::BelalangConstantsPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<ConstantOpLowering>(&getContext());

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
} // namespace

} // namespace bir
