#include "gc.h"

#include <gc.h>

void brt_gc_init() { GC_init(); }

void *brt_gc_alloc(size_t size) { return GC_malloc(size); }
