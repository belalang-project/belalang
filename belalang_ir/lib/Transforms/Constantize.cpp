#include "belalang_ir/IR/BIRDialect.h"
#include "belalang_ir/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"

namespace bir {

namespace {
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

void populateBelalangConstantsPatterns(mlir::RewritePatternSet &patterns) {
  patterns.add<ConstantOpLowering>(patterns.getContext());
}

} // namespace bir
