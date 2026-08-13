#ifndef BRT_GC_H_
#define BRT_GC_H_

#include <stddef.h>

void brt_gc_init();
void *brt_gc_alloc(size_t size);

#endif // BRT_GC_H_
