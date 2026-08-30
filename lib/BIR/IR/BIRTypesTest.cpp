#include "belalang/BIR/IR/BIR.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "gtest/gtest.h"

// NOTE: Some of these tests assume 64-bit data layout which may not work on
// 32-bit systems.

namespace belalang {
namespace bir {
namespace {

class BIRTypesTest : public ::testing::Test {
protected:
  mlir::MLIRContext context;
  mlir::ModuleOp module;

  BIRTypesTest()
      : module(mlir::ModuleOp::create(mlir::UnknownLoc::get(&context))) {
    context.getOrLoadDialect<BIRDialect>();
    context.getOrLoadDialect<mlir::LLVM::LLVMDialect>();
  }

  mlir::DataLayout getDataLayout() {
    return mlir::DataLayout::closest(module.getOperation());
  }
};

TEST_F(BIRTypesTest, IntType) {
  auto dl = getDataLayout();
  auto ty = IntType::get(&context);

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, FloatType) {
  auto dl = getDataLayout();
  auto ty = FloatType::get(&context);

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, BoolType) {
  auto dl = getDataLayout();
  auto ty = BoolType::get(&context);

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 8);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 1);
}

TEST_F(BIRTypesTest, StringType) {
  auto dl = getDataLayout();
  auto ty = StringType::get(&context);

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 128);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, RefTypeMatchesLLVMPointer) {
  auto dl = getDataLayout();
  auto ty = RefType::get(&context, IntType::get(&context));
  auto llvmPtr = mlir::LLVM::LLVMPointerType::get(&context);

  EXPECT_EQ(dl.getTypeSizeInBits(ty), dl.getTypeSizeInBits(llvmPtr));
  EXPECT_EQ(dl.getTypeABIAlignment(ty), dl.getTypeABIAlignment(llvmPtr));
}

TEST_F(BIRTypesTest, EmptyStructType) {
  auto dl = getDataLayout();
  auto ty = StructType::get(&context, {},
                            mlir::StringAttr::get(&context, "Empty"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 0);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 1);
}

TEST_F(BIRTypesTest, StructTypeWithLeadingPadding) {
  auto dl = getDataLayout();
  auto ty = StructType::get(&context,
                            {BoolType::get(&context), IntType::get(&context)},
                            mlir::StringAttr::get(&context, "BoolInt"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 128);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, StructTypeWithTrailingPadding) {
  auto dl = getDataLayout();
  auto ty = StructType::get(&context,
                            {IntType::get(&context), BoolType::get(&context)},
                            mlir::StringAttr::get(&context, "IntBool"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 128);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, StructTypeWithOnlyByteAlignedMembers) {
  auto dl = getDataLayout();
  auto ty = StructType::get(&context,
                            {BoolType::get(&context), BoolType::get(&context),
                             BoolType::get(&context)},
                            mlir::StringAttr::get(&context, "Bools"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 24);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 1);
}

TEST_F(BIRTypesTest, StructTypeWithStringAndBool) {
  auto dl = getDataLayout();
  auto ty = StructType::get(
      &context, {StringType::get(&context), BoolType::get(&context)},
      mlir::StringAttr::get(&context, "StringBool"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 192);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, StructTypeWithReferenceAndBool) {
  auto dl = getDataLayout();
  auto ty = StructType::get(
      &context,
      {RefType::get(&context, IntType::get(&context)), BoolType::get(&context)},
      mlir::StringAttr::get(&context, "RefBool"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 128);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, StructTypeReorderUsesPhysicalOrder) {
  auto dl = getDataLayout();
  auto ty = StructType::get(&context,
                            {BoolType::get(&context), IntType::get(&context),
                             BoolType::get(&context), IntType::get(&context)},
                            mlir::StringAttr::get(&context, "Reordered"),
                            {2, 0, 3, 1});

  EXPECT_EQ(ty.getReorder(), llvm::ArrayRef<int32_t>({2, 0, 3, 1}));
  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 192);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, NestedStructType) {
  auto dl = getDataLayout();
  auto inner = StructType::get(
      &context, {BoolType::get(&context), IntType::get(&context)},
      mlir::StringAttr::get(&context, "Inner"), {});
  auto outer = StructType::get(
      &context, {BoolType::get(&context), inner, BoolType::get(&context)},
      mlir::StringAttr::get(&context, "Outer"), {});

  EXPECT_EQ(dl.getTypeSizeInBits(outer).getFixedValue(), 256);
  EXPECT_EQ(dl.getTypeABIAlignment(outer), 8);
}

TEST_F(BIRTypesTest, EmptyArrayType) {
  auto dl = getDataLayout();
  auto ty = ArrayType::get(&context, {});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, ArrayTypeWithOnlyByteAlignedMembers) {
  auto dl = getDataLayout();
  auto ty = ArrayType::get(&context,
                           {BoolType::get(&context), BoolType::get(&context),
                            BoolType::get(&context)});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, ArrayTypeWithLeadingPadding) {
  auto dl = getDataLayout();
  auto ty = ArrayType::get(&context,
                           {BoolType::get(&context), IntType::get(&context)});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, ArrayTypeWithTrailingPadding) {
  auto dl = getDataLayout();
  auto ty = ArrayType::get(&context,
                           {IntType::get(&context), BoolType::get(&context)});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, ArrayTypeWithStringAndReference) {
  auto dl = getDataLayout();
  auto ty = ArrayType::get(
      &context,
      {StringType::get(&context), RefType::get(&context, IntType::get(&context))});

  EXPECT_EQ(dl.getTypeSizeInBits(ty).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(ty), 8);
}

TEST_F(BIRTypesTest, NestedArrayType) {
  auto dl = getDataLayout();
  auto inner = ArrayType::get(
      &context, {BoolType::get(&context), IntType::get(&context)});
  auto outer = ArrayType::get(
      &context, {BoolType::get(&context), inner, BoolType::get(&context)});

  EXPECT_EQ(dl.getTypeSizeInBits(outer).getFixedValue(), 64);
  EXPECT_EQ(dl.getTypeABIAlignment(outer), 8);
}

} // namespace
} // namespace bir
} // namespace belalang
