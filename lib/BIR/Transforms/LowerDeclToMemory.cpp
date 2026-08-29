#include "belalang/BIR/Analysis/EscapeAnalysis.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Analysis/DataFlow/Utils.h"
#include "mlir/Analysis/DataFlowFramework.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGLOWERDECLTOMEMORYPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {

using namespace belalang;
using namespace belalang::bir;

struct DeclareOpLowering final : public mlir::OpRewritePattern<bir::DeclareOp> {
  DeclareOpLowering(mlir::MLIRContext *ctx, mlir::DataFlowSolver &solver)
      : mlir::OpRewritePattern<bir::DeclareOp>(ctx), solver(solver) {}

  mlir::LogicalResult
  matchAndRewrite(bir::DeclareOp op,
                  mlir::PatternRewriter &rewriter) const override {
    const auto *state = 
        solver.lookupState<EscapeLattice>(op.getResult());
    bool escapes = state && state->escapes;

    auto refType = mlir::cast<bir::RefType>(op.getType());
    if (escapes)
      rewriter.replaceOpWithNewOp<bir::AllocHeapOp>(op, refType,
                                                    mlir::ValueRange{});
    else
      rewriter.replaceOpWithNewOp<bir::AllocStackOp>(op, refType);

    return mlir::success();
  };

private:
  mlir::DataFlowSolver &solver;
};

} // namespace

void belalang::bir::populateBelalangLowerDeclToMemoryPatterns(
    mlir::RewritePatternSet &patterns, mlir::DataFlowSolver &solver) {
  patterns.add<DeclareOpLowering>(patterns.getContext(), solver);
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangLowerDeclToMemoryPass
    : public mlir::impl::BelalangLowerDeclToMemoryPassBase<
          BelalangLowerDeclToMemoryPass> {
  using mlir::impl::BelalangLowerDeclToMemoryPassBase<
      BelalangLowerDeclToMemoryPass>::BelalangLowerDeclToMemoryPassBase;

  void runOnOperation() override {
    mlir::SymbolTableCollection symbolTable;

    mlir::DataFlowSolver solver;
    mlir::dataflow::loadBaselineAnalyses(solver);
    solver.load<EscapeAnalysis>(symbolTable);
    if (solver.initializeAndRun(getOperation()).failed())
      return signalPassFailure();

    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangLowerDeclToMemoryPatterns(patterns, solver);

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
