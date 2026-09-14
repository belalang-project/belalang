#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_GCTOLLVMPASS
#include "mlir/Dialect/GC/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace mlir::gc;

static void configureGCToLLVMTypeConverter(LLVMTypeConverter &c) {
  c.addConversion([](PtrType type) {
    return LLVM::LLVMPointerType::get(type.getContext());
  });
}

struct CallOpConversion final : OpConversionPattern<CallOp> {
  using OpConversionPattern<CallOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(CallOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<Type> resultTypes;
    if (getTypeConverter()->convertTypes(op.getResultTypes(), resultTypes).failed())
      return failure();
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(
        op, resultTypes, op.getCalleeAttr(), adaptor.getOperands());
    return success();
  }
};

struct AllocaOpLowering final : OpConversionPattern<AllocaOp> {
  using OpConversionPattern<AllocaOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(AllocaOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto ctx = op.getContext();
    auto loc = op.getLoc();

    mlir::Type resultTy = LLVM::LLVMPointerType::get(ctx);
    mlir::Type elementTy = getTypeConverter()->convertType(
        op.getType().getPointee());

    // Currently limit the possible allocation size to one element.
    auto i64ty = rewriter.getI64Type();
    mlir::Value arraySize = LLVM::ConstantOp::create(rewriter, loc, i64ty, 1);

    rewriter.replaceOpWithNewOp<LLVM::AllocaOp>(op, resultTy, elementTy,
                                                arraySize);
    return success();
  }
};

void populateGCToLLVMPatterns(mlir::RewritePatternSet &patterns,
                              mlir::TypeConverter &typeConverter) {
  patterns.add<CallOpConversion, AllocaOpLowering>(typeConverter,
                                                   patterns.getContext());
}

struct GCIRToLLVMPass
    : public mlir::impl::GCToLLVMPassBase<GCIRToLLVMPass> {
  using mlir::impl::GCToLLVMPassBase<
      GCIRToLLVMPass>::GCToLLVMPassBase;

  void runOnOperation() override {
    LLVMTypeConverter converter(&getContext());
    configureGCToLLVMTypeConverter(converter);

    RewritePatternSet patterns(&getContext());
    populateGCToLLVMPatterns(patterns, converter);
    populateFuncToLLVMConversionPatterns(converter, patterns);

    ConversionTarget target(getContext());
    target.addLegalDialect<LLVM::LLVMDialect, BuiltinDialect>();
    target.addIllegalDialect<GCDialect>();

    if (applyFullConversion(getOperation(), target, std::move(patterns))
            .failed())
      signalPassFailure();
  }
};

struct GCToLLVMDialectInterface final : ConvertToLLVMPatternInterface {
  GCToLLVMDialectInterface(Dialect *dialect)
      : ConvertToLLVMPatternInterface(dialect) {}

  void loadDependentDialects(MLIRContext *ctx) const final {
    ctx->loadDialect<LLVM::LLVMDialect>();
  }

  void populateConvertToLLVMConversionPatterns(
      ConversionTarget &target, LLVMTypeConverter &typeConverter,
      RewritePatternSet &patterns) const final {
    configureGCToLLVMTypeConverter(typeConverter);
    populateGCToLLVMPatterns(patterns, typeConverter);
  }
};

} // namespace

void mlir::gc::registerGCToLLVMInterface(mlir::DialectRegistry &registry) {
  registry.addExtension(+[](mlir::MLIRContext *context,
                            mlir::gc::GCDialect *dialect) {
    dialect->addInterfaces<GCToLLVMDialectInterface>();
  });
}
