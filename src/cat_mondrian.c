#include "utils.h"
#include <signal.h>
#include <time.h>

config cfg = {0, NULL, 0, NULL, 0, NULL, NULL};
subjects subjs = {0, NULL};
quasi qi = {NULL, NULL, NULL, NULL, NULL};
partitions parts = {0, NULL};

int choose_dimension(partition *part) {
  double max_width = -1.0;
  int max_dim = -1;
  for (int i = 0; i < cfg.n_qid; i++) {
    if (!part->allow[i])
      continue;
    double norm_width = normalized_width(part, i);
    if (norm_width > max_width) {
      max_width = norm_width;
      max_dim = i;
    }
  }
  return max_dim;
}

void mondrian_init() {
  qi.range = (int *)malloc(cfg.n_qid * sizeof(int));
  qi.n_order = (int *)malloc(cfg.n_qid * sizeof(int));
  qi.order = (int **)malloc(cfg.n_qid * sizeof(int *));
  qi.original = (int **)malloc(subjs.n_subj * sizeof(int *));
  qi.dict = (dictionary *)malloc(cfg.n_qid * sizeof(dictionary));
  for (int i = 0; i < cfg.n_qid; i++) {
    qi.dict[i] = (dictionary){0, NULL};
    qi.n_order[i] = 0;
    qi.order[i] = NULL;
  }
  for (int i = 0; i < subjs.n_subj; i++)
    qi.original[i] = NULL;
  for (int i = 0; i < subjs.n_subj; i++) {
    qi.original[i] = (int *)realloc(qi.original[i], cfg.n_qid * sizeof(int));
    for (int j = 0; j < cfg.n_qid; j++) {
      if (int_in_list(qi.order[j], qi.n_order[j], subjs.attr_values[i][j]) ==
          -1) {
        qi.n_order[j]++;
        qi.order[j] = (int *)realloc(qi.order[j], qi.n_order[j] * sizeof(int));
        qi.order[j][qi.n_order[j] - 1] = subjs.attr_values[i][j];
      }
      qi.original[i][j] = subjs.attr_values[i][j];
    }
  }

  /* printf("QI_ORIG: ["); */
  /* for (int i = 0; i < subjs.n_subj; i++) { */
  /*   if (i) */
  /*     printf(", "); */
  /*   printf("["); */
  /*   for (int j = 0; j < cfg.n_qid; j++) */
  /*     printf("%d, ", qi.original[i][j]); */
  /*   printf("%d", qi.original[i][cfg.n_qid - 1]); */
  /*   printf("]"); */
  /* } */
  /* printf("]\n"); */

  for (int i = 0; i < cfg.n_qid; i++) {
    qsort(qi.order[i], qi.n_order[i], sizeof(int), compare);
    /* printf("QI_ORDER: ["); */
    /* for (int j = 0; j < qi.n_order[i] - 1; j++) */
    /*   printf("%d, ", qi.order[i][j]); */
    /* printf("%d", qi.order[i][qi.n_order[i] - 1]); */
    /* printf("]\n"); */
    qi.range[i] = qi.order[i][qi.n_order[i] - 1] - qi.order[i][0];
    /* printf("QI_RANGE: %d\n", qi.range[i]); */
    qi.dict[i].n_tuple = qi.n_order[i];
    qi.dict[i].tuple = (tuple *)malloc(qi.n_order[i] * sizeof(tuple));
    for (int j = 0; j < qi.n_order[i]; j++) {
      qi.dict[i].tuple[j].key = qi.order[i][j];
      qi.dict[i].tuple[j].value = j;
    }
    /* printf("QI_DICT: {"); */
    /* for (int j = 0; j < qi.n_order[i] - 1; j++) */
    /*   printf("%d: %d, ", qi.dict[i].tuple[j].key, qi.dict[i].tuple[j].value);
     */
    /* printf("%d: %d", qi.dict[i].tuple[qi.n_order[i] - 1].key, */
    /*        qi.dict[i].tuple[qi.n_order[i] - 1].value); */
    /* printf("}\n"); */
  }
}

void partition_init(partition *part, int **data, int n_data, int *low,
                    int *high) {
  part->low = (int *)malloc(cfg.n_qid * sizeof(int));
  memcpy(part->low, low, cfg.n_qid * sizeof(int));
  part->high = (int *)malloc(cfg.n_qid * sizeof(int));
  memcpy(part->high, high, cfg.n_qid * sizeof(int));
  part->n_allow = cfg.n_qid; // array of 8 elements, each value is 1
  part->allow = (int *)malloc(cfg.n_qid * sizeof(int));
  for (int i = 0; i < cfg.n_qid; i++)
    part->allow[i] = 1;
  part->n_member = n_data;
  part->member = (int **)malloc(n_data * sizeof(int *));
  for (int i = 0; i < n_data; i++) {
    part->member[i] = (int *)malloc(cfg.n_qid * sizeof(int));
    memcpy(part->member[i], data[i], cfg.n_qid * sizeof(int));
  }
}

frequency *find_median(partition *part, int dim) {
  dictionary dict = {0, NULL};
  frequency *freq = (frequency *)malloc(sizeof(frequency));
  *freq = (frequency){-1, -1, -1, -1};
  for (int i = 0; i < part->n_member; i++) {
    dict_value_inc(&dict, part->member[i][dim]);
  }
  int *keys = dict_keys(&dict);
  int total = dict_sum(&dict);
  int middle = total / 2;
  int index = -1;
  if (dict.n_tuple > 0) {
    freq->low = keys[0];
    freq->high = keys[dict.n_tuple - 1];
  }
  if (middle >= GL_K && dict.n_tuple > 1) {
    int sum = 0;
    for (int i = 0; i < dict.n_tuple && index == -1; i++) {
      sum += dict_value(&dict, keys[i]);
      if (sum >= middle) {
        freq->split = keys[i];
        index = i;
      }
    }
    if (index == -1) {
      fprintf(stderr, "Error: Could not find freq.split\n");
      exit(1);
    }
    if (index + 1 >= dict.n_tuple)
      freq->next = freq->split;
    else
      freq->next = keys[index + 1];
  }
  /* free(dict.tuple); */
  return freq;
}

void anonymize_strict(partition *part) {
  if (!part->n_allow) {
    add_partition(part);
    return;
  }
  for (int i = 0; i < part->n_allow; i++) {
    int dim = choose_dimension(part);
    if (dim == -1) {
      fprintf(stderr, "Error: dim = -1");
      exit(1);
    }
    frequency *freq = find_median(part, dim);
    if (freq->low != -1) {
      part->low[dim] = dict_value(&qi.dict[dim], freq->low);
      part->high[dim] = dict_value(&qi.dict[dim], freq->high);
    }
    if (freq->split == -1 || freq->split == freq->next) {
      part->allow[dim] = 0;
      /* free(freq); */
      continue;
    }
    int mean = dict_value(&qi.dict[dim], freq->split);
    int *lhs_high = (int *)malloc(cfg.n_qid * sizeof(int));
    int *rhs_low = (int *)malloc(cfg.n_qid * sizeof(int));
    for (int i = 0; i < cfg.n_qid; i++) {
      lhs_high[i] = part->high[i];
      rhs_low[i] = part->low[i];
    }
    lhs_high[dim] = mean;
    rhs_low[dim] = dict_value(&qi.dict[dim], freq->next);
    partition lhs = {NULL, NULL, 0, NULL, 0, NULL};
    partition_init(&lhs, NULL, 0, part->low, lhs_high);
    partition rhs = {NULL, NULL, 0, NULL, 0, NULL};
    partition_init(&rhs, NULL, 0, rhs_low, part->high);
    for (int j = 0; j < part->n_member; j++) {
      int pos = dict_value(&qi.dict[dim], part->member[j][dim]);
      if (pos <= mean)
        add_to_partition(&lhs, part->member[j]);
      else
        add_to_partition(&rhs, part->member[j]);
    }
    if (lhs.n_member < GL_K || rhs.n_member < GL_K) {
      part->allow[dim] = 0;
      free(freq);
      free(lhs_high);
      free(rhs_low);
      continue;
    }
    anonymize_strict(&lhs);
    anonymize_strict(&rhs);
    free(freq);
    free(lhs_high);
    free(rhs_low);
    return;
  }
  add_partition(part);
}

void mondrian() {
  mondrian_init();
  int *low = (int *)malloc(cfg.n_qid * sizeof(int));
  int *high = (int *)malloc(cfg.n_qid * sizeof(int));
  for (int i = 0; i < cfg.n_qid; i++) {
    low[i] = 0;
    high[i] = qi.dict[i].n_tuple - 1;
  }
  partition part = {NULL, NULL, 0, NULL, 0, NULL};
  int **data = (int **)malloc(subjs.n_subj * sizeof(int *));
  for (int i = 0; i < subjs.n_subj; i++) {
    data[i] = (int *)malloc(cfg.n_qid * sizeof(int));
    memcpy(data[i], qi.original[i], cfg.n_qid * sizeof(int));
  }
  partition_init(&part, data, subjs.n_subj, low, high);

  /* set up clock to calculate execution time */
  clock_t start;
  double cpu_time_used;
  start = clock();
  anonymize_strict(&part);
  cpu_time_used = ((double)(clock() - start)) / CLOCKS_PER_SEC;
  double ncp = 0.0;
  double dp = 0.0;
  int n_res = 0;
  char ***res = NULL;
  for (int i = 0; i < parts.n_part; i++) {
    n_res = n_res + parts.part[i].n_member;
    res = (char ***)realloc(res, n_res * sizeof(char **));
    double rncp = 0.0;
    for (int j = 0; j < cfg.n_qid; j++)
      rncp += normalized_width(&parts.part[i], j);
    rncp = rncp * parts.part[i].n_member;
    ncp = ncp + rncp;
    dp = dp + power(parts.part[i].n_member, 2);
    for (int j = parts.part[i].n_member; j > 0; j--) {
      res[n_res - j] = (char **)malloc(cfg.n_qid * sizeof(char *));
      for (int k = 0; k < cfg.n_qid; k++) {
        res[n_res - j][k] =
            deanonymized_range(k, qi.order[k][parts.part[i].low[k]],
                               qi.order[k][parts.part[i].high[k]]);
      }
    }
  }
  ncp = ncp / cfg.n_qid;
  ncp = ncp / subjs.n_subj;
  ncp = ncp * 100;

  write_to_file(res);

  printf("NCP %0.2f%%\n", ncp);
  printf("Running time %0.10f sec\n", cpu_time_used);

  int idx = 0;
  for (int i = 0; i < parts.n_part; i++) {
    idx = idx + parts.part[i].n_member;
    for (int j = parts.part[i].n_member; j > 0; j--) {
      for (int k = 0; k < cfg.n_qid; k++) {
        free(res[idx - j][k]);
      }
      free(res[idx - j]);
    }
  }
  free(res);

  for (int i = 0; i < subjs.n_subj; i++)
    free(data[i]);
  free(data);
}

int main(int argc, char **argv) {
  parse_dataset();
  mondrian();
  free_mem();
  return 0;
}
