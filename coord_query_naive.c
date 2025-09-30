#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

static inline double dist2(double lon, double lat, const struct record *r) {
  double dx = lon - r->lon;
  double dy = lat - r->lat;
  return dx*dx + dy*dy; 
}

struct naive_data* mk_naive(const struct record *rs_in, int n) {
  struct naive_data *data = malloc(sizeof *data);
  if (!data) {
    fprintf(stderr, "Out of memory in mk_naive\n");
    exit(1);
  }

  data->rs = (struct record*)rs_in;
  data->n  = n;
  return data;
}

void free_naive(struct naive_data *data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  const struct record *best = NULL;
  double best_d2 = HUGE_VAL;

  for (int i = 0; i < data->n; i++) {
    const struct record *r = &data->rs[i];
    double d2 = dist2(lon, lat, r);
    if (d2 < best_d2) {
      best_d2 = d2;
      best = r;
    }
  }
  return best; 
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
