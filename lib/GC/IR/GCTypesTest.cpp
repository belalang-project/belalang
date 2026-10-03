#include "mlir/Dialect/GC/IR/GC.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "llvm/ADT/STLExtras.h"

#include "gtest/gtest.h"

namespace mlir::gc {
namespace {

class GCTest : public ::testing::Test {
protected:
  MLIRContext context;
  ModuleOp module;
  OpBuilder builder;

  GCTest()
      : module(ModuleOp::create(UnknownLoc::get(&context))), builder(&context) {
    context.getOrLoadDialect<GCDialect>();
    context.getOrLoadDialect<LLVM::LLVMDialect>();
    builder.setInsertionPointToStart(module.getBody());
  }

  Location getLoc() { return UnknownLoc::get(&context); }
};

TEST_F(GCTest, PtrTypeMatchesLLVMPointerDataLayout) {
  DataLayout dataLayout = DataLayout::closest(module);
  Type ptr = PtrType::get(&context, builder.getI64Type());
  Type llvmPtr = LLVM::LLVMPointerType::get(&context);

  EXPECT_EQ(dataLayout.getTypeSizeInBits(ptr),
            dataLayout.getTypeSizeInBits(llvmPtr));
  EXPECT_EQ(dataLayout.getTypeABIAlignment(ptr),
            dataLayout.getTypeABIAlignment(llvmPtr));
}

TEST_F(GCTest, DerivedPtrTypeMatchesLLVMPointerDataLayout) {
  DataLayout dataLayout = DataLayout::closest(module);
  Type ptr = DerivedPtrType::get(&context, builder.getI64Type());
  Type llvmPtr = LLVM::LLVMPointerType::get(&context);

  EXPECT_EQ(dataLayout.getTypeSizeInBits(ptr),
            dataLayout.getTypeSizeInBits(llvmPtr));
  EXPECT_EQ(dataLayout.getTypeABIAlignment(ptr),
            dataLayout.getTypeABIAlignment(llvmPtr));
}

TEST_F(GCTest, GetGCPointerPointee) {
  Type pointee = builder.getI64Type();
  Type ptr = PtrType::get(&context, pointee);
  Type derivedPtr = DerivedPtrType::get(&context, pointee);

  EXPECT_EQ(getGCPointerPointee(ptr), pointee);
  EXPECT_EQ(getGCPointerPointee(derivedPtr), pointee);
  EXPECT_FALSE(getGCPointerPointee(pointee));
}

TEST_F(GCTest, AllocaHasUninitializedPromotableSlot) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp alloca = AllocaOp::create(builder, getLoc(), ptr);

  SmallVector<MemorySlot> slots = alloca.getPromotableSlots();
  ASSERT_EQ(slots.size(), 1u);
  EXPECT_EQ(slots.front().ptr, alloca.getResult());
  EXPECT_EQ(slots.front().elemType, builder.getI64Type());
  EXPECT_FALSE(alloca.getDefaultValue(slots.front(), builder));
}

TEST_F(GCTest, LoadReadsFromAddress) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp alloca = AllocaOp::create(builder, getLoc(), ptr);
  LoadOp load = LoadOp::create(builder, getLoc(), builder.getI64Type(), alloca);

  auto effectOp = cast<MemoryEffectOpInterface>(load.getOperation());
  SmallVector<MemoryEffects::EffectInstance> effects;
  effectOp.getEffects(effects);

  ASSERT_EQ(effects.size(), 1u);
  EXPECT_TRUE(isa<MemoryEffects::Read>(effects.front().getEffect()));
  ASSERT_NE(effects.front().getEffectValue<OpOperand *>(), nullptr);
  EXPECT_EQ(effects.front().getEffectValue<OpOperand *>()->getOperandNumber(),
            0u);
  EXPECT_EQ(effects.front().getResource(),
            SideEffects::DefaultResource::get());
  EXPECT_FALSE(effects.front().getEffectOnFullRegion());
}

TEST_F(GCTest, StoreWritesToAddress) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp alloca = AllocaOp::create(builder, getLoc(), ptr);
  LoadOp value =
      LoadOp::create(builder, getLoc(), builder.getI64Type(), alloca);
  StoreOp store = StoreOp::create(builder, getLoc(), value, alloca);

  auto effectOp = cast<MemoryEffectOpInterface>(store.getOperation());
  SmallVector<MemoryEffects::EffectInstance> effects;
  effectOp.getEffects(effects);

  ASSERT_EQ(effects.size(), 1u);
  EXPECT_TRUE(isa<MemoryEffects::Write>(effects.front().getEffect()));
  ASSERT_NE(effects.front().getEffectValue<OpOperand *>(), nullptr);
  EXPECT_EQ(effects.front().getEffectValue<OpOperand *>()->getOperandNumber(),
            1u);
  EXPECT_EQ(effects.front().getResource(),
            SideEffects::DefaultResource::get());
  EXPECT_FALSE(effects.front().getEffectOnFullRegion());
}

TEST_F(GCTest, AllocAllocatesPrimaryResult) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp root = AllocaOp::create(builder, getLoc(), ptr);
  Type resultTypes[] = {ptr, ptr};
  AllocOp alloc = AllocOp::create(
      builder, getLoc(), TypeRange(resultTypes), IntegerAttr{},
      DenseI32ArrayAttr{}, ValueRange{root});

  auto effectOp = cast<MemoryEffectOpInterface>(alloc.getOperation());
  SmallVector<MemoryEffects::EffectInstance> effects;
  effectOp.getEffects(effects);

  ASSERT_EQ(effects.size(), 1u);
  EXPECT_TRUE(isa<MemoryEffects::Allocate>(effects.front().getEffect()));
  EXPECT_EQ(effects.front().getValue(), alloc.getResult());
  EXPECT_NE(effects.front().getValue(), alloc.getRelocatedRoots().front());
  EXPECT_EQ(effects.front().getResource(),
            SideEffects::DefaultResource::get());
  EXPECT_TRUE(effects.front().getEffectOnFullRegion());
  EXPECT_TRUE(isOpTriviallyDead(alloc));
}

TEST_F(GCTest, AllocImplementsSafepointInterface) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp firstRoot = AllocaOp::create(builder, getLoc(), ptr);
  AllocaOp secondRoot = AllocaOp::create(builder, getLoc(), ptr);
  Type resultTypes[] = {ptr, ptr};
  IntegerAttr size = builder.getI64IntegerAttr(8);
  DenseI32ArrayAttr pointerOffsets = builder.getDenseI32ArrayAttr({0});
  AllocOp alloc = AllocOp::create(builder, getLoc(), TypeRange(resultTypes),
                                  size, pointerOffsets, ValueRange{firstRoot});
  StringAttr marker = builder.getStringAttr("preserved");
  alloc->setDiscardableAttr("test.marker", marker);

  auto safepoint = cast<SafepointOpInterface>(alloc.getOperation());
  ASSERT_EQ(safepoint.getRoots().size(), 1u);
  EXPECT_EQ(safepoint.getRoots().front(), firstRoot.getResult());
  ASSERT_EQ(safepoint.getRelocatedRoots().size(), 1u);

  IRRewriter rewriter(&context);
  SmallVector<Value> roots = {firstRoot.getResult(), secondRoot.getResult()};
  FailureOr<SafepointOpInterface> rebuilt = safepoint.rebuildWithRoots(rewriter,
                                                                       roots);

  ASSERT_TRUE(succeeded(rebuilt));
  auto rebuiltAlloc = cast<AllocOp>(rebuilt->getOperation());
  EXPECT_EQ(rebuiltAlloc->getNextNode(), alloc.getOperation());
  EXPECT_EQ(rebuiltAlloc.getSizeAttr(), size);
  EXPECT_EQ(rebuiltAlloc.getPointerOffsetsAttr(), pointerOffsets);
  EXPECT_EQ(rebuiltAlloc->getDiscardableAttr("test.marker"), marker);
  EXPECT_EQ(rebuilt->getRoots().size(), roots.size());
  EXPECT_EQ(rebuilt->getRelocatedRoots().size(), roots.size());
  EXPECT_TRUE(llvm::equal(rebuilt->getRoots(), roots));
}

TEST_F(GCTest, AllocaAllocatesAutomaticStorage) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp alloca = AllocaOp::create(builder, getLoc(), ptr);

  auto effectOp = cast<MemoryEffectOpInterface>(alloca.getOperation());
  SmallVector<MemoryEffects::EffectInstance> effects;
  effectOp.getEffects(effects);

  ASSERT_EQ(effects.size(), 1u);
  EXPECT_TRUE(isa<MemoryEffects::Allocate>(effects.front().getEffect()));
  EXPECT_EQ(effects.front().getValue(), alloca.getResult());
  EXPECT_EQ(effects.front().getResource(),
            SideEffects::AutomaticAllocationScopeResource::get());
  EXPECT_TRUE(effects.front().getEffectOnFullRegion());
  EXPECT_TRUE(isOpTriviallyDead(alloca));
}

TEST_F(GCTest, CallHasUnknownEffects) {
  CallOp call = CallOp::create(builder, getLoc(),
                               FlatSymbolRefAttr::get(&context, "callee"),
                               TypeRange{}, ValueRange{});

  EXPECT_FALSE(isa<MemoryEffectOpInterface>(call.getOperation()));
  EXPECT_TRUE(hasUnknownEffects(call));
}

} // namespace
} // namespace mlir::gc
