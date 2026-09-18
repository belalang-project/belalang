#include "belalang/BIR/Conversions/Passes.h"
#include "belalang/BIR/IR/BIR.h"
#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGBIRTOGCPASS
#include "belalang/BIR/Conversions/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace belalang;

struct AllocHeapOpConversion final
    : public OpConversionPattern<bir::AllocHeapOp> {
  using OpConversionPattern<bir::AllocHeapOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::AllocHeapOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto sourceTy = cast<bir::RefType>(op.getResult().getType());
    auto layout = bir::getGCAllocationLayout(sourceTy.getReferent(), op);
    if (!layout)
      return rewriter.notifyMatchFailure(op,
                                         "could not compute allocation layout");

    SmallVector<Type> resultTypes;
    if (failed(
            getTypeConverter()->convertTypes(op.getResultTypes(), resultTypes)))
      return failure();

    SmallVector<int32_t> pointerOffsets(layout->pointerOffsets.begin(),
                                        layout->pointerOffsets.end());
    SmallVector<NamedAttribute> attributes = {
        rewriter.getNamedAttr("size", rewriter.getI64IntegerAttr(layout->size)),
        rewriter.getNamedAttr("pointer_offsets",
                              rewriter.getDenseI32ArrayAttr(pointerOffsets))};
    rewriter.replaceOpWithNewOp<gc::AllocOp>(op, resultTypes,
                                             adaptor.getRoots(), attributes);
    return success();
  }
};

struct AllocStackOpConversion final
    : public OpConversionPattern<bir::AllocStackOp> {
  using OpConversionPattern<bir::AllocStackOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::AllocStackOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    mlir::Type sourceTy = cast<bir::RefType>(op.getResult().getType());
    mlir::Type targetTy = getTypeConverter()->convertType(sourceTy);
    rewriter.replaceOpWithNewOp<gc::AllocaOp>(op, targetTy);
    return success();
  }
};

struct BelalangBIRToGCPass
    : impl::BelalangBIRToGCPassBase<BelalangBIRToGCPass> {
  using impl::BelalangBIRToGCPassBase<
      BelalangBIRToGCPass>::BelalangBIRToGCPassBase;

  void runOnOperation() override {
    auto ctx = &getContext();

    TypeConverter typeConverter;
    typeConverter.addConversion([](Type type) { return type; });
    typeConverter.addConversion([ctx](bir::RefType ref) {
      return gc::PtrType::get(ctx, ref.getReferent());
    });

    auto materializeCast = [](OpBuilder &builder, Type type, ValueRange inputs,
                              Location loc) -> Value {
      if (inputs.size() != 1)
        return {};
      return UnrealizedConversionCastOp::create(builder, loc, type, inputs)
          .getResult(0);
    };
    typeConverter.addSourceMaterialization(materializeCast);
    typeConverter.addTargetMaterialization(materializeCast);

    ConversionTarget target(getContext());
    target.addDynamicallyLegalDialect<bir::BIRDialect>([](Operation *op) {
      return !isa<bir::AllocHeapOp, bir::AllocStackOp>(op);
    });
    target.addLegalDialect<gc::GCDialect>();
    target.addLegalOp<UnrealizedConversionCastOp>();

    RewritePatternSet patterns(&getContext());
    patterns.add<AllocHeapOpConversion, AllocStackOpConversion>(typeConverter,
                                                                ctx);

    if (applyPartialConversion(getOperation(), target, std::move(patterns))
            .failed())
      return signalPassFailure();
  }
};

} // namespace

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangBIRToGCPass() {
  return std::make_unique<BelalangBIRToGCPass>();
}
