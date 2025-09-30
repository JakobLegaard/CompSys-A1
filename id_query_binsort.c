#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct binsort {
    struct record *rs;
    int n;
};

int compare(const void *a, const void *b) {
    const struct record *ra = (const struct record *)a;
    const struct record *rb = (const struct record *)b;
    if (ra->osm_id < rb->osm_id) {
        return -1;
    } else if (ra->osm_id > rb->osm_id) {
        return 1;
     } else {
        return 0;
    }
}

struct binsort* mk_binsort(struct record* rs, int n) {
    struct binsort *data = malloc(sizeof(*data));
    if (data == NULL) {
        return NULL;
    }

    data->rs = malloc(n * sizeof(struct record));
    if (data->rs == NULL) {
        free(data);
        return NULL;
}
for (int i = 0; i < n; i++) {
        data->rs[i] = rs[i];
    }
    data->n = n;

    qsort(data->rs, n, sizeof(struct record), compare);

    return data;
}

void free_binsort(struct binsort* data) {
    if (data){
        free(data->rs);
        free(data);
    }
}

const struct record* lookup_binsort(void *data, int64_t needle) {
    struct binsort *binsort = (struct binsort *)data;
    struct record *rs = binsort->rs;
    int left = 0;
    int right = binsort->n - 1;

    while (left <= right) {
        int mid = (left + right) / 2;
        int64_t mid_id = rs[mid].osm_id;

        if (mid_id == needle) {
            return &rs[mid];
        } else if (mid_id < needle) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}