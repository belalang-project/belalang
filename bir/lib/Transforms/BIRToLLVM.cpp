#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "belalang/BIR/BRTUtils.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"

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
        auto arrTy = LLVM::LLVMArrayType::get(mlir::IntegerType::get(ctx, 8), str.size());

        global = module.lookupSymbol<LLVM::GlobalOp>(globalName);
        if (!global) {
          auto attr = mlir::StringAttr::get(ctx, str);
          global = LLVM::GlobalOp::create(rewriter, op.getLoc(), arrTy, true, LLVM::Linkage::Private, globalName, attr);
        }
      }

      auto addrOfOp = LLVM::AddressOfOp::create(rewriter, op.getLoc(), global);

      auto lenTy = mlir::IntegerType::get(ctx, 64);
      auto len = LLVM::ConstantOp::create(rewriter, op.getLoc(), lenTy, str.size());

      auto container = LLVM::UndefOp::create(rewriter, op.getLoc(), type);
      auto c1 = LLVM::InsertValueOp::create(rewriter, op.getLoc(), container.getType(), container, addrOfOp, rewriter.getDenseI64ArrayAttr({0}));
      rewriter.replaceOpWithNewOp<LLVM::InsertValueOp>(op, c1.getType(), c1, len, rewriter.getDenseI64ArrayAttr({1}));

      return success();
    } else if (auto fnAttr = llvm::dyn_cast<bir::FnAttr>(value)) {
      FlatSymbolRefAttr attr = fnAttr.getValue();
      rewriter.replaceOpWithNewOp<LLVM::AddressOfOp>(op, type, attr);
      return success();
    } else if (auto boolAttr = llvm::dyn_cast<bir::BoolAttr>(value)) {
      value = rewriter.getIntegerAttr(type, boolAttr.getValue());
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
    auto srcType = op.getFunctionType();

    SmallVector<mlir::Type> inputTypes;
    for (auto t : srcType.getInputs()) {
      auto converted = getTypeConverter()->convertType(t);
      if (!converted)
        return failure();
      inputTypes.push_back(converted);
    }

    mlir::Type resultType;
    if (srcType.getNumResults() == 0)
      resultType = LLVM::LLVMVoidType::get(getContext());
    else if (srcType.getNumResults() == 1)
      resultType = getTypeConverter()->convertType(srcType.getResult(0));
    else {
      // TODO: decide on num results
      return failure();
    }

    auto llvmFuncType = LLVM::LLVMFunctionType::get(resultType, inputTypes);

    TypeConverter::SignatureConversion signatureConverter(
        op.getFunctionType().getNumInputs());
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

struct CallIndirectOpLowering final
    : public OpConversionPattern<bir::CallIndirectOp> {
  using OpConversionPattern<bir::CallIndirectOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::CallIndirectOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto funcType = mlir::cast<mlir::FunctionType>(op.getCallee().getType());

    SmallVector<mlir::Type> paramTypes;
    for (auto t : funcType.getInputs())
      paramTypes.push_back(getTypeConverter()->convertType(t));

    SmallVector<mlir::Type> resultTypes;
    for (auto t : funcType.getResults())
      resultTypes.push_back(getTypeConverter()->convertType(t));
    mlir::Type llvmResultType = resultTypes.empty()
                                    ? LLVM::LLVMVoidType::get(getContext())
                                    : resultTypes.front();

    auto llvmFuncType = LLVM::LLVMFunctionType::get(llvmResultType, paramTypes);

    rewriter.replaceOpWithNewOp<LLVM::CallOp>(op, llvmFuncType,
                                              adaptor.getOperands());
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

    if (mlir::isa<mlir::IntegerType>(type)) {
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

    if (mlir::isa<mlir::IntegerType>(type)) {
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

    if (mlir::isa<mlir::IntegerType>(type)) {
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

    if (mlir::isa<mlir::IntegerType>(type)) {
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

    if (mlir::isa<mlir::IntegerType>(type)) {
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

struct AndOpLowering final : public OpConversionPattern<bir::AndOp> {
  using OpConversionPattern<bir::AndOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::AndOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<mlir::IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::AndOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct OrOpLowering final : public OpConversionPattern<bir::OrOp> {
  using OpConversionPattern<bir::OrOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::OrOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<mlir::IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::OrOp>(op, type, adaptor.getLhs(),
                                              adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct XorOpLowering final : public OpConversionPattern<bir::XorOp> {
  using OpConversionPattern<bir::XorOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::XorOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<mlir::IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::XOrOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct ShlOpLowering final : public OpConversionPattern<bir::ShlOp> {
  using OpConversionPattern<bir::ShlOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::ShlOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<mlir::IntegerType>(type)) {
      rewriter.replaceOpWithNewOp<LLVM::ShlOp>(op, type, adaptor.getLhs(),
                                               adaptor.getRhs());
    } else {
      return failure();
    }

    return success();
  }
};

struct ShrOpLowering final : public OpConversionPattern<bir::ShrOp> {
  using OpConversionPattern<bir::ShrOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::ShrOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto type = getTypeConverter()->convertType(op.getType());
    if (!type)
      return failure();

    if (mlir::isa<mlir::IntegerType>(type)) {
      // TODO: lshr? we currently only use signed integers
      rewriter.replaceOpWithNewOp<LLVM::AShrOp>(op, type, adaptor.getLhs(),
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
    else if (mlir::isa<bir::BoolType>(elType))
      elSize = 8;
    else if (mlir::isa<mlir::FunctionType>(elType))
      elSize = 8;
    else
      return failure();

    auto module = op->getParentOfType<mlir::ModuleOp>();
    if (!module.lookupSymbol(kGCAlloc)) {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(module.getBody());
      auto funcType = LLVM::LLVMFunctionType::get(
          LLVM::LLVMPointerType::get(ctx), mlir::IntegerType::get(ctx, 64));
      OperationState funcState(UnknownLoc::get(ctx),
                               LLVM::LLVMFuncOp::getOperationName());
      LLVM::LLVMFuncOp::build(rewriter, funcState, kGCAlloc,
                              funcType);
      rewriter.create(funcState);
    }

    auto i64Type = mlir::IntegerType::get(ctx, 64);
    auto sizeVal =
        LLVM::ConstantOp::create(rewriter, loc, i64Type, rewriter.getI64IntegerAttr(elSize));

    auto ptrType = LLVM::LLVMPointerType::get(ctx);
    auto calleeAttr = FlatSymbolRefAttr::get(ctx, kGCAlloc);
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

struct CondBrLowering final : public OpConversionPattern<bir::CondBrOp> {
  using OpConversionPattern<bir::CondBrOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::CondBrOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<cf::CondBranchOp>(
        op, adaptor.getCond(), op.getDestTrue(), op.getDestOperandsTrue(),
        op.getDestFalse(), op.getDestOperandsFalse());
    return success();
  }
};

struct CmpOpLowering final : public OpConversionPattern<bir::CmpOp> {
  using OpConversionPattern<bir::CmpOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bir::CmpOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    mlir::Type ty = op.getLhs().getType();
    mlir::Type llvmTy = getTypeConverter()->convertType(ty);
    if (!llvmTy)
      return failure();

    mlir::Type resultTy =
        getTypeConverter()->convertType(op.getResult().getType());

    if (mlir::isa<bir::IntType>(ty)) {
      LLVM::ICmpPredicate pred;
      switch (op.getKind()) {
      case bir::CmpOpKind::eq:
        pred = LLVM::ICmpPredicate::eq;
        break;
      case bir::CmpOpKind::ne:
        pred = LLVM::ICmpPredicate::ne;
        break;
      case bir::CmpOpKind::lt:
        pred = LLVM::ICmpPredicate::slt;
        break;
      case bir::CmpOpKind::le:
        pred = LLVM::ICmpPredicate::sle;
        break;
      case bir::CmpOpKind::gt:
        pred = LLVM::ICmpPredicate::sgt;
        break;
      case bir::CmpOpKind::ge:
        pred = LLVM::ICmpPredicate::sge;
        break;
      default:
        return failure();
      }
      rewriter.replaceOpWithNewOp<LLVM::ICmpOp>(
          op, resultTy, pred, adaptor.getLhs(), adaptor.getRhs());
      return success();
    }

    if (mlir::isa<bir::FloatType>(ty)) {
      LLVM::FCmpPredicate pred;
      switch (op.getKind()) {
      case bir::CmpOpKind::eq:
        pred = LLVM::FCmpPredicate::oeq;
        break;
      case bir::CmpOpKind::ne:
        pred = LLVM::FCmpPredicate::one;
        break;
      case bir::CmpOpKind::lt:
        pred = LLVM::FCmpPredicate::olt;
        break;
      case bir::CmpOpKind::le:
        pred = LLVM::FCmpPredicate::ole;
        break;
      case bir::CmpOpKind::gt:
        pred = LLVM::FCmpPredicate::ogt;
        break;
      case bir::CmpOpKind::ge:
        pred = LLVM::FCmpPredicate::oge;
        break;
      default:
        return failure();
      }
      rewriter.replaceOpWithNewOp<LLVM::FCmpOp>(
          op, resultTy, pred, adaptor.getLhs(), adaptor.getRhs());
      return success();
    }

    return op.emitOpError("unsupported type");
  }
};

struct BIRToLLVMTypeConverter : public mlir::LLVMTypeConverter {
  BIRToLLVMTypeConverter(mlir::MLIRContext *ctx)
      : mlir::LLVMTypeConverter(ctx) {
    addConversion([](bir::IntType ty) {
      return mlir::IntegerType::get(ty.getContext(), 64);
    });
    addConversion([](bir::FloatType ty) {
      return mlir::Float64Type::get(ty.getContext());
    });
    addConversion([](bir::BoolType ty) {
      return mlir::IntegerType::get(ty.getContext(), 1);
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
    addConversion([](mlir::FunctionType type) -> mlir::Type {
      return LLVM::LLVMPointerType::get(type.getContext());
    });
  }
};

} // namespace

void belalang::bir::populateBelalangBIRToLLVMPatterns(
    mlir::RewritePatternSet &patterns, mlir::TypeConverter &typeConverter) {
  patterns.add<ConstantOpLowering, FuncOpLowering, CallOpLowering,
               CallIndirectOpLowering, ReturnOpLowering, AddOpLowering,
               SubOpLowering, MulOpLowering, DivOpLowering, ModOpLowering,
               AndOpLowering, OrOpLowering, XorOpLowering, ShlOpLowering,
               ShrOpLowering, VarDeclareOpLowering, VarStoreOpLowering,
               VarLoadOpLowering, CondBrLowering, CmpOpLowering>(
      typeConverter, patterns.getContext());
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangBIRToLLVMPass
    : public impl::BelalangBIRToLLVMPassBase<BelalangBIRToLLVMPass> {
  using impl::BelalangBIRToLLVMPassBase<
      BelalangBIRToLLVMPass>::BelalangBIRToLLVMPassBase;

  void runOnOperation() override {
    BIRToLLVMTypeConverter typeConverter(&getContext());

    mlir::ConversionTarget target(getContext());
    target.addLegalDialect<mlir::LLVM::LLVMDialect, mlir::BuiltinDialect>();
    target.addIllegalDialect<bir::BIRDialect>();

    mlir::RewritePatternSet patterns(&getContext());
    belalang::bir::populateBelalangBIRToLLVMPatterns(patterns, typeConverter);
    mlir::cf::populateControlFlowToLLVMConversionPatterns(typeConverter,
                                                          patterns);

    if (mlir::failed(mlir::applyFullConversion(getOperation(), target,
                                                  std::move(patterns)))) {
      signalPassFailure();
      return;
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangBIRToLLVMPass() {
  return std::make_unique<BelalangBIRToLLVMPass>();
}
