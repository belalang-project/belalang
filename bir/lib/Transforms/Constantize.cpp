#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGCONSTANTSPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang::bir;

struct ConstantOpLowering : public mlir::OpRewritePattern<ConstantOp> {
  using OpRewritePattern<ConstantOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(ConstantOp op,
                  mlir::PatternRewriter &rewriter) const override {
    auto value = op.getValue();
    if (auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(value)) {
      auto newOp = mlir::arith::ConstantOp::create(
          rewriter, op.getLoc(), rewriter.getI32IntegerAttr(intAttr.getInt()));
      rewriter.replaceOpWithNewOp<mlir::UnrealizedConversionCastOp>(
          op, op.getType(), newOp.getResult());
      return mlir::success();
    }
    if (auto floatAttr = mlir::dyn_cast<mlir::FloatAttr>(value)) {
      auto newOp = mlir::arith::ConstantOp::create(
          rewriter, op.getLoc(),
          rewriter.getF32FloatAttr(floatAttr.getValueAsDouble()));
      rewriter.replaceOpWithNewOp<mlir::UnrealizedConversionCastOp>(
          op, op.getType(), newOp.getResult());
      return mlir::success();
    }
    return mlir::failure();
  }
};
} // namespace

void belalang::bir::populateBelalangConstantsPatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<ConstantOpLowering>(patterns.getContext());
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangConstantsPass
    : public impl::BelalangConstantsPassBase<BelalangConstantsPass> {
  using impl::BelalangConstantsPassBase<
      BelalangConstantsPass>::BelalangConstantsPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    populateBelalangConstantsPatterns(patterns);

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangConstantsPass() {
  return std::make_unique<BelalangConstantsPass>();
}
