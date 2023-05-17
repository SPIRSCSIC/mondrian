#include "utils.h"

config cfg = {0, NULL, 0, NULL, 0, NULL, NULL};
subjects subjs = {0, NULL};
quasi qi = {NULL, NULL, NULL};
partitions parts = {0, NULL};

int choose_dimension(partition *part) {
  int max_width = -1;
  int max_dim = -1;
  for (int i = 0; i < cfg.n_qid; i++) {
    if (!part->allow[i])
      continue;
    int norm_width = normalized_width(part, i);
    if (norm_width > max_width) {
      max_width = norm_width;
      max_dim = i;
    }
  }
  return max_dim;
}

void mondrian_init() {
  qi.range = (int *)malloc(cfg.n_qid * sizeof(int));
  qi.order = (int **)malloc(cfg.n_qid * sizeof(int *));
  qi.dict = (dictionary *)malloc(cfg.n_qid * sizeof(dictionary));
  int *unique = (int *)malloc(cfg.n_qid * sizeof(int));
  for (int i = 0; i < cfg.n_qid; i++) {
    qi.dict[i] = (dictionary){0, NULL};
    qi.order[i] = NULL;
    unique[i] = 0;
  }
  for (int i = 0; i < subjs.n_subj; i++) {
    for (int j = 0; j < cfg.n_qid; j++) {
      if (int_in_list(qi.order[j], unique[j], subjs.attr_values[i][j]) == -1) {
        unique[j]++;
        qi.order[j] = (int *)realloc(qi.order[j], unique[j] * sizeof(int));
        qi.order[j][unique[j] - 1] = subjs.attr_values[i][j];
      }
    }
  }

  for (int i = 0; i < cfg.n_qid; i++) {
    qsort(qi.order[i], unique[i], sizeof(int), compare);
    printf("QI_ORDER: [");
    for (int j = 0; j < unique[i] - 1; j++) {
      printf("%d, ", qi.order[i][j]);
    }
    printf("%d", qi.order[i][unique[i] - 1]);
    printf("]\n");
    qi.range[i] = qi.order[i][unique[i] - 1] - qi.order[i][0];
    printf("QI_RANGE: %d\n", qi.range[i]);
    qi.dict[i].n_tuple = unique[i];
    qi.dict[i].tuple = (tuple *)malloc(unique[i] * sizeof(tuple));
    for (int j = 0; j < unique[i]; j++) {
      qi.dict[i].tuple[j].key = qi.order[i][j];
      qi.dict[i].tuple[j].value = j;
    }
    printf("QI_DICT: {");
    for (int j = 0; j < unique[i] - 1; j++) {
      printf("%d: %d, ", qi.dict[i].tuple[j].key, qi.dict[i].tuple[j].value);
    }
    printf("%d: %d", qi.dict[i].tuple[unique[i] - 1].key,
           qi.dict[i].tuple[unique[i] - 1].value);
    printf("}\n");
  }
  free(unique);
}

void partition_init(partition *part, int *low, int *high) {
  part->low = (int *)malloc(cfg.n_qid * sizeof(int));
  memcpy(part->low, low, cfg.n_qid * sizeof(int));
  part->high = (int *)malloc(cfg.n_qid * sizeof(int));
  memcpy(part->high, high, cfg.n_qid * sizeof(int));
  part->n_member = cfg.n_qid;
  part->member = (int **)malloc(cfg.n_qid * sizeof(int *));
  part->n_allow = cfg.n_qid; // array of 8 elements, each value is 1
  part->allow = (int *)malloc(cfg.n_qid * sizeof(int));
  for (int i = 0; i < cfg.n_qid; i++) {
    part->member[i] = (int *)malloc(qi.dict[i].n_tuple * sizeof(int));
    memcpy(part->member[i], qi.order[i], qi.dict[i].n_tuple * sizeof(int));
    part->allow[i] = 1;
  }
}

void frequency(partition *part, int dim) {
  dictionary dict = {0, NULL};
  raise(SIGINT);
  for (int i = 0; i < part->n_member; i++) {
    increase_dict_value(&dict, part->member[i][dim]);
  }
}

void find_median(partition *part, int dim) { frequency(part, dim); }

void anonymize_strict(partition *part) {
  if (!part->n_allow) {
    add_partition(part);
    return;
  }
  for (int i = 0; i < part->n_allow; i++) {
    int dim = choose_dimension(part);
    if (dim == -1) {
      fprintf(stderr, "Error: dim = -1");
      raise(SIGINT);
    }
    find_median(part, dim);
  }
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
  partition_init(&part, low, high);
  /* set up clock to calculate execution time */
  anonymize_strict(&part);
  free(low);
  free(high);
}

int main(int argc, char **argv) {
  parse_dataset();
  mondrian();
  free_mem();
  return 0;
}
