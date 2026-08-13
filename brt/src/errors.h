#ifndef BRT_ERRORS_H_
#define BRT_ERRORS_H_

#include <stdlib.h>
#include <stdio.h>

_Noreturn void brt_fatal(const char *msg) {
  fprintf(stderr, "brt: %s\n", msg);
  abort();
}

#endif // BRT_ERRORS_H_
