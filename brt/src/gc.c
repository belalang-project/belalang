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
  /// Aligned payload size in bytes.
  size_t size;
  /// Number of pointer fields in the payload.
  size_t pointer_count;
  /// Array of byte offsets identifying where pointer fields occur.
  const uint32_t *pointer_offsets;
  /// Used during copying collection.
  struct gc_obj *forwarding;
  /// Flexible array member representing the object payload.
  unsigned char data[];
};

struct gc_root_frame {
  /// Number of root slots in this frame.
  size_t count;
  /// Array of addresses of pointer variables.
  void ***roots;
  /// Links to the previous root frame.
  struct gc_root_frame *prev;
};

struct gc_semispace {
  /// Points to the start of the semispace.
  uintptr_t start;
  /// Heap pointer.
  uintptr_t hp;
  /// Points to the end of the semispace. (Exclusive)
  uintptr_t limit;
};

struct gc_heap {
  /// from-space.
  struct gc_semispace fromsp;
  /// to-space.
  struct gc_semispace tosp;
  /// Top of the registered root-frame stack.
  struct gc_root_frame *roots;
};

/// Converts a payload pointer back to its object header.
static struct gc_obj *
gc_obj_from_data(void *ptr)
{
  return (struct gc_obj *)((uintptr_t)ptr - offsetof(struct gc_obj, data));
}

/// Creates a new semispace with the specified size;
static struct gc_semispace
gc_semispace_create(size_t size)
{
  void *mem =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (mem == MAP_FAILED)
    brt_fatal("mmap failed");

  struct gc_semispace sp;
  sp.start = sp.hp = (uintptr_t)mem;
  sp.limit = sp.start + size;
  return sp;
}

/// Checks if the semispace sp contains the pointer ptr.
static bool
gc_semispace_contains(const struct gc_semispace *sp,
                      const void *ptr)
{
  uintptr_t addr = (uintptr_t)ptr;
  return addr >= sp->start && addr < sp->limit;
}

/// Reserves aligned raw storage in a semispace.
static void *
gc_semispace_alloc_in(struct gc_semispace *sp,
                      size_t size)
{
  size_t aligned_size = align_size(size);
  if (aligned_size > sp->limit - sp->hp)
    return NULL;

  void *memory = (void *)sp->hp;
  sp->hp += aligned_size;
  return memory;
}

/// Initialize GC heap.
static void
gc_heap_init(struct gc_heap *heap,
             size_t size)
{
  size_t semispace_size = align_size(size / 2);
  if (semispace_size <= sizeof(struct gc_obj))
    brt_fatal("GC heap too small");

  heap->fromsp = gc_semispace_create(semispace_size);
  heap->tosp = gc_semispace_create(semispace_size);
  heap->roots = NULL;
}

/// Performs bump allocation in the active semispace and initializes the object
/// header and zeroed payload.
static void *
gc_heap_alloc_obj(struct gc_heap *heap,
                  size_t size,
                  size_t pointer_count,
                  const uint32_t *pointer_offsets)
{
  size_t payload_size = align_size(size);
  size_t object_size = sizeof(struct gc_obj) + payload_size;

  struct gc_obj *obj = gc_semispace_alloc_in(&heap->fromsp, object_size);
  if (!obj)
    return NULL;

  obj->size = payload_size;
  obj->pointer_count = pointer_count;
  obj->pointer_offsets = pointer_offsets;
  obj->forwarding = NULL;

  memset(obj->data, 0, payload_size);
  return obj->data;
}

/// Copies a live object to to-space. It uses the forwarding pointer to ensure
/// each object is copied only once.
static void *
gc_heap_copy_obj(struct gc_heap *heap,
                 void *ptr)
{
  if (ptr == NULL || !gc_semispace_contains(&heap->fromsp, ptr))
    return ptr;

  struct gc_obj *old = gc_obj_from_data(ptr);
  if (old->forwarding)
    return old->forwarding->data;

  struct gc_obj *new_obj =
      gc_semispace_alloc_in(&heap->tosp, sizeof(struct gc_obj) + old->size);
  if (!new_obj)
    brt_fatal("out of GC heap memory");

  memcpy(new_obj, old, sizeof(struct gc_obj) + old->size);
  new_obj->forwarding = NULL;
  old->forwarding = new_obj;
  return new_obj->data;
}

/// Replaces a root or object field with its relocated pointer.
static void
gc_heap_update_slot(struct gc_heap *heap,
                    void **slot)
{
  if (slot)
    *slot = gc_heap_copy_obj(heap, *slot);
}

/// Traverses copied objects and updates fields listed in their pointer-offset
/// metadata.
static void
gc_heap_scan_copied_objects(struct gc_heap *heap)
{
  uintptr_t it = heap->tosp.start;
  while (it < heap->tosp.hp) {
    struct gc_obj *obj = (struct gc_obj *)it;
    for (size_t i = 0; i < obj->pointer_count; ++i) {
      uint32_t offset = obj->pointer_offsets[i];
      if (offset + sizeof(void *) <= obj->size)
        gc_heap_update_slot(heap, (void **)(obj->data + offset));
    }
    it += align_size(sizeof(struct gc_obj) + obj->size);
  }
}

/// Global heap instance.
static struct gc_heap the_heap;

void
brt_gc_init()
{
  gc_heap_init(&the_heap, 1024 * 1024);
}

void
brt_gc_push_roots(size_t count,
                  void ***roots)
{
  struct gc_root_frame *frame = malloc(sizeof(struct gc_root_frame));
  if (!frame)
    brt_fatal("malloc failed");

  frame->count = count;
  frame->roots = roots;
  frame->prev = the_heap.roots;
  the_heap.roots = frame;
}

void
brt_gc_pop_roots()
{
  if (!the_heap.roots)
    brt_fatal("GC root stack underflow");

  struct gc_root_frame *frame = the_heap.roots;
  the_heap.roots = frame->prev;
  free(frame);
}

void
brt_gc_collect()
{
  the_heap.tosp.hp = the_heap.tosp.start;

  for (struct gc_root_frame *frame = the_heap.roots; frame;
       frame = frame->prev) {
    for (size_t i = 0; i < frame->count; ++i)
      gc_heap_update_slot(&the_heap, frame->roots[i]);
  }

  gc_heap_scan_copied_objects(&the_heap);

  struct gc_semispace old_fromsp = the_heap.fromsp;
  the_heap.fromsp = the_heap.tosp;
  the_heap.tosp = old_fromsp;

  the_heap.tosp.hp = the_heap.tosp.start;
}

void *
brt_gc_alloc_layout(size_t size,
                    size_t pointer_count,
                    const uint32_t *pointer_offsets)
{
  void *ptr =
      gc_heap_alloc_obj(&the_heap, size, pointer_count, pointer_offsets);

  // Retry if fail.
  if (!ptr) {
    brt_gc_collect();
    ptr = gc_heap_alloc_obj(&the_heap, size, pointer_count, pointer_offsets);
  }

  // We really failed.
  if (!ptr)
    brt_fatal("out of GC heap memory");

  return ptr;
}

void *
brt_gc_alloc(size_t size)
{
  return brt_gc_alloc_layout(size, 0, NULL);
}
