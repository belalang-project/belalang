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
  using OpRewritePattern<bir::ScopeOp>::OpRewritePattern;

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
    cf::BranchOp::create(rewriter, op.getLoc(), mlir::ValueRange(), beforeBody);

    rewriter.setInsertionPointToEnd(afterBody);
    if (auto yieldOp = dyn_cast<bir::YieldOp>(afterBody->getTerminator()))
      rewriter.replaceOpWithNewOp<cf::BranchOp>(yieldOp, yieldOp.getArgs(),
                                                continueBlock);

    rewriter.replaceOp(op, continueBlock->getArguments());

    return success();
  }
};

class BIRIfOpFlattening : public mlir::OpRewritePattern<bir::IfOp> {
public:
  using OpRewritePattern<bir::IfOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(bir::IfOp op,
                  mlir::PatternRewriter &rewriter) const override {
    OpBuilder::InsertionGuard guard(rewriter);

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
            dyn_cast<bir::YieldOp>(thenAfterBody->getTerminator())) {
      rewriter.setInsertionPointToEnd(thenAfterBody);
      rewriter.replaceOpWithNewOp<cf::BranchOp>(
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
            dyn_cast<bir::YieldOp>(elseAfterBody->getTerminator())) {
      rewriter.setInsertionPointToEnd(elseAfterBody);
      rewriter.replaceOpWithNewOp<cf::BranchOp>(
          elseYieldOp, elseYieldOp.getArgs(), continueBlock);
    }

    // Decide where to go, either the then block or else block.
    rewriter.setInsertionPointToEnd(currentBlock);
    bir::CondBrOp::create(rewriter, op.getLoc(), op.getCond(), thenBeforeBody,
                          elseBeforeBody);

    rewriter.replaceOp(op, continueBlock->getArguments());
    return success();
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
    cf::BranchOp::create(rewriter, op.getLoc(), entry);

    // Lower condition.
    auto conditionOp =
        cast<bir::ConditionOp>(op.getCond().back().getTerminator());
    {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPoint(conditionOp);
      rewriter.replaceOpWithNewOp<bir::CondBrOp>(
          conditionOp, conditionOp.getCond(), body, exit);
    }

    // Replace continues and breaks with branch op.
    op.walk([&](mlir::Operation *op) {
      if (isa<bir::BreakOp>(op)) {
        OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointAfter(op);
        rewriter.replaceOpWithNewOp<cf::BranchOp>(op, exit);
      }
      if (isa<bir::ContinueOp>(op)) {
        OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointAfter(op);
        rewriter.replaceOpWithNewOp<cf::BranchOp>(op, cond);
      }
    });

    // End body with jump to condition.
    {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToEnd(&op.getBody().back());
      if (auto continueOp =
              dyn_cast<bir::ContinueOp>(op.getBody().back().getTerminator()))
        rewriter.replaceOpWithNewOp<cf::BranchOp>(continueOp, cond);
    }

    // Inline contents.
    rewriter.inlineRegionBefore(op.getCond(), exit);
    rewriter.inlineRegionBefore(op.getBody(), exit);

    // Yay!
    rewriter.eraseOp(op);
    return success();
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
    : public impl::BelalangFlattenCFGPassBase<BelalangFlattenCFGPass> {
  using impl::BelalangFlattenCFGPassBase<
      BelalangFlattenCFGPass>::BelalangFlattenCFGPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangFlattenCFGPatterns(patterns);

    llvm::SmallVector<Operation *, 16> ops;
    getOperation()->walk<mlir::WalkOrder::PostOrder>([&](Operation *op) {
      if (isa<ScopeOp, IfOp, WhileOp>(op))
        ops.push_back(op);
    });

    if (applyOpPatternsGreedily(ops, std::move(patterns)).failed())
      signalPassFailure();
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangFlattenCFGPass() {
  return std::make_unique<BelalangFlattenCFGPass>();
}
