#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "belalang/BRT/BRT.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"

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
    if (auto intAttr = llvm::dyn_cast<bir::IntegerAttr>(value)) {
      value = rewriter.getIntegerAttr(type, intAttr.getValue());
    } else if (auto floatAttr = llvm::dyn_cast<bir::FloatAttr>(value)) {
      value = rewriter.getFloatAttr(type, floatAttr.getValue());
    } else if (auto strAttr = llvm::dyn_cast<bir::StringAttr>(value)) {
      auto module = op->getParentOfType<mlir::ModuleOp>();
      auto ctx = op->getContext();
      StringRef str = strAttr.getValue();

      std::string globalName = "str." + std::to_string(llvm::hash_value(str));

      LLVM::GlobalOp global;
      {
        OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(module.getBody());
        auto arrTy = LLVM::LLVMArrayType::get(IntegerType::get(ctx, 8), str.size());

        global = module.lookupSymbol<LLVM::GlobalOp>(globalName);
        if (!global) {
          auto attr = mlir::StringAttr::get(ctx, str);
          global = LLVM::GlobalOp::create(rewriter, op.getLoc(), arrTy, true, LLVM::Linkage::Private, globalName, attr);
        }
      }

      auto addrOfOp = LLVM::AddressOfOp::create(rewriter, op.getLoc(), global);

      auto lenTy = IntegerType::get(ctx, 64);
      auto len = LLVM::ConstantOp::create(rewriter, op.getLoc(), lenTy, str.size());

      auto container = LLVM::UndefOp::create(rewriter, op.getLoc(), type);
      auto c1 = LLVM::InsertValueOp::create(rewriter, op.getLoc(), container.getType(), container, addrOfOp, rewriter.getDenseI64ArrayAttr({0}));
      rewriter.replaceOpWithNewOp<LLVM::InsertValueOp>(op, c1.getType(), c1, len, rewriter.getDenseI64ArrayAttr({1}));

      return success();
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

struct VarDeclareOpLowering final : public OpConversionPattern<bir::VarDeclareOp> {
  using OpConversionPattern<bir::VarDeclareOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::VarDeclareOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto refType = mlir::cast<bir::RefType>(op.getType());
    auto elType = refType.getEl();
    auto loc = op.getLoc();
    auto ctx = op.getContext();

    int64_t elSize;
    if (mlir::isa<bir::IntType>(elType))
      elSize = 8;
    else if (mlir::isa<bir::FloatType>(elType))
      elSize = 8;
    else if (mlir::isa<bir::StringType>(elType))
      elSize = 16;
    else
      return failure();

    auto module = op->getParentOfType<mlir::ModuleOp>();
    if (!module.lookupSymbol(brt::BRT_MMTK_ALLOC)) {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(module.getBody());
      auto funcType = LLVM::LLVMFunctionType::get(
          LLVM::LLVMPointerType::get(ctx), IntegerType::get(ctx, 64));
      OperationState funcState(UnknownLoc::get(ctx),
                               LLVM::LLVMFuncOp::getOperationName());
      LLVM::LLVMFuncOp::build(rewriter, funcState, brt::BRT_MMTK_ALLOC,
                              funcType);
      rewriter.create(funcState);
    }

    auto i64Type = IntegerType::get(ctx, 64);
    auto sizeVal =
        LLVM::ConstantOp::create(rewriter, loc, i64Type, rewriter.getI64IntegerAttr(elSize));

    auto ptrType = LLVM::LLVMPointerType::get(ctx);
    auto calleeAttr = FlatSymbolRefAttr::get(ctx, brt::BRT_MMTK_ALLOC);
    auto allocCall = LLVM::CallOp::create(rewriter, loc, ptrType, calleeAttr, sizeVal.getResult());

    rewriter.replaceOp(op, allocCall.getResult());
    return success();
  };
};

struct VarStoreOpLowering final : public OpConversionPattern<bir::VarStoreOp> {
  using OpConversionPattern<bir::VarStoreOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::VarStoreOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    if (auto str = llvm::dyn_cast<bir::StringType>(adaptor.getSrc().getType())) {
      auto type = getTypeConverter()->convertType(adaptor.getSrc().getType());
      LLVM::UndefOp::create(rewriter, op.getLoc(), type);
      rewriter.eraseOp(op);
      return success();
    }

    rewriter.replaceOpWithNewOp<LLVM::StoreOp>(op, adaptor.getSrc(),
                                               adaptor.getDest());
    return success();
  };
};

struct VarLoadOpLowering final : public OpConversionPattern<bir::VarLoadOp> {
  using OpConversionPattern<bir::VarLoadOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::VarLoadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    // TODO: this should return the GC-allocated pointer instead of loading
    // a new one
    rewriter.replaceOpWithNewOp<LLVM::LoadOp>(op, type, adaptor.getRef());
    return success();
  };
};

struct BIRToLLVMTypeConverter : public mlir::TypeConverter {
  BIRToLLVMTypeConverter() {
    addConversion([](bir::IntType ty) {
      return mlir::IntegerType::get(ty.getContext(), 64);
    });
    addConversion([](bir::FloatType ty) {
      return mlir::Float64Type::get(ty.getContext());
    });
    addConversion([](bir::StringType ty) {
      mlir::MLIRContext *ctx = ty.getContext();
      mlir::Type ptrType = mlir::LLVM::LLVMPointerType::get(ctx);
      mlir::Type iType = mlir::IntegerType::get(ctx, 64);
      return LLVM::LLVMStructType::getLiteral(ctx, {ptrType, iType});
    });
    addConversion([](bir::RefType ty) {
      return LLVM::LLVMPointerType::get(ty.getContext());
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
               DivOpLowering, ModOpLowering, VarDeclareOpLowering,
               VarStoreOpLowering, VarLoadOpLowering>(typeConverter,
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
      return;
    }

    auto module = mlir::dyn_cast<mlir::ModuleOp>(getOperation());
    if (!module)
      return;

    MLIRContext *ctx = &getContext();
    auto mainFunc = module.lookupSymbol<LLVM::LLVMFuncOp>("main");
    if (!mainFunc || mainFunc.isExternal())
      return;

    if (!module.lookupSymbol(brt::BRT_MMTK_INIT)) {
      OpBuilder builder(&module.getBodyRegion().front(),
                        module.getBodyRegion().front().begin());
      auto voidType = LLVM::LLVMVoidType::get(ctx);
      auto funcType = LLVM::LLVMFunctionType::get(voidType, {});
      OperationState funcState(UnknownLoc::get(ctx),
                               LLVM::LLVMFuncOp::getOperationName());
      LLVM::LLVMFuncOp::build(builder, funcState, brt::BRT_MMTK_INIT,
                              funcType);
      builder.create(funcState);
    }

    {
      Block &entryBlock = mainFunc.getBody().front();
      OpBuilder builder(&entryBlock, entryBlock.begin());
      auto calleeAttr = FlatSymbolRefAttr::get(ctx, brt::BRT_MMTK_INIT);
      OperationState callState(UnknownLoc::get(ctx),
                               LLVM::CallOp::getOperationName());
      LLVM::CallOp::build(builder, callState, TypeRange{}, calleeAttr,
                          ValueRange{});
      builder.create(callState);
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangBIRToLLVMPass() {
  return std::make_unique<BelalangBIRToLLVMPass>();
}
