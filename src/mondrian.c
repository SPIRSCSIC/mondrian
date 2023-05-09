#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int MAX_ROW = 1024;
int DEFAULT_REC = 10;
int DEFAULT_ATT = 3;
int DEFAULT_K = 2;
char *DEFAULT_DATA = "../datasets/dataset.csv";
char *DEFAULT_MAP = "map.json";

struct config {
  int n_records;
  int n_attributes;
  int k;
  char *dataset_path;
  char *map_path;
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
  for (int idx_attr = 0; idx_attr < cfg.n_attributes; idx_attr++) {
    for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
      if (data[idx_rec][idx_attr] < min_vals[idx_attr])
        min_vals[idx_attr] = data[idx_rec][idx_attr];
      if (data[idx_rec][idx_attr] > max_vals[idx_attr])
        max_vals[idx_attr] = data[idx_rec][idx_attr];
    }
  }
  for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
    for (int idx_attr = 0; idx_attr < cfg.n_attributes; idx_attr++) {
      if (min_vals[idx_attr] == max_vals[idx_attr]) {
        printf("%d ", data[idx_rec][idx_attr]);
      } else {
        printf("[%d, %d] ", min_vals[idx_attr], max_vals[idx_attr]);
      }
    }
    printf("\n");
  }
}

void find_largest_range(struct config cfg, int **data, int *index_attr,
                        int *range) {
  int max_range = INT_MIN;
  *index_attr = -1;

  for (int idx_attr = 0; idx_attr < cfg.n_attributes; idx_attr++) {
    int min = INT_MAX;
    int max = INT_MIN;

    for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
      if (data[idx_rec][idx_attr] < min)
        min = data[idx_rec][idx_attr];
      if (data[idx_rec][idx_attr] > max)
        max = data[idx_rec][idx_attr];
    }

    int current_range = max - min;
    if (current_range > max_range) {
      max_range = current_range;
      *index_attr = idx_attr;
    }
  }

  *range = max_range;
}

void mondrian(struct config cfg, int **data) {
  if (cfg.n_records < 2 * cfg.k) {
    anonymized_partition(cfg, data);
    return;
  }

  int index_attr;
  int range;
  find_largest_range(cfg, data, &index_attr, &range);

  if (index_attr == -1) {
    return;
  }

  int allowable_cut = 0;
  int iterator = 0;
  int l_size;
  int r_size = 0;
  int median;
  while (!allowable_cut) {
    int *values = (int *)malloc(cfg.n_records * sizeof(int));
    for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
      values[idx_rec] =
          data[idx_rec][(index_attr + iterator) % cfg.n_attributes];
    }

    qsort(values, cfg.n_records, sizeof(int), compare);
    median = values[cfg.n_records / 2];
    // printf("Cut by: %d, %d\n", (attribute_idx + iterator) % cfg.n_attributes,
    // median);
    free(values);

    r_size = 0;

    for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
      if (data[idx_rec][(index_attr + iterator) % cfg.n_attributes] >= median) {
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
    index_attr = (index_attr + iterator) % cfg.n_attributes;
  }
  int **r_part = (int **)malloc(sizeof(int *) * r_size);
  int **l_part = (int **)malloc(sizeof(int *) * l_size);
  int r_count = 0;
  int l_count = 0;
  for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
    if (data[idx_rec][index_attr] >= median) {
      r_part[r_count] = data[idx_rec];
      r_count++;
    } else {
      l_part[l_count] = data[idx_rec];
      l_count++;
    }
  }

  struct config r_cfg = {r_size, cfg.n_attributes, cfg.k, cfg.dataset_path,
                         cfg.map_path};
  struct config l_cfg = {l_size, cfg.n_attributes, cfg.k, cfg.dataset_path,
                         cfg.map_path};

  mondrian(r_cfg, r_part);
  mondrian(l_cfg, l_part);
}

struct config parse_args(int argc, char **argv) {
  struct config args = {DEFAULT_REC, DEFAULT_ATT, DEFAULT_K, DEFAULT_DATA,
                        DEFAULT_MAP};
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
    case 'd': // dataset path
      args.map_path = optarg;
      break;
    case 'm': // map path
      args.map_path = optarg;
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

void fill_dataset(struct config cfg, int **data) {
  FILE *fp;
  char row[MAX_ROW];
  char *token;
  int idx_row = 0;
  int idx_token = 0;

  fp = fopen(cfg.dataset_path, "r");

  while (!feof(fp)) {
    idx_token = 0;
    fgets(row, MAX_ROW, fp);
    /* printf("Row: %s", row); */

    token = strtok(row, ",");
    while (token != NULL) {
      if (*token == 10) { // newline
        fprintf(stderr,
                "Error: Invalid dataset. "
                "Unfinished line %d. "
                "Unexpected termination token.",
                idx_row);
        exit(1);
      }
      data[idx_row][idx_token] = atoi(token);
      /* printf("Token: %s\n", token); */
      token = strtok(NULL, ",");
      idx_token++;
    }
    if (idx_token != cfg.n_attributes) {
      fprintf(stderr, "Error: Invalid dataset. "
                      "Number of tokens per row do not "
                      "coincide with n_attributes.");
      exit(1);
    }
    idx_row++;
  }
  if (idx_row != cfg.n_records) {
    fprintf(stderr, "Error: Invalid dataset. "
                    "Number of rows do not "
                    "coincide with n_records.");
    exit(1);
  }
}

int main(int argc, char **argv) {
  struct config cfg = parse_args(argc, argv);

  int **data = (int **)malloc(cfg.n_records * sizeof(int *));
  for (int idx_rec = 0; idx_rec < cfg.n_records; idx_rec++) {
    data[idx_rec] = (int *)malloc(cfg.n_attributes * sizeof(int));
  }

  fill_dataset(cfg, data);

  // cfg.n_attributes, cfg.k, cfg.n_records
  mondrian(cfg, data);

  // Free the memory allocated for the data array
  for (int i = 0; i < cfg.n_records; i++) {
    free(data[i]);
  }
  free(data);

  return 0;
}
