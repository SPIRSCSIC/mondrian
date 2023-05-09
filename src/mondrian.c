#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int DEFAULT_REC = 10;
int DEFAULT_ATT = 3;
int DEFAULT_K = 2;
char *DEFAULT_MAP = "map.json";

struct config {
  int n_records;
  int n_attributes;
  int k;
  char *file_path;
};

int compare(const void *a, const void *b) {
  int f_a = *((int *)a);
  int f_b = *((int *)b);

  return f_a - f_b;
}

void anonymized_partition(struct config cfg, int **data) {
  int *min_vals = malloc(sizeof(int *) * cfg.n_attributes);
  int *max_vals = malloc(sizeof(int *) * cfg.n_attributes);
  for (int i = 0; i < cfg.n_attributes; i++) {
    min_vals[i] = INT_MAX;
    max_vals[i] = INT_MIN;
  }
  for (int i = 0; i < cfg.n_attributes; i++) {
    for (int j = 0; j < cfg.n_records; j++) {
      if (data[j][i] < min_vals[i])
        min_vals[i] = data[j][i];
      if (data[j][i] > max_vals[i])
        max_vals[i] = data[j][i];
    }
  }
  for (int i = 0; i < cfg.n_records; i++) {
    for (int j = 0; j < cfg.n_attributes; j++) {
      if (min_vals[j] == max_vals[j]) {
        printf("%d ", data[i][j]);
      } else {
        printf("[%d, %d] ", min_vals[j], max_vals[j]);
      }
    }
    printf("\n");
  }
}

void find_largest_range(struct config cfg, int **data, int *attribute_idx,
                        int *range) {
  int max_range = INT_MIN;
  *attribute_idx = -1;

  for (int attr = 0; attr < cfg.n_attributes; attr++) {
    int min = INT_MAX;
    int max = INT_MIN;

    for (int i = 0; i < cfg.n_records; i++) {
      if (data[i][attr] < min)
        min = data[i][attr];
      if (data[i][attr] > max)
        max = data[i][attr];
    }

    int current_range = max - min;
    if (current_range > max_range) {
      max_range = current_range;
      *attribute_idx = attr;
    }
  }

  *range = max_range;
}

void mondrian(struct config cfg, int **data) {
  if (cfg.n_records < 2 * cfg.k) {
    anonymized_partition(cfg, data);
    return;
  }

  int attribute_idx;
  int range;
  find_largest_range(cfg, data, &attribute_idx, &range);

  if (attribute_idx == -1) {
    return;
  }

  int allowable_cut = 0;
  int iterator = 0;
  int l_size;
  int r_size = 0;
  int median;
  while (!allowable_cut) {
    int *values = (int *)malloc(cfg.n_records * sizeof(int));
    for (int i = 0; i < cfg.n_records; i++) {
      values[i] = data[i][(attribute_idx + iterator) % cfg.n_attributes];
    }

    qsort(values, cfg.n_records, sizeof(int), compare);
    median = values[cfg.n_records / 2];
    // printf("Cut by: %d, %d\n", (attribute_idx + iterator) % cfg.n_attributes,
    // median);
    free(values);

    r_size = 0;

    for (int i = 0; i < cfg.n_records; i++) {
      if (data[i][(attribute_idx + iterator) % cfg.n_attributes] >= median) {
        r_size += 1;
      }
    }
    l_size = cfg.n_records - r_size;
    if (r_size == cfg.n_records || l_size == cfg.n_records || r_size < cfg.k ||
        l_size < cfg.k) {
      iterator += 1;
      if (iterator == cfg.n_attributes) {
        anonymized_partition(cfg, data);
        return;
      }
      continue;
    }
    allowable_cut = 1;
    attribute_idx = (attribute_idx + iterator) % cfg.n_attributes;
  }
  int **r_part = (int **)malloc(sizeof(int *) * r_size);
  int **l_part = (int **)malloc(sizeof(int *) * l_size);
  int r_count = 0;
  int l_count = 0;
  for (int i = 0; i < cfg.n_records; i++) {
    if (data[i][attribute_idx] >= median) {
      r_part[r_count] = data[i];
      r_count++;
    } else {
      l_part[l_count] = data[i];
      l_count++;
    }
  }

  struct config r_cfg = {r_size, cfg.n_attributes, cfg.k, cfg.file_path};
  struct config l_cfg = {l_size, cfg.n_attributes, cfg.k, cfg.file_path};

  mondrian(r_cfg, r_part);
  mondrian(l_cfg, l_part);
}

struct config parse_args(int argc, char **argv) {
  struct config args = {DEFAULT_REC, DEFAULT_ATT, DEFAULT_K, DEFAULT_MAP};
  int c;
  while ((c = getopt(argc, argv, ":k:r:a:f:")) != -1) {
    switch (c) {
    case 'k': // k_range
      args.k = atoi(optarg);
      break;
    case 'r': // n_records
      args.n_records = atoi(optarg);
      break;
    case 'a': // n_attributes
      args.n_attributes = atoi(optarg);
      break;
    case 'f': // file
      args.file_path = optarg;
      break;
    case '?':
      fprintf(stderr,
              "Usage: %s [OPTION...]\n"
              "  -k <value>\t\tSet the K-anonimity\n"
              "  -r <value>\t\tSet the number of records\n"
              "  -a <value>\t\tSet number of attributes\n"
              "  -f <value>\t\tSet the file path to anonimyze\n",
              *argv);
      exit(1);
    case ':':
      fprintf(stderr, "Error: option -%c requires an argument\n", optopt);
      exit(1);
    }
  }
  return args;
}

int main(int argc, char **argv) {
  struct config cfg = parse_args(argc, argv);

  int **data = (int **)malloc(cfg.n_records * sizeof(int *));
  for (int i = 0; i < cfg.n_records; i++) {
    data[i] = (int *)malloc(cfg.n_attributes * sizeof(int));
  }

  // Example dataset
  data[0][0] = 34;
  data[0][1] = 12;
  data[0][2] = 1;
  data[1][0] = 20;
  data[1][1] = 9;
  data[1][2] = 5;
  data[2][0] = 25;
  data[2][1] = 7;
  data[2][2] = 3;
  data[3][0] = 50;
  data[3][1] = 15;
  data[3][2] = 6;
  data[4][0] = 55;
  data[4][1] = 20;
  data[4][2] = 7;
  data[5][0] = 40;
  data[5][1] = 12;
  data[5][2] = 2;
  data[6][0] = 30;
  data[6][1] = 10;
  data[6][2] = 4;
  data[7][0] = 45;
  data[7][1] = 18;
  data[7][2] = 6;
  data[8][0] = 60;
  data[8][1] = 22;
  data[8][2] = 8;
  data[9][0] = 15;
  data[9][1] = 5;
  data[9][2] = 3;

  // cfg.n_attributes, cfg.k, cfg.n_records
  mondrian(cfg, data);

  // Free the memory allocated for the data array
  for (int i = 0; i < cfg.n_records; i++) {
    free(data[i]);
  }
  free(data);

  return 0;
}
