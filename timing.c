#include "timing.h"
#include <sys/time.h>
#include <stddef.h> 
#include <stdint.h>

uint64_t microseconds(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}
