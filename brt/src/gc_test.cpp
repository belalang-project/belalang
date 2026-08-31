#include "gc.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

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

TEST(GCTest,
     ConfiguredHeapTriggersRepeatedAutomaticCollections)
{
  brt_gc_init_with_size(512);

  auto *root =
      static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
  *root = 42;
  void **roots[] = {reinterpret_cast<void **>(&root)};

  brt_gc_push_roots(1, roots);
  for (std::uint64_t i = 0; i < 64; ++i) {
    // Immediately becomes garbage.
    auto *garbage =
        static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
    *garbage = i;
    EXPECT_EQ(*root, 42);
  }
  brt_gc_pop_roots();

  auto stats = brt_gc_get_stats();
  EXPECT_EQ(stats.allocations, 65); // 1 root + 64 garbage
  EXPECT_GE(stats.collections, 2);
  EXPECT_GE(stats.relocated_objects, 2);
  EXPECT_GE(stats.relocated_bytes, 2 * sizeof(std::uint64_t));
}

TEST(GCTest,
     HeapSizeCanBeConfiguredFromEnvironment)
{
  const char *sizes[] = {"1GiB", "1MiB", "1KiB", "512", "512B"};
  for (const char *size : sizes) {
    ASSERT_EQ(setenv("BELALANG_GC_HEAP_SIZE", size, 1), 0);
    brt_gc_init();
  }
  ASSERT_EQ(unsetenv("BELALANG_GC_HEAP_SIZE"), 0);

  auto *root =
      static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
  *root = 42;
  void **roots[] = {reinterpret_cast<void **>(&root)};

  brt_gc_push_roots(1, roots);
  for (std::uint64_t i = 0; i < 64; ++i) {
    auto *garbage =
        static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
    *garbage = i;
  }
  brt_gc_pop_roots();

  auto stats = brt_gc_get_stats();
  EXPECT_EQ(stats.allocations, 65);
  EXPECT_GE(stats.collections, 2);
  EXPECT_GE(stats.relocated_objects, 2);
  EXPECT_EQ(*root, 42);
}

TEST(GCTest,
     HeapSizeRejectsUnsupportedSuffixes)
{
  EXPECT_DEATH(
      {
        setenv("BELALANG_GC_HEAP_SIZE", "1TiB", 1);
        brt_gc_init();
      },
      "invalid BELALANG_GC_HEAP_SIZE suffix");
  EXPECT_DEATH(
      {
        setenv("BELALANG_GC_HEAP_SIZE", "1MB", 1);
        brt_gc_init();
      },
      "invalid BELALANG_GC_HEAP_SIZE suffix");
}

TEST(GCTest,
     AutomaticCollectionPreservesCycles)
{
  brt_gc_init_with_size(1024);

  auto *first = static_cast<Node *>(
      brt_gc_alloc_layout(sizeof(Node), 1, kNodePointerOffsets));
  auto *second = static_cast<Node *>(
      brt_gc_alloc_layout(sizeof(Node), 1, kNodePointerOffsets));

  first->child = second;
  first->value = 1;
  second->child = first;
  second->value = 2;

  void **roots[] = {reinterpret_cast<void **>(&first)};

  brt_gc_push_roots(1, roots);
  for (std::uint64_t i = 0; i < 64; ++i) {
    auto *garbage =
        static_cast<std::uint64_t *>(brt_gc_alloc(sizeof(std::uint64_t)));
    *garbage = i;

    auto *relocated_second = static_cast<Node *>(first->child);
    EXPECT_EQ(first->value, 1);
    EXPECT_EQ(relocated_second->value, 2);
    EXPECT_EQ(relocated_second->child, first);
  }
  brt_gc_pop_roots();

  auto stats = brt_gc_get_stats();
  EXPECT_EQ(stats.allocations, 66);
  EXPECT_GT(stats.collections, 0);
  EXPECT_GE(stats.relocated_objects, 2);
  EXPECT_GE(stats.relocated_bytes, 2 * sizeof(Node));
}
