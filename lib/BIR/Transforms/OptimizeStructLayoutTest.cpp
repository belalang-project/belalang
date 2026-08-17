#include "belalang/BIR/Transforms/OptimizeStructLayout.h"

#include "gtest/gtest.h"

namespace belalang {
namespace bir {
namespace {

TEST(ComputeStructReorderTest, SortsByAlignmentDescending) {
  EXPECT_EQ(computeStructReorder({{1, 1}, {8, 8}, {1, 1}}),
            llvm::ArrayRef<int32_t>({1, 0, 2}));
}

TEST(ComputeStructReorderTest, SortsBySizeWhenAlignmentTies) {
  EXPECT_EQ(computeStructReorder({{8, 8}, {8, 16}, {1, 1}}),
            llvm::ArrayRef<int32_t>({1, 0, 2}));
}

TEST(ComputeStructReorderTest, PreservesStableOrderForEqualMembers) {
  EXPECT_EQ(computeStructReorder({{8, 8}, {8, 8}, {1, 1}}),
            llvm::ArrayRef<int32_t>({0, 1, 2}));
}

TEST(ComputeStructReorderTest, HandlesEmptyStruct) {
  EXPECT_TRUE(computeStructReorder({}).empty());
}

} // namespace
} // namespace bir
} // namespace belalang
