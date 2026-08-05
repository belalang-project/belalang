#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGLOWERFUNCEXPRPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;
using namespace belalang::bir;

struct FuncExprOpLowering : public mlir::OpRewritePattern<FuncExprOp> {
  using OpRewritePattern<FuncExprOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(FuncExprOp op,
                  mlir::PatternRewriter &rewriter) const override {
    // TODO: handle nested functions
    auto enclosingFunc = op->getParentOfType<FuncOp>();
    auto module = op->getParentOfType<ModuleOp>();
    mlir::SymbolTable symTable(module);
    FlatSymbolRefAttr symbolRef;

    {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(module.getBody());
      std::string baseName = ("fn." + enclosingFunc.getName() + ".anon").str();
      auto fn = FuncOp::create(rewriter, op.getLoc(), baseName, op.getType());
      rewriter.inlineRegionBefore(op.getBody(), fn.getBody(),
                                  fn.getBody().end());
      auto finalName = symTable.insert(fn);
      symbolRef = mlir::FlatSymbolRefAttr::get(finalName);
    }

    auto attr = FnAttr::get(op.getContext(), op.getType(), symbolRef);
    rewriter.replaceOpWithNewOp<ConstantOp>(op, op.getType(), attr);
    return success();
  }
};
} // namespace

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

void belalang::bir::populateBelalangLowerFuncExprPatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<FuncExprOpLowering>(patterns.getContext());
}

struct BelalangLowerFuncExprPass
    : public impl::BelalangLowerFuncExprPassBase<BelalangLowerFuncExprPass> {
  using impl::BelalangLowerFuncExprPassBase<
      BelalangLowerFuncExprPass>::BelalangLowerFuncExprPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangLowerFuncExprPatterns(patterns);

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangLowerFuncExprPass() {
  return std::make_unique<BelalangLowerFuncExprPass>();
}
