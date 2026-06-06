#include "belalang_ir/Passes.h"
#include "belalang_ir/IR/BIRDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace bir {

#define GEN_PASS_DEF_BELALANGCONSTANTSPASS
#include "belalang_ir/Passes.h.inc"

#define GEN_PASS_DEF_BELALANGLOWERPRINTPASS
#include "belalang_ir/Passes.h.inc"

namespace {

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

struct BelalangLowerPrintPass
    : public impl::BelalangLowerPrintPassBase<BelalangLowerPrintPass> {
  using impl::BelalangLowerPrintPassBase<
      BelalangLowerPrintPass>::BelalangLowerPrintPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    populateBelalangLowerPrintPatterns(patterns);

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

} // namespace bir
