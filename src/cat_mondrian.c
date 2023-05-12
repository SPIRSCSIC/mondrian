#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int MAX_ROW = 1024;
char *DATASET = "../datasets/adults.csv";

struct category {
  int qid_index;
  int n_values;
  char **values;
};

struct subjects {
  int n_subjs;
  int **attr_values;
};

struct config {
  int n_attr;
  char **attr_names;
  int n_attr_num;
  int *n_attr_num_indexes;
  int n_qid;
  int *qid_indexes;
  struct category *cats;
};

struct tuple {
  int index;
  int value;
};

struct dictionary {
  int n_tuples;
  struct tuple *tuples;
};

struct qi_data {
  int *range;
  int **order;
  struct dictionary *dict;
};

int string_in_list(char **list, int length, char *value) {
  for (int i = 0; i < length; i++)
    if (strcmp(list[i], value) == 0)
      return i;
  return -1;
}

int int_in_list(int *list, int length, int value) {
  for (int i = 0; i < length; i++)
    if (list[i] == value)
      return i;
  return -1;
}

void parse_dataset(struct config *cfg, struct subjects *subjs) {
  FILE *fp;
  char row[MAX_ROW];
  char *res;
  char *token;
  int idx_row = 0;
  int idx_token = 0;
  int idx_attr = 0;
  int digit = 0;

  fp = fopen("../datasets/adults.csv", "r");

  while (!feof(fp)) {
    res = fgets(row, MAX_ROW, fp);
    if (res != NULL) {
      /* printf("Row: %s\n", row); */
      if (!strchr(row, '?')) {
        idx_token = 0;

        if (idx_row == 0 || idx_row == 1) {
          if (idx_row == 0) {
            for (int i = 0; row[i]; i++)
              cfg->n_attr += (row[i] == ',');
            cfg->n_attr++;
            cfg->attr_names = (char **)malloc(cfg->n_attr * sizeof(char *));
            /* printf("Nº attributes: %d\n", cfg->n_attr); */
          } else {
            for (int i = 0; row[i]; i++)
              cfg->n_qid += (row[i] == ',');
            cfg->n_qid++;
            cfg->qid_indexes = (int *)malloc(cfg->n_qid * sizeof(int));
            /* printf("\nNº QID: %d\n", cfg->n_qid); */
          }
          token = strtok(row, ",");
          while (token != NULL) {
            if (*token == ' ')
              token++;
            int len_t = strlen(token);
            if (token[len_t - 1] == '\n')
              token[len_t - 1] = '\0';
            if (idx_row == 0) {
              cfg->attr_names[idx_token++] = strdup(token);
              /* printf("attr: %s\n", cfg->attr_names[idx_token-1]); */
            } else {
              cfg->qid_indexes[idx_token++] = atoi(token);
              /* printf("qid: %d = %s\n", cfg->qid_indexes[idx_token-1], */
              /*        cfg->attr_names[cfg->qid_indexes[idx_token-1]]); */
            }
            token = strtok(NULL, ",");
          }
        } else {
          if (idx_row == 2) {
            /* printf("\nLines:\n"); */
            cfg->cats = (struct category *)realloc(
                cfg->cats, cfg->n_qid * sizeof(struct category));
            for (int i = 0; i < cfg->n_qid; i++)
              cfg->cats[i] = (struct category){0, 0, NULL};
          }
          subjs->n_subjs++;
          subjs->attr_values = (int **)realloc(subjs->attr_values,
                                               subjs->n_subjs * sizeof(int *));
          subjs->attr_values[subjs->n_subjs - 1] =
              (int *)malloc(cfg->n_qid * sizeof(int));
          token = strtok(row, ",");
          idx_attr = 0;
          while (token != NULL) {
            if (*token == ' ')
              token++;
            int len_t = strlen(token);
            if (token[len_t - 1] == '\n')
              token[len_t - 1] = '\0';
            for (int i = 0; token[i]; i++) {
              if ((token[i] != '\0') && (digit = isdigit(token[i])))
                continue;
              else
                break;
            }
            if (int_in_list(cfg->qid_indexes, cfg->n_qid, idx_token) > -1) {
              cfg->cats[idx_attr].qid_index = idx_token;
              if (digit) {
                if (idx_row == 2) {
                  cfg->n_attr_num++;
                  cfg->n_attr_num_indexes = (int *)realloc(
                      cfg->n_attr_num_indexes, cfg->n_attr_num * sizeof(int));
                  cfg->n_attr_num_indexes[cfg->n_attr_num - 1] = idx_token;
                } else {
                  if (int_in_list(cfg->n_attr_num_indexes, cfg->n_attr_num,
                                  idx_token) == -1) {
                    fprintf(
                        stderr,
                        "Error: Invalid attribute type. "
                        "Attribute with index %d (L:%d) should be categorical.",
                        idx_attr, idx_row);
                    exit(1);
                  }
                }
                subjs->attr_values[subjs->n_subjs - 1][idx_attr] = atoi(token);
              } else {
                if (idx_row > 2) {
                  if (int_in_list(cfg->n_attr_num_indexes, cfg->n_attr_num,
                                  idx_token) > -1) {
                    fprintf(
                        stderr,
                        "Error: Invalid attribute type. "
                        "Attribute with index %d (L:%d) should be numerical.",
                        idx_attr, idx_row);
                    exit(1);
                  }
                }
                int tmp = string_in_list(cfg->cats[idx_attr].values,
                                         cfg->cats[idx_attr].n_values, token);
                if (tmp == -1) {
                  cfg->cats[idx_attr].n_values++;
                  cfg->cats[idx_attr].values = (char **)realloc(
                      cfg->cats[idx_attr].values,
                      cfg->cats[idx_attr].n_values * sizeof(char *));
                  cfg->cats[idx_attr].values[cfg->cats[idx_attr].n_values - 1] =
                      strdup(token);
                  subjs->attr_values[subjs->n_subjs - 1][idx_attr] =
                      cfg->cats[idx_attr].n_values - 1;
                } else {
                  subjs->attr_values[subjs->n_subjs - 1][idx_attr] = tmp;
                }
              }
              /* printf("attr: %s = %d\n", cfg->attr_names[idx_token], */
              /*        subjs->attr_values[subjs->n_subjs-1][idx_attr]); */
              idx_attr++;
            }
            token = strtok(NULL, ",");
            idx_token++;
          }
          if (idx_token != cfg->n_attr) {
            fprintf(stderr,
                    "Error: Invalid dataset. "
                    "Number of attributes in line %d does not "
                    "coincide with n_attr.",
                    idx_row);
            exit(1);
          }
        }
      }
    }
    idx_row++;
  }
}

int compare(const void *a, const void *b) {
  int f_a = *((int *)a);
  int f_b = *((int *)b);
  return f_a - f_b;
}

void free_mem(struct config *cfg, struct subjects *subjs, struct qi_data *qi) {
  free(qi->range);
  for (int i = 0; i < cfg->n_qid; i++) {
    free(cfg->cats[i].values);
    free(qi->order[i]);
    free(qi->dict[i].tuples);
  }
  free(qi->order);
  free(qi->dict);
  free(cfg->cats);
  free(cfg->attr_names);
  free(cfg->qid_indexes);
  for (int i = 0; i < subjs->n_subjs; i++) {
    free(subjs->attr_values[i]);
  }
  free(subjs->attr_values);
}

void mondrian_init(struct config *cfg, struct subjects *subjs,
                   struct qi_data *qi) {
  qi->range = (int *)malloc(cfg->n_qid * sizeof(int));
  qi->order = (int **)malloc(cfg->n_qid * sizeof(int *));
  qi->dict =
      (struct dictionary *)malloc(cfg->n_qid * sizeof(struct dictionary));
  int *unique = (int *)malloc(cfg->n_qid * sizeof(int));
  for (int i = 0; i < cfg->n_qid; i++) {
    qi->dict[i] = (struct dictionary){0, NULL};
    unique[i] = 0;
  }
  for (int i = 0; i < subjs->n_subjs; i++) {
    for (int j = 0; j < cfg->n_qid; j++) {
      if (int_in_list(qi->order[j], unique[j], subjs->attr_values[i][j]) ==
          -1) {
        unique[j]++;
        qi->order[j] = (int *)realloc(qi->order[j], unique[j] * sizeof(int));
        qi->order[j][unique[j] - 1] = subjs->attr_values[i][j];
      }
    }
  }
  /* raise(SIGINT); */
  for (int i = 0; i < cfg->n_qid; i++) {
    qsort(qi->order[i], unique[i], sizeof(int), compare);
    printf("QI_ORDER: [");
    for (int j = 0; j < unique[i] - 1; j++) {
      printf("%d, ", qi->order[i][j]);
    }
    printf("%d", qi->order[i][unique[i] - 1]);
    printf("]\n");
    qi->range[i] = qi->order[i][unique[i] - 1] - qi->order[i][0];
    printf("QI_RANGE: %d\n", qi->range[i]);
    qi->dict[i].n_tuples = unique[i];
    qi->dict[i].tuples =
        (struct tuple *)malloc(unique[i] * sizeof(struct tuple));
    for (int j = 0; j < unique[i]; j++) {
      qi->dict[i].tuples[j].index = qi->order[i][j];
      qi->dict[i].tuples[j].value = j;
    }
    printf("QI_DICT: {");
    for (int j = 0; j < unique[i] - 1; j++) {
      printf("%d: %d, ", qi->dict[i].tuples[j].index,
             qi->dict[i].tuples[j].value);
    }
    printf("%d: %d", qi->dict[i].tuples[unique[i] - 1].index,
           qi->dict[i].tuples[unique[i] - 1].value);
    printf("}\n");
  }
  free(unique);
}

int main(int argc, char **argv) {
  struct config cfg = {0, NULL, 0, NULL, 0, NULL, NULL};
  struct subjects subjs = {0, NULL};
  parse_dataset(&cfg, &subjs);
  struct qi_data qi = {NULL, NULL, NULL};
  mondrian_init(&cfg, &subjs, &qi);
  free_mem(&cfg, &subjs, &qi);
  return 0;
}
