#include "llvm_stackmap.h"
#include "gc.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

struct BrString {
  const uint8_t *ptr;
  uint64_t len;
};

void brt_print_int(int64_t v) { printf("%ld\n", v); }
void brt_print_float(double v) { printf("%f\n", v); }
void brt_print_bool(bool v) { printf("%s\n", v ? "true" : "false"); }

void brt_print_string(struct BrString v) {
  if (v.ptr && v.len > 0) {
    write(STDOUT_FILENO, v.ptr, v.len);
    write(STDOUT_FILENO, "\n", 1);
  }
}

void brt_init() {
  (void)get_llvm_stkmap_section();
  brt_gc_init();
}
