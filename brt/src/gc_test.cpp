#include "gc.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace {

struct Node {
  void *child;
  std::uint64_t value;
};

const std::uint32_t kNodePointerOffsets[] = {offsetof(Node, child)};

} // namespace

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

TEST(GCTest,
     CollectionUpdatesRoots)
{
  brt_gc_init();

  auto *root =
      static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
  *root = 42;
  auto *original = root;
  void **roots[] = {reinterpret_cast<void **>(&root)};

  brt_gc_push_roots(1, roots);
  brt_gc_collect();
  brt_gc_pop_roots();

  EXPECT_NE(root, original);
  EXPECT_EQ(*root, 42);
}

TEST(GCTest,
     CollectionUpdatesPointerFields)
{
  brt_gc_init();

  auto *child =
      static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
  *child = 99;
  auto *parent = static_cast<Node *>(
      brt_gc_alloc_layout(sizeof(Node), 1, kNodePointerOffsets));
  parent->child = child;
  parent->value = 7;

  auto *originalChild = child;
  void **roots[] = {reinterpret_cast<void **>(&parent)};

  brt_gc_push_roots(1, roots);
  brt_gc_collect();
  brt_gc_pop_roots();

  ASSERT_NE(parent->child, nullptr);
  EXPECT_NE(parent->child, originalChild);
  EXPECT_EQ(*static_cast<std::uint64_t *>(parent->child), 99);
  EXPECT_EQ(parent->value, 7);
}
