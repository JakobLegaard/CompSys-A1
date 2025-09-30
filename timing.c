#include "timing.h"
#include <sys/time.h>
#include <stddef.h>

long get_usec(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long)tv.tv_sec * 1000000L + (long)tv.tv_usec;
}
