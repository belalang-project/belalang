#include "llvm_stackmap.h"

extern const uint8_t __start_llvm_stackmaps[];
extern const uint8_t __stop_llvm_stackmaps[];

struct LLVMStkMapSection get_llvm_stkmap_section() {
  uintptr_t start = (uintptr_t)__start_llvm_stackmaps;
  uintptr_t stop = (uintptr_t)__stop_llvm_stackmaps;

  if (stop < start)
    return (struct LLVMStkMapSection){.start = NULL, .size = 0};

  struct LLVMStkMapSection s = {
      .start = (const uint8_t *)start,
      .size = (size_t)(stop - start),
  };
  return s;
}
