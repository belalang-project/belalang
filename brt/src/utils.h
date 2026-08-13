#ifndef BRT_UTILS_H_
#define BRT_UTILS_H_

#include <stdint.h>

static inline uintptr_t align(uintptr_t val, uintptr_t alignment) {
  return (val + alignment - 1) & ~(alignment - 1);
}

static inline uintptr_t align_size(uintptr_t size) {
  return align(size, sizeof(uintptr_t));
}

#endif // BRT_UTILS_H_
