#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_GCLOWERALLOCATIONSPASS
#include "mlir/Dialect/GC/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;

struct LowerAllocPattern final : public OpRewritePattern<gc::AllocOp> {
  LowerAllocPattern(MLIRContext *context, StringRef allocator)
      : OpRewritePattern<gc::AllocOp>(context), allocator(allocator.str()) {}

  LogicalResult matchAndRewrite(gc::AllocOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<gc::CallOp>(
        op, FlatSymbolRefAttr::get(op.getContext(), allocator),
        op.getResultTypes(), op.getRoots());
    return success();
  }

  std::string allocator;
};

struct GCLowerAllocations
    : public impl::GCLowerAllocationsPassBase<GCLowerAllocations> {
  using impl::GCLowerAllocationsPassBase<
      GCLowerAllocations>::GCLowerAllocationsPassBase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<LowerAllocPattern>(&getContext(), allocator);
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace
