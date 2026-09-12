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

struct AllocOpLowering final : OpConversionPattern<AllocOp> {
  AllocOpLowering(TypeConverter &converter, MLIRContext *context,
                  llvm::StringRef allocator)
      : OpConversionPattern<AllocOp>(converter, context),
        allocator(allocator.str()) {}

  LogicalResult
  matchAndRewrite(AllocOp op, OpAdaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto resultType = getTypeConverter()->convertType(op.getResult().getType());
    if (!resultType)
      return failure();

    auto ptrType = cast<PtrType>(op.getResult().getType());
    auto dataLayout = DataLayout::closest(op);
    auto size = dataLayout.getTypeSize(ptrType.getPointee());
    if (size.isScalable()) {
      op.emitError("cannot lower allocation of a scalable type");
      return failure();
    }

    auto module = op->getParentOfType<ModuleOp>();
    auto *ctx = op.getContext();
    auto i64 = IntegerType::get(ctx, 64);
    auto llvmPtr = LLVM::LLVMPointerType::get(ctx);
    auto functionType = LLVM::LLVMFunctionType::get(llvmPtr, {i64});
    auto function = module.lookupSymbol<LLVM::LLVMFuncOp>(allocator);
    if (function) {
      if (function.getFunctionType() != functionType) {
        op.emitError(
            "allocator function has incompatible type; expected ptr(i64)");
        return failure();
      }
    } else {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(module.getBody());
      function = LLVM::LLVMFuncOp::create(rewriter, op.getLoc(), allocator,
                                          functionType);
    }

    auto sizeValue = LLVM::ConstantOp::create(
        rewriter, op.getLoc(), i64,
        rewriter.getI64IntegerAttr(size.getFixedValue()));
    auto call = LLVM::CallOp::create(rewriter, op.getLoc(), llvmPtr,
                                     FlatSymbolRefAttr::get(ctx, allocator),
                                     ValueRange{sizeValue});
    rewriter.replaceOp(op, call.getResult());
    return success();
  }

  std::string allocator;
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
    patterns.add<AllocOpLowering>(converter, &getContext(), allocator);
    patterns.add<AllocaOpLowering>(converter, &getContext());
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
