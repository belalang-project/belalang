#ifndef BRT_GC_H_
#define BRT_GC_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void brt_gc_init();
void *brt_gc_alloc(size_t size);

#ifdef __cplusplus
}
#endif

#endif // BRT_GC_H_
