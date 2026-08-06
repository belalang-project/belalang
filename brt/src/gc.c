#define _DEFAULT_SOURCE

#include "gc.h"
#include "errors.h"
#include "utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

struct gc_obj {
  size_t size;
  bool marked;
  struct gc_obj *next;
  unsigned char data[];
};

struct gc_heap {
  uintptr_t hp;
  uintptr_t limit;
  uintptr_t space;
  struct gc_obj *objects;
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
  uintptr_t payload_size = align_size(size);
  uintptr_t alloc_size = sizeof(struct gc_obj) + payload_size;

  if (alloc_size > the_heap.limit - the_heap.hp)
    brt_fatal("out of GC heap memory");

  struct gc_obj *obj = (struct gc_obj *)the_heap.hp;
  obj->size = payload_size;
  obj->marked = false;
  obj->next = the_heap.objects;
  the_heap.objects = obj;
  the_heap.hp += alloc_size;

  memset(obj->data, 0, alloc_size);
  return obj->data;
}
