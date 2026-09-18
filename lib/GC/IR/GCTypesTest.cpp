#include "mlir/Dialect/GC/IR/GC.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

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

} // namespace
} // namespace mlir::gc
