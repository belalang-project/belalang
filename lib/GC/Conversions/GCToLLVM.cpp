#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"

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

struct GCIRToLLVMTypeConverter final : LLVMTypeConverter {
  explicit GCIRToLLVMTypeConverter(MLIRContext *context)
      : LLVMTypeConverter(context) {
    addConversion([](PtrType type) {
      return LLVM::LLVMPointerType::get(type.getContext());
    });
  }
};

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

struct GCIRToLLVMPass
    : public mlir::impl::GCToLLVMPassBase<GCIRToLLVMPass> {
  using mlir::impl::GCToLLVMPassBase<
      GCIRToLLVMPass>::GCToLLVMPassBase;

  void runOnOperation() override {
    GCIRToLLVMTypeConverter converter(&getContext());

    RewritePatternSet patterns(&getContext());
    patterns.add<CallOpConversion, AllocaOpLowering>(converter, &getContext());
    populateFuncToLLVMConversionPatterns(converter, patterns);

    ConversionTarget target(getContext());
    target.addLegalDialect<LLVM::LLVMDialect, BuiltinDialect>();
    target.addIllegalDialect<GCDialect>();

    if (applyFullConversion(getOperation(), target, std::move(patterns))
            .failed())
      signalPassFailure();
  }
};

} // namespace
