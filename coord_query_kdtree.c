#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>  
#include "record.h"
#include "coord_query.h"

typedef struct kdnode {
  const struct record *rec;
  int axis;
  struct kdnode *left, *right;
} kdnode;

struct kdtree {
  kdnode *root;
};

static inline double dist2(double qlon, double qlat, const struct record *r) {
  double dx = qlon - r->lon;
  double dy = qlat - r->lat;
  return dx*dx + dy*dy;
}

static int cmp_lon(const void *a, const void *b) {
  const struct record *ra = *(const struct record* const*)a;
  const struct record *rb = *(const struct record* const*)b;
  return (ra->lon < rb->lon) ? -1 : (ra->lon > rb->lon);
}

static int cmp_lat(const void *a, const void *b) {
  const struct record *ra = *(const struct record* const*)a;
  const struct record *rb = *(const struct record* const*)b;
  return (ra->lat < rb->lat) ? -1 : (ra->lat > rb->lat);
}

static kdnode* build_rec(const struct record **pts, int n, int depth) {
  if (n <= 0) return NULL;
  int axis = depth & 1;
  qsort(pts, n, sizeof(*pts), axis == 0 ? cmp_lon : cmp_lat);
  int mid = n / 2;
  kdnode *node = (kdnode*)malloc(sizeof(kdnode));
  if (!node) { perror("malloc"); exit(1); }
  node->rec = pts[mid];
  node->axis = axis;
  node->left  = build_rec(pts,         mid,        depth+1);
  node->right = build_rec(pts + mid+1, n - mid -1, depth+1);
  return node;
}

static void free_rec(kdnode *node) {
  if (!node) return;
  free_rec(node->left);
  free_rec(node->right);
  free(node);
}

static struct kdtree* mk_kdtree(const struct record *rs_in, int n) {
  struct kdtree *t = (struct kdtree*)malloc(sizeof *t);
  if (!t) { perror("malloc"); exit(1); }
  const struct record **pts = (const struct record**)malloc(sizeof(*pts)*n);
  if (!pts) { perror("malloc"); exit(1); }
  for (int i = 0; i < n; i++) pts[i] = &rs_in[i];
  t->root = build_rec(pts, n, 0);
  free(pts);
  return t;
}

static void free_kdtree(struct kdtree *t) {
  if (!t) return;
  free_rec(t->root);
  free(t);
}

static void nn_rec(const kdnode *node, double qlon, double qlat,
                   const struct record **best, double *best_d2) {
  if (!node) return;

  double d2 = (qlon - node->rec->lon) * (qlon - node->rec->lon)
            + (qlat - node->rec->lat) * (qlat - node->rec->lat);
  if (d2 < *best_d2) { *best_d2 = d2; *best = node->rec; }

  double diff = (node->axis == 0) ? (node->rec->lon - qlon) : (node->rec->lat - qlat);
  double diff2 = diff * diff;

  if (diff >= 0 || *best_d2 > diff2)
    nn_rec(node->left, qlon, qlat, best, best_d2);

  if (diff <= 0 || *best_d2 > diff2)
    nn_rec(node->right, qlon, qlat, best, best_d2);
}


static const struct record* lookup_kdtree(struct kdtree *t, double lon, double lat) {
  const struct record *best = NULL;
  double best_d2 = HUGE_VAL;
  nn_rec(t->root, lon, lat, &best, &best_d2);
  return best;
}

int main(int argc, char **argv) {
  return coord_query_loop(
    argc, argv,
    (mk_index_fn)  mk_kdtree,
    (free_index_fn)free_kdtree,
    (lookup_fn)    lookup_kdtree
  );
}
