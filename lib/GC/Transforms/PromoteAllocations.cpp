#include "mlir/Analysis/DataFlow/Utils.h"
#include "mlir/Analysis/DataFlowFramework.h"
#include "mlir/Dialect/GC/Analysis/EscapeAnalysis.h"
#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_GCPROMOTEALLOCATIONSPASS
#include "mlir/Dialect/GC/Passes.h.inc"
} // namespace mlir

namespace {
using namespace mlir;

struct AllocOpPromotion final : mlir::OpRewritePattern<gc::AllocOp> {
  AllocOpPromotion(mlir::MLIRContext *ctx, mlir::DataFlowSolver &solver)
      : mlir::OpRewritePattern<gc::AllocOp>(ctx), solver(solver) {}

  mlir::LogicalResult
  matchAndRewrite(gc::AllocOp op,
                  mlir::PatternRewriter &rewriter) const override {
    const auto *state = solver.lookupState<gc::EscapeLattice>(op.getResult());

    if (!state || state->escapes)
      return failure();

    // Replace alloc with alloca if it does not escape.
    auto ty = op.getResult().getType();
    rewriter.replaceOpWithNewOp<gc::AllocaOp>(op, ty);
    return success();
  }

private:
  mlir::DataFlowSolver &solver;
};

struct GCPromoteAllocationsPass
    : impl::GCPromoteAllocationsPassBase<GCPromoteAllocationsPass> {
  using impl::GCPromoteAllocationsPassBase<
      GCPromoteAllocationsPass>::GCPromoteAllocationsPassBase;

  void runOnOperation() override {
    SymbolTableCollection symbolTable;

    DataFlowSolver solver;
    dataflow::loadBaselineAnalyses(solver);
    solver.load<gc::EscapeAnalysis>(symbolTable);

    if (solver.initializeAndRun(getOperation()).failed())
      return signalPassFailure();

    RewritePatternSet patterns(&getContext());
    patterns.add<AllocOpPromotion>(patterns.getContext(), solver);

    if (applyPatternsGreedily(getOperation(), std::move(patterns)).failed())
      return signalPassFailure();
  }
};

} // namespace
