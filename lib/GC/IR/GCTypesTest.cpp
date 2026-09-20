#include "mlir/Dialect/GC/IR/GC.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "gtest/gtest.h"

namespace mlir::gc {
namespace {

class GCIRTest : public ::testing::Test {
protected:
  MLIRContext context;
  ModuleOp module;
  OpBuilder builder;

  GCIRTest()
      : module(ModuleOp::create(UnknownLoc::get(&context))), builder(&context) {
    context.getOrLoadDialect<GCDialect>();
    context.getOrLoadDialect<LLVM::LLVMDialect>();
    builder.setInsertionPointToStart(module.getBody());
  }

  Location getLoc() { return UnknownLoc::get(&context); }
};

TEST_F(GCIRTest, PtrTypeMatchesLLVMPointerDataLayout) {
  DataLayout dataLayout = DataLayout::closest(module);
  Type ptr = PtrType::get(&context, builder.getI64Type());
  Type llvmPtr = LLVM::LLVMPointerType::get(&context);

  EXPECT_EQ(dataLayout.getTypeSizeInBits(ptr),
            dataLayout.getTypeSizeInBits(llvmPtr));
  EXPECT_EQ(dataLayout.getTypeABIAlignment(ptr),
            dataLayout.getTypeABIAlignment(llvmPtr));
}

TEST_F(GCIRTest, AllocaHasUninitializedPromotableSlot) {
  Type ptr = PtrType::get(&context, builder.getI64Type());
  AllocaOp alloca = AllocaOp::create(builder, getLoc(), ptr);

  SmallVector<MemorySlot> slots = alloca.getPromotableSlots();
  ASSERT_EQ(slots.size(), 1u);
  EXPECT_EQ(slots.front().ptr, alloca.getResult());
  EXPECT_EQ(slots.front().elemType, builder.getI64Type());
  EXPECT_FALSE(alloca.getDefaultValue(slots.front(), builder));
}

TEST_F(GCIRTest, LoadReadsFromAddress) {
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

TEST_F(GCIRTest, StoreWritesToAddress) {
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

TEST_F(GCIRTest, AllocAllocatesPrimaryResult) {
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

TEST_F(GCIRTest, AllocaAllocatesAutomaticStorage) {
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

TEST_F(GCIRTest, CallHasUnknownEffects) {
  CallOp call = CallOp::create(builder, getLoc(),
                               FlatSymbolRefAttr::get(&context, "callee"),
                               TypeRange{}, ValueRange{});

  EXPECT_FALSE(isa<MemoryEffectOpInterface>(call.getOperation()));
  EXPECT_TRUE(hasUnknownEffects(call));
}

} // namespace
} // namespace mlir::gc
