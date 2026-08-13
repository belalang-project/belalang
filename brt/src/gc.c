#include "gc.h"
#include "errors.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

struct gc_heap {
  uintptr_t hp;
  uintptr_t limit;
  uintptr_t space;
};

/// Global heap instance.
static struct gc_heap the_heap;

/// Initialize GC heap.
static void gc_heap_init(size_t size) {
  void *mem =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (mem == MAP_FAILED)
    brt_fatal("mmap failed");

  the_heap.space = the_heap.hp = (uintptr_t)mem;
  the_heap.limit = the_heap.hp + size;
}

void brt_gc_init() { gc_heap_init(1024 * 1024); }

void *brt_gc_alloc(size_t size) {
  uintptr_t alloc_size = align_size(size);

  if (alloc_size > the_heap.limit - the_heap.hp)
    brt_fatal("out of GC heap memory");

  void *result = (void *)the_heap.hp;
  the_heap.hp += alloc_size;
  memset(result, 0, alloc_size);
  return result;
}
