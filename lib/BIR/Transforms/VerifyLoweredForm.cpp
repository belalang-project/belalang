#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGVERIFYLOWEREDFORMPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;
using namespace belalang::bir;

bool isUnexpectedAfterLowering(mlir::Operation *op) {
  return mlir::isa<bir::DeclareOp, bir::ScopeOp, bir::IfOp, bir::WhileOp,
                   bir::BreakOp, bir::ContinueOp, bir::ConditionOp,
                   bir::YieldOp, bir::FuncExprOp>(op);
}

struct BelalangVerifyLoweredFormPass
    : public mlir::impl::BelalangVerifyLoweredFormPassBase<
          BelalangVerifyLoweredFormPass> {
  using mlir::impl::BelalangVerifyLoweredFormPassBase<
      BelalangVerifyLoweredFormPass>::BelalangVerifyLoweredFormPassBase;

  void runOnOperation() override {
    auto result = getOperation()->walk<mlir::WalkOrder::PreOrder>(
        [&](mlir::Operation *op) {
          if (!isUnexpectedAfterLowering(op))
            return mlir::WalkResult::advance();

          op->emitOpError()
              << "unexpected high-level BIR operation after lowering pipeline";
          return mlir::WalkResult::interrupt();
        });

    if (result.wasInterrupted())
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<mlir::Pass>
belalang::bir::createBelalangVerifyLoweredFormPass() {
  return std::make_unique<BelalangVerifyLoweredFormPass>();
}
