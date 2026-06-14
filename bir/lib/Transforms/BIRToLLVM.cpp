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

struct FuncOpLowering final : public OpConversionPattern<bir::FuncOp> {
  using OpConversionPattern<bir::FuncOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::FuncOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    TypeConverter::SignatureConversion signatureConverter(
        op.getFunctionType().getNumInputs());

    auto funcType = getTypeConverter()->convertType(op.getFunctionType());
    if (!funcType || !mlir::isa<LLVM::LLVMFunctionType>(funcType))
      return failure();

    auto llvmFuncType = mlir::cast<LLVM::LLVMFunctionType>(funcType);
    for (unsigned i = 0; i < op.getFunctionType().getNumInputs(); ++i)
      signatureConverter.addInputs(i, llvmFuncType.getParamType(i));

    auto llvmFuncOp = LLVM::LLVMFuncOp::create(rewriter, op.getLoc(),
                                               op.getName(), llvmFuncType);

    rewriter.inlineRegionBefore(op.getBody(), llvmFuncOp.getBody(),
                                llvmFuncOp.end());
    if (failed(rewriter.convertRegionTypes(
            &llvmFuncOp.getBody(), *getTypeConverter(), &signatureConverter)))
      return failure();

    rewriter.eraseOp(op);
    return success();
  }
};

struct CallOpLowering final : public OpConversionPattern<bir::CallOp> {
  using OpConversionPattern<bir::CallOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::CallOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<mlir::Type> resultTypes;
    for (mlir::Type t : op.getResultTypes()) {
      mlir::Type converted = getTypeConverter()->convertType(t);
      if (!converted)
        return failure();
      resultTypes.push_back(converted);
    }
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(
        op, resultTypes, op.getCalleeAttr(), adaptor.getOperands());
    return success();
  }
};

struct ReturnOpLowering final : public OpConversionPattern<bir::ReturnOp> {
  using OpConversionPattern<bir::ReturnOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::ReturnOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<LLVM::ReturnOp>(op, adaptor.getOperands());
    return success();
  }
};

struct AddOpLowering final : public OpConversionPattern<bir::AddOp> {
  using OpConversionPattern<bir::AddOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::AddOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::AddOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else if (mlir::isa<FloatType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::FAddOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct SubOpLowering final : public OpConversionPattern<bir::SubOp> {
  using OpConversionPattern<bir::SubOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::SubOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::SubOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else if (mlir::isa<FloatType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::FSubOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct MulOpLowering final : public OpConversionPattern<bir::MulOp> {
  using OpConversionPattern<bir::MulOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::MulOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::MulOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else if (mlir::isa<FloatType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::FMulOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct DivOpLowering final : public OpConversionPattern<bir::DivOp> {
  using OpConversionPattern<bir::DivOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::DivOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::SDivOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else if (mlir::isa<FloatType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::FDivOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct ModOpLowering final : public OpConversionPattern<bir::ModOp> {
  using OpConversionPattern<bir::ModOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::ModOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    // TODO: The operation in bir is Mod, while in LLVM its Rem.
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::SRemOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else if (mlir::isa<FloatType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::FRemOp>(op, type, adaptor.getLhs(),
                                                adaptor.getRhs());
    } else {
      return failure();
    }

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
    addConversion([this](mlir::FunctionType type) -> mlir::Type {
      SmallVector<mlir::Type> inputs;
      for (mlir::Type t : type.getInputs())
        inputs.push_back(convertType(t));

      mlir::Type result;
      if (type.getNumResults() == 0)
        result = LLVM::LLVMVoidType::get(type.getContext());
      else if (type.getNumResults() == 1)
        result = convertType(type.getResult(0));
      else {
        // TODO: decide on num results
        return mlir::Type();
      }

      return LLVM::LLVMFunctionType::get(result, inputs);
    });
  }
};

} // namespace

void belalang::bir::populateBelalangBIRToLLVMPatterns(
    mlir::RewritePatternSet &patterns, mlir::TypeConverter &typeConverter) {
  patterns.add<ConstantOpLowering, FuncOpLowering, CallOpLowering,
               ReturnOpLowering, AddOpLowering, SubOpLowering, MulOpLowering,
               DivOpLowering, ModOpLowering>(typeConverter,
                                             patterns.getContext());
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
