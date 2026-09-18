#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_GCTOLLVMPASS
#include "mlir/Dialect/GC/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace mlir::gc;

static LLVM::LLVMFuncOp getOrCreateRuntimeFunction(ModuleOp module,
                                                   Location loc, StringRef name,
                                                   LLVM::LLVMFunctionType type,
                                                   OpBuilder &builder) {
  if (auto function = module.lookupSymbol<LLVM::LLVMFuncOp>(name))
    return function;

  OpBuilder::InsertionGuard guard(builder);
  builder.setInsertionPointToStart(module.getBody());
  return LLVM::LLVMFuncOp::create(builder, loc, name, type);
}

static std::string getPointerOffsetsGlobalName(ArrayRef<int32_t> offsets) {
  llvm::hash_code hash = llvm::hash_combine_range(offsets.begin(),
                                                  offsets.end());
  return "gc.ptr_offsets." + std::to_string(static_cast<size_t>(hash));
}

static Value getOrCreatePointerOffsetsGlobal(Location loc, ModuleOp module,
                                             ArrayRef<int32_t> offsets,
                                             OpBuilder &builder) {
  auto ptrType = LLVM::LLVMPointerType::get(module.getContext());
  if (offsets.empty())
    return LLVM::ZeroOp::create(builder, loc, ptrType);

  MLIRContext *ctx = module.getContext();
  std::string globalName = getPointerOffsetsGlobalName(offsets);
  auto i32Type = IntegerType::get(ctx, 32);
  auto arrayType = LLVM::LLVMArrayType::get(i32Type, offsets.size());

  {
    OpBuilder::InsertionGuard guard(builder);
    builder.setInsertionPointToStart(module.getBody());
    if (!module.lookupSymbol<LLVM::GlobalOp>(globalName)) {
      SmallVector<Attribute> values;
      for (int32_t offset : offsets)
        values.push_back(builder.getI32IntegerAttr(offset));
      LLVM::GlobalOp::create(builder, loc, arrayType, true,
                             LLVM::Linkage::Private, globalName,
                             builder.getArrayAttr(values));
    }
  }

  return LLVM::AddressOfOp::create(builder, loc, ptrType, globalName);
}

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

struct AllocOpLowering final : OpConversionPattern<AllocOp> {
  using OpConversionPattern<AllocOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(AllocOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto module = op->getParentOfType<ModuleOp>();
    auto function = op->getParentOfType<FunctionOpInterface>();

    if (!module || !function)
      return rewriter.notifyMatchFailure(
          op, "expected module and function parents");

    auto sizeAttr = op->getAttrOfType<IntegerAttr>("size");
    auto offsetsAttr = op->getAttrOfType<DenseI32ArrayAttr>("pointer_offsets");

    if (!sizeAttr || !offsetsAttr)
      return op.emitOpError(
          "requires 'size' and 'pointer_offsets' attributes for LLVM lowering");

    auto allocName = module->getAttrOfType<StringAttr>(kRuntimeAllocAttrName);
    auto pushName = module->getAttrOfType<StringAttr>(
        kRuntimePushRootsAttrName);
    auto popName = module->getAttrOfType<StringAttr>(kRuntimePopRootsAttrName);

    if (!allocName || !pushName || !popName)
      return op.emitOpError("requires GC runtime symbol configuration on the "
                            "parent module");

    Location loc = op.getLoc();
    MLIRContext *ctx = op.getContext();
    auto ptrType = LLVM::LLVMPointerType::get(ctx);
    auto i64Type = rewriter.getI64Type();
    auto voidType = LLVM::LLVMVoidType::get(ctx);

    getOrCreateRuntimeFunction(
        module, loc, allocName.getValue(),
        LLVM::LLVMFunctionType::get(ptrType, {i64Type, i64Type, ptrType}),
        rewriter);

    SmallVector<Value> rootSlots;
    Value rootCount;
    Value rootsArray;
    if (!adaptor.getRoots().empty()) {
      OpBuilder entryBuilder(ctx);
      entryBuilder.setInsertionPointToStart(
          &function.getFunctionBody().front());
      rootCount = LLVM::ConstantOp::create(
          entryBuilder, loc, i64Type,
          entryBuilder.getI64IntegerAttr(adaptor.getRoots().size()));
      rootsArray = LLVM::AllocaOp::create(entryBuilder, loc, ptrType, ptrType,
                                          rootCount);
      Value one = LLVM::ConstantOp::create(entryBuilder, loc, i64Type,
                                           entryBuilder.getI64IntegerAttr(1));

      for (auto [index, root] : llvm::enumerate(adaptor.getRoots())) {
        Value slot = LLVM::AllocaOp::create(entryBuilder, loc, ptrType, ptrType,
                                            one);
        rootSlots.push_back(slot);
        SmallVector<LLVM::GEPArg> indices = {static_cast<int32_t>(index)};
        Value element = LLVM::GEPOp::create(
            entryBuilder, loc, ptrType, ptrType, rootsArray, indices,
            LLVM::GEPNoWrapFlags::inbounds | LLVM::GEPNoWrapFlags::nuw);
        LLVM::StoreOp::create(entryBuilder, loc, slot, element);
        LLVM::StoreOp::create(rewriter, loc, root, slot);
      }

      getOrCreateRuntimeFunction(
          module, loc, pushName.getValue(),
          LLVM::LLVMFunctionType::get(voidType, {i64Type, ptrType}), rewriter);
      LLVM::CallOp::create(rewriter, loc, TypeRange{},
                           FlatSymbolRefAttr::get(ctx, pushName.getValue()),
                           ValueRange{rootCount, rootsArray});
    }

    Value size = LLVM::ConstantOp::create(rewriter, loc, i64Type, sizeAttr);
    ArrayRef<int32_t> offsets = offsetsAttr.asArrayRef();
    Value pointerCount = LLVM::ConstantOp::create(
        rewriter, loc, i64Type, rewriter.getI64IntegerAttr(offsets.size()));
    Value pointerOffsets = getOrCreatePointerOffsetsGlobal(loc, module, offsets,
                                                           rewriter);
    auto allocated = LLVM::CallOp::create(
        rewriter, loc, ptrType,
        FlatSymbolRefAttr::get(ctx, allocName.getValue()),
        ValueRange{size, pointerCount, pointerOffsets});

    SmallVector<Value> results = {allocated.getResult()};
    if (!rootSlots.empty()) {
      getOrCreateRuntimeFunction(module, loc, popName.getValue(),
                                 LLVM::LLVMFunctionType::get(voidType, {}),
                                 rewriter);
      LLVM::CallOp::create(rewriter, loc, TypeRange{},
                           FlatSymbolRefAttr::get(ctx, popName.getValue()),
                           ValueRange{});
      for (Value slot : rootSlots)
        results.push_back(LLVM::LoadOp::create(rewriter, loc, ptrType, slot));
    }

    rewriter.replaceOp(op, results);
    return success();
  }
};

void populateGCToLLVMPatterns(mlir::RewritePatternSet &patterns,
                              mlir::TypeConverter &typeConverter) {
  patterns.add<CallOpConversion, AllocOpLowering, AllocaOpLowering>(
      typeConverter, patterns.getContext());
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
    target.addIllegalDialect<GCDialect>();
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
