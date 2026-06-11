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
  using OpConversionPattern<bir::ConstantOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::ConstantOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    Attribute value = op.getValue();
    if (auto intAttr = llvm::dyn_cast<IntegerAttr>(value)) {
      value = rewriter.getIntegerAttr(type, intAttr.getValue());
    } else if (auto floatAttr = llvm::dyn_cast<FloatAttr>(value)) {
      value = rewriter.getFloatAttr(type, floatAttr.getValue());
    } else {
      return failure();
    }

    rewriter.replaceOpWithNewOp<LLVM::ConstantOp>(op, type, value);
    return success();
  }
};

struct BIRToLLVMTypeConverter : public mlir::TypeConverter {
  BIRToLLVMTypeConverter() {
    addConversion([](bir::IntType ty) {
      return mlir::IntegerType::get(ty.getContext(), 32);
    });
    addConversion([](bir::FloatType ty) {
      return mlir::Float32Type::get(ty.getContext());
    });
  }
};

} // namespace

void belalang::bir::populateBelalangBIRToLLVMPatterns(
    mlir::RewritePatternSet &patterns, mlir::TypeConverter &typeConverter) {
  patterns.add<ConstantOpLowering>(typeConverter, patterns.getContext());
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangBIRToLLVMPass
    : public impl::BelalangBIRToLLVMPassBase<BelalangBIRToLLVMPass> {
  using impl::BelalangBIRToLLVMPassBase<
      BelalangBIRToLLVMPass>::BelalangBIRToLLVMPassBase;

  void runOnOperation() override {
    BIRToLLVMTypeConverter typeConverter;

    mlir::ConversionTarget target(getContext());
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();
    target.addIllegalDialect<bir::BIRDialect>();

    // TODO: make this full by making all bir ops illegal
    target.addLegalOp<bir::FuncOp, bir::ReturnOp>();

    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangBIRToLLVMPatterns(patterns, typeConverter);

    if (mlir::failed(mlir::applyPartialConversion(getOperation(), target,
                                                  std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangBIRToLLVMPass() {
  return std::make_unique<BelalangBIRToLLVMPass>();
}
