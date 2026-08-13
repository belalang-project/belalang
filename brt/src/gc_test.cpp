#include "gc.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

TEST(GCTest, AllocatesAlignedZeroInitializedMemory) {
  brt_gc_init();

  auto *memory = static_cast<unsigned char *>(brt_gc_alloc(17));

  ASSERT_NE(memory, nullptr);
  EXPECT_EQ(reinterpret_cast<uintptr_t>(memory) % sizeof(uintptr_t), 0);

  for (std::size_t i = 0; i < 17; ++i)
    EXPECT_EQ(memory[i], 0);
}

TEST(GCTest, ReturnsDistinctAllocations) {
  brt_gc_init();

  void *first = brt_gc_alloc(1);
  void *second = brt_gc_alloc(1);

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  EXPECT_NE(first, second);
}
