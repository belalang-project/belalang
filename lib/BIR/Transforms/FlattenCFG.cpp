#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Interfaces/LoopOpInterface.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGFLATTENCFGPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;
using namespace belalang::bir;

class BIRScopeOpFlattening : public mlir::OpRewritePattern<bir::ScopeOp> {
public:
  using mlir::OpRewritePattern<bir::ScopeOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(bir::ScopeOp op,
                  mlir::PatternRewriter &rewriter) const override {
    // The implementation here is very similar to CIRScopeOpFlattening pattern
    // used in ClangIR. The biggest difference is that we're using the cf
    // dialect instead of making our own branch op.
    mlir::Block *currentBlock = rewriter.getInsertionBlock();
    mlir::Block *continueBlock =
        rewriter.splitBlock(currentBlock, rewriter.getInsertionPoint());

    if (op.getNumResults() > 0)
      continueBlock->addArguments(op.getResultTypes(), op.getLoc());

    mlir::Block *beforeBody = &op.getScopeRegion().front();
    mlir::Block *afterBody = &op.getScopeRegion().back();
    rewriter.inlineRegionBefore(op.getScopeRegion(), continueBlock);

    rewriter.setInsertionPointToEnd(currentBlock);
    mlir::cf::BranchOp::create(rewriter, op.getLoc(), mlir::ValueRange(),
                               beforeBody);

    rewriter.setInsertionPointToEnd(afterBody);
    if (auto yieldOp = mlir::dyn_cast<bir::YieldOp>(afterBody->getTerminator()))
      rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(
          yieldOp, yieldOp.getArgs(), continueBlock);

    rewriter.replaceOp(op, continueBlock->getArguments());

    return mlir::success();
  }
};

class BIRIfOpFlattening : public mlir::OpRewritePattern<bir::IfOp> {
public:
  using mlir::OpRewritePattern<bir::IfOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(bir::IfOp op,
                  mlir::PatternRewriter &rewriter) const override {
    mlir::OpBuilder::InsertionGuard guard(rewriter);

    mlir::Block *currentBlock = rewriter.getInsertionBlock();
    mlir::Block *continueBlock =
        rewriter.splitBlock(currentBlock, rewriter.getInsertionPoint());

    if (!op.getResults().empty())
      // Add arguments for the if expression output
      continueBlock->addArguments(op->getResultTypes(), {op.getLoc()});

    // Inline then
    mlir::Block *thenBeforeBody = &op.getThenRegion().front();
    mlir::Block *thenAfterBody = &op.getThenRegion().back();
    rewriter.inlineRegionBefore(op.getThenRegion(), continueBlock);

    if (auto thenYieldOp =
            mlir::dyn_cast<bir::YieldOp>(thenAfterBody->getTerminator())) {
      rewriter.setInsertionPointToEnd(thenAfterBody);
      rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(
          thenYieldOp, thenYieldOp.getArgs(), continueBlock);
    }

    // Inline else, if exists
    mlir::Block *elseBeforeBody = nullptr;
    mlir::Block *elseAfterBody = nullptr;
    if (op.hasElse()) {
      elseBeforeBody = &op.getElseRegion().front();
      elseAfterBody = &op.getElseRegion().back();
      rewriter.inlineRegionBefore(op.getElseRegion(), continueBlock);
    } else {
      elseBeforeBody = elseAfterBody = continueBlock;
    }

    if (auto elseYieldOp =
            mlir::dyn_cast<bir::YieldOp>(elseAfterBody->getTerminator())) {
      rewriter.setInsertionPointToEnd(elseAfterBody);
      rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(
          elseYieldOp, elseYieldOp.getArgs(), continueBlock);
    }

    // Decide where to go, either the then block or else block.
    rewriter.setInsertionPointToEnd(currentBlock);
    bir::CondBrOp::create(rewriter, op.getLoc(), op.getCond(), thenBeforeBody,
                          elseBeforeBody);

    rewriter.replaceOp(op, continueBlock->getArguments());
    return mlir::success();
  }
};

class BIRLoopOpInterfaceFlattening
    : public mlir::OpInterfaceRewritePattern<bir::LoopOpInterface> {
public:
  using OpInterfaceRewritePattern<
      bir::LoopOpInterface>::OpInterfaceRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(bir::LoopOpInterface op,
                  mlir::PatternRewriter &rewriter) const override {
    // CFG blocks.
    mlir::Block *currentBlock = rewriter.getInsertionBlock();
    mlir::Block *exit =
        rewriter.splitBlock(currentBlock, rewriter.getInsertionPoint());
    mlir::Block *entry = &op.getEntry().front();
    mlir::Block *cond = &op.getCond().front();
    mlir::Block *body = &op.getBody().front();
    // step

    // Loop entry branch.
    rewriter.setInsertionPointToEnd(currentBlock);
    mlir::cf::BranchOp::create(rewriter, op.getLoc(), entry);

    // Lower condition.
    auto conditionOp =
        mlir::cast<bir::ConditionOp>(op.getCond().back().getTerminator());
    {
      mlir::OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPoint(conditionOp);
      rewriter.replaceOpWithNewOp<bir::CondBrOp>(
          conditionOp, conditionOp.getCond(), body, exit);
    }

    // Replace continues and breaks with branch op.
    op.walk([&](mlir::Operation *op) {
      if (mlir::isa<bir::BreakOp>(op)) {
        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointAfter(op);
        rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(op, exit);
      }
      if (mlir::isa<bir::ContinueOp>(op)) {
        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointAfter(op);
        rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(op, cond);
      }
    });

    // Lower yield terminator.
    for (mlir::Block &blk : op.getBody().getBlocks()) {
      if (auto yield = mlir::dyn_cast<bir::YieldOp>(blk.getTerminator())) {
        rewriter.setInsertionPointToEnd(&op.getBody().back());
        rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(yield, cond);
      }
    }

    // Inline contents.
    rewriter.inlineRegionBefore(op.getCond(), exit);
    rewriter.inlineRegionBefore(op.getBody(), exit);

    // Yay!
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

}; // namespace

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

void belalang::bir::populateBelalangFlattenCFGPatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<BIRScopeOpFlattening, BIRIfOpFlattening,
               BIRLoopOpInterfaceFlattening>(patterns.getContext());
}

struct BelalangFlattenCFGPass
    : public mlir::impl::BelalangFlattenCFGPassBase<BelalangFlattenCFGPass> {
  using mlir::impl::BelalangFlattenCFGPassBase<
      BelalangFlattenCFGPass>::BelalangFlattenCFGPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangFlattenCFGPatterns(patterns);

    llvm::SmallVector<mlir::Operation *, 16> ops;
    getOperation()->walk<mlir::WalkOrder::PostOrder>([&](mlir::Operation *op) {
      if (mlir::isa<ScopeOp, IfOp, WhileOp>(op))
        ops.push_back(op);
    });

    if (applyOpPatternsGreedily(ops, std::move(patterns)).failed())
      signalPassFailure();
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangFlattenCFGPass() {
  return std::make_unique<BelalangFlattenCFGPass>();
}
