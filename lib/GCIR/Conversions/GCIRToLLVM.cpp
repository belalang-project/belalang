#include "belalang/GCIR/IR/GCIR.h"
#include "belalang/GCIR/Conversions/Passes.h"

#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGGCIRTOLLVMPASS
#include "belalang/GCIR/Conversions/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace belalang::gc;

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

struct GCIRToLLVMPass
    : public mlir::impl::BelalangGCIRToLLVMPassBase<GCIRToLLVMPass> {
  using mlir::impl::BelalangGCIRToLLVMPassBase<
      GCIRToLLVMPass>::BelalangGCIRToLLVMPassBase;

  void runOnOperation() override {
    GCIRToLLVMTypeConverter converter(&getContext());

    RewritePatternSet patterns(&getContext());
    patterns.add<AllocOpLowering>(converter, &getContext(), allocator);
    populateFuncToLLVMConversionPatterns(converter, patterns);

    ConversionTarget target(getContext());
    target.addLegalDialect<LLVM::LLVMDialect, BuiltinDialect>();
    target.addIllegalDialect<GCIRDialect>();

    if (applyFullConversion(getOperation(), target, std::move(patterns))
            .failed())
      signalPassFailure();
  }
};

} // namespace
