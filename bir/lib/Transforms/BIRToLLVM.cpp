#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGBIRTOLLVMPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace belalang;

struct ConstantOpLowering final : public OpConversionPattern<bir::ConstantOp> {
  LogicalResult
  matchAndRewrite(bir::ConstantOp *op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const override {
    return failure();
  }
};

} // namespace

void belalang::bir::populateBelalangBIRToLLVMPatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<ConstantOpLowering>();
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangBIRToLLVMPass
    : public impl::BelalangBIRToLLVMPassBase<BelalangBIRToLLVMPass> {
  using impl::BelalangBIRToLLVMPassBase<
      BelalangBIRToLLVMPass>::BelalangBIRToLLVMPassBase;

  void runOnOperation() override {
    mlir::ConversionTarget target(getContext());
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();
    target.addIllegalDialect<bir::BIRDialect>();

    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangBIRToLLVMPatterns(patterns);

    if (mlir::failed(mlir::applyPartialConversion(getOperation(), target,
                                                  std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
