#ifndef BRT_GC_H_
#define BRT_GC_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void
brt_gc_init();
void *
brt_gc_alloc(size_t size);
void *
brt_gc_alloc_layout(size_t size,
                    size_t pointer_count,
                    const uint32_t *pointer_offsets);
void
brt_gc_push_roots(size_t count,
                  void ***roots);
void
brt_gc_pop_roots();
void
brt_gc_collect();

#ifdef __cplusplus
}
#endif

#endif // BRT_GC_H_
