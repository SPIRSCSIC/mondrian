#include "utils.h"
#include <signal.h>

int MAX_ROW = 1024;
char *DATASET = "../datasets/adults.csv";
char *OUTPUT = "output.csv";
int GL_K = 10;

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

void add_to_partition(partition *part, int *record) {
  part->n_member++;
  part->member = (int **)realloc(part->member, part->n_member * sizeof(int *));
  int *tmp = (int *)malloc(cfg.n_qid * sizeof(int));
  memcpy(tmp, record, cfg.n_qid * sizeof(int));
  part->member[part->n_member - 1] = tmp;
}

void addn_to_partition(partition *part, int n_records, int **records) {
  for (int i = 0; i < n_records; i++) {
    add_to_partition(part, records[i]);
  }
}

void add_partition(partition *part) {
  parts.n_part++;
  parts.part =
      (partition *)realloc(parts.part, parts.n_part * sizeof(partition));
  parts.part[parts.n_part - 1] = *part;
}

double normalized_width(partition *part, int index) {
  int diff =
      qi.order[index][part->high[index]] - qi.order[index][part->low[index]];
  double range = qi.range[index];
  return diff / range;
}

void parse_dataset() {
  FILE *fp;
  char row[MAX_ROW];
  char *res;
  char *token;
  int idx_row = 0;
  int idx_token = 0;
  int idx_attr = 0;
  int digit = 0;

  fp = fopen(DATASET, "r");

  while (!feof(fp)) {
    res = fgets(row, MAX_ROW, fp);
    if (res != NULL) {
      /* printf("Row: %s\n", row); */
      if (!strchr(row, '?')) {
        idx_token = 0;

        if (idx_row == 0 || idx_row == 1) {
          if (idx_row == 0) {
            for (int i = 0; row[i]; i++)
              cfg.n_attr += (row[i] == ',');
            cfg.n_attr++;
            cfg.attr_names = (char **)malloc(cfg.n_attr * sizeof(char *));
            /* printf("Nº attributes: %d\n", cfg.n_attr); */
          } else {
            for (int i = 0; row[i]; i++)
              cfg.n_qid += (row[i] == ',');
            cfg.n_qid++;
            cfg.qid_indexes = (int *)malloc(cfg.n_qid * sizeof(int));
            /* printf("\nNº QID: %d\n", cfg.n_qid); */
          }
          token = strtok(row, ",");
          while (token != NULL) {
            if (*token == ' ')
              token++;
            int len_t = strlen(token);
            if (token[len_t - 1] == '\n')
              token[len_t - 1] = '\0';
            if (idx_row == 0) {
              cfg.attr_names[idx_token++] = strdup(token);
              /* printf("attr: %s\n", cfg.attr_names[idx_token-1]); */
            } else {
              cfg.qid_indexes[idx_token++] = atoi(token);
              /* printf("qid: %d = %s\n", cfg.qid_indexes[idx_token-1], */
              /*        cfg.attr_names[cfg.qid_indexes[idx_token-1]]); */
            }
            token = strtok(NULL, ",");
          }
        } else {
          if (idx_row == 2) {
            /* printf("\nLines:\n"); */
            cfg.cat =
                (category *)realloc(cfg.cat, cfg.n_qid * sizeof(category));
            for (int i = 0; i < cfg.n_qid; i++)
              cfg.cat[i] = (category){0, 0, NULL};
          }
          subjs.n_subj++;
          subjs.attr_values =
              (int **)realloc(subjs.attr_values, subjs.n_subj * sizeof(int *));
          subjs.attr_values[subjs.n_subj - 1] =
              (int *)malloc(cfg.n_qid * sizeof(int));
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
            if (int_in_list(cfg.qid_indexes, cfg.n_qid, idx_token) != -1) {
              cfg.cat[idx_attr].qid_index = idx_token;
              if (digit) {
                if (idx_row == 2) {
                  cfg.n_attr_num++;
                  cfg.n_attr_num_indexes = (int *)realloc(
                      cfg.n_attr_num_indexes, cfg.n_attr_num * sizeof(int));
                  cfg.n_attr_num_indexes[cfg.n_attr_num - 1] = idx_attr;
                } else {
                  if (int_in_list(cfg.n_attr_num_indexes, cfg.n_attr_num,
                                  idx_attr) == -1) {
                    fprintf(
                        stderr,
                        "Error: Invalid attribute type. "
                        "Attribute with index %d (L:%d) should be categorical.",
                        idx_attr, idx_row);
                    exit(1);
                  }
                }
                subjs.attr_values[subjs.n_subj - 1][idx_attr] = atoi(token);
              } else {
                if (idx_row > 2) {
                  if (int_in_list(cfg.n_attr_num_indexes, cfg.n_attr_num,
                                  idx_token) > -1) {
                    fprintf(
                        stderr,
                        "Error: Invalid attribute type. "
                        "Attribute with index %d (L:%d) should be numerical.",
                        idx_attr, idx_row);
                    exit(1);
                  }
                }
                int tmp = string_in_list(cfg.cat[idx_attr].values,
                                         cfg.cat[idx_attr].n_values, token);
                if (tmp == -1) {
                  cfg.cat[idx_attr].n_values++;
                  cfg.cat[idx_attr].values = (char **)realloc(
                      cfg.cat[idx_attr].values,
                      cfg.cat[idx_attr].n_values * sizeof(char *));
                  cfg.cat[idx_attr].values[cfg.cat[idx_attr].n_values - 1] =
                      strdup(token);
                  subjs.attr_values[subjs.n_subj - 1][idx_attr] =
                      cfg.cat[idx_attr].n_values - 1;
                } else {
                  subjs.attr_values[subjs.n_subj - 1][idx_attr] = tmp;
                }
              }
              /* printf("attr: %s = %d\n", cfg.attr_names[idx_token], */
              /*        subjs.attr_values[subjs.n_subjs-1][idx_attr]); */
              idx_attr++;
            }
            token = strtok(NULL, ",");
            idx_token++;
          }
          if (idx_token != cfg.n_attr) {
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
  fclose(fp);
}

int compare(const void *a, const void *b) {
  int f_a = *((int *)a);
  int f_b = *((int *)b);
  return f_a - f_b;
}

void free_mem() {
  for (int i = 0; i < parts.n_part; i++) {
    free(parts.part[i].low);
    free(parts.part[i].high);
    for (int j = 0; j < parts.part[i].n_member; j++) {
      free(parts.part[i].member[j]);
    }
    free(parts.part[i].member);
    free(parts.part[i].allow);
  }
  free(parts.part);

  for (int i = 0; i < subjs.n_subj; i++) {
    free(subjs.attr_values[i]);
  }
  free(subjs.attr_values);

  for (int i = 0; i < cfg.n_qid; i++) {
    free(qi.order[i]);
    free(qi.original[i]);
    free(qi.dict[i].tuple);
  }
  free(qi.range);
  free(qi.n_order);
  free(qi.order);
  free(qi.dict);

  for (int i = 0; i < cfg.n_qid; i++) {
    free(cfg.cat[i].values);
  }
  free(cfg.cat);
  free(cfg.attr_names);
  free(cfg.qid_indexes);
}

void dict_value_inc(dictionary *dict, int key) {
  int found = 0;
  for (int i = 0; i < dict->n_tuple; i++) {
    if (dict->tuple[i].key == key) {
      dict->tuple[i].value++;
      found = 1;
    }
  }
  if (!found) {
    dict->n_tuple++;
    dict->tuple = (tuple *)realloc(dict->tuple, dict->n_tuple * sizeof(tuple));
    tuple tmp = {key, 1};
    dict->tuple[dict->n_tuple - 1] = tmp;
  }
}

int dict_sum(dictionary *dict) {
  int sum = 0;
  for (int i = 0; i < dict->n_tuple; i++)
    sum += dict->tuple[i].value;
  return sum;
}

int dict_value(dictionary *dict, int key) {
  int res = 0;
  for (int i = 0; i < dict->n_tuple && !res; i++) {
    if (dict->tuple[i].key == key) {
      res = dict->tuple[i].value;
    }
  }
  return res;
}

int *dict_keys(dictionary *dict) {
  int *keys = (int *)malloc(dict->n_tuple * sizeof(int));
  for (int i = 0; i < dict->n_tuple; i++) {
    keys[i] = dict->tuple[i].key;
  }
  qsort(keys, dict->n_tuple, sizeof(int), compare);
  return keys;
}

double power(double x, int n) {
  if (n == 0)
    return 1;
  if (x == 0.0)
    return 0;
  return x * power(x, n - 1);
}

range *range_categories(int index, int start, int end) {
  range *rng = (range *)malloc(sizeof(range));
  if (!start && !end)
    rng->n_cat = 1;
  else
    rng->n_cat = end - start + 1;
  rng->cat = (char **)malloc(rng->n_cat * sizeof(char *));
  category cat = cfg.cat[index];
  for (int i = 0, j = start; i < rng->n_cat; i++, j++) {
    rng->cat[i] = cat.values[j];
  }
  return rng;
}

char *deanonymized_range(int index, int left, int right) {
  char *merged = NULL;
  /* printf("index: %d, left: %d, right: %d\n", index, left, right); */
  if (int_in_list(cfg.n_attr_num_indexes, cfg.n_attr_num, index) == -1) {
    range *rng = range_categories(index, left, right);
    /* printf("range: %d\n", rng->n_cat); */
    if (left == right && !left && !right) {
      /* printf("allocating %ld\n", (strlen(rng->cat[0])+1)); */
      merged = (char *)malloc((strlen(rng->cat[0]) + 1) * sizeof(char));
      sprintf(merged, "%s", rng->cat[0]);
    } else {
      int sz = 0;
      for (int i = 0; i < rng->n_cat; i++) {
        sz = sz + strlen(rng->cat[i]);
        /* printf("cat: %s\n", rng->cat[i]); */
      }
      sz = sz + rng->n_cat - 1; // add the space for ~ symbol
      /* printf("allocating %d\n", (sz+1)); */
      merged = (char *)malloc((sz + 1) * sizeof(char));
      int written = 0;
      for (int i = 0; i < rng->n_cat - 1; i++) {
        int tmp = sprintf(&merged[written], "%s~", rng->cat[i]);
        written = written + tmp;
      }
      sprintf(&merged[written], "%s", rng->cat[rng->n_cat - 1]);
    }
    /* printf("merged: %s, %ld\n", merged, strlen(merged)); */
  } else {
    merged = (char *)malloc(22 * sizeof(char));
    if (left == right)
      sprintf(merged, "%d", left);
    else
      sprintf(merged, "%d~%d", left, right);
  }
  return merged;
}

char *anonymized_range(int index, int left, int right) {
  /* assuming maximum value of int is 2147483647, len = 10 */
  /* len(int_max) + len(int_max) + "~" + \0 */
  char *merged = (char *)malloc(22 * sizeof(char));
  if (left == right)
    sprintf(merged, "%d", left);
  else
    sprintf(merged, "%d~%d", left, right);
  /* printf("merged: %s\n", merged); */
  return merged;
}

void write_to_file(char ***data) {
  FILE *fp;
  fp = fopen(OUTPUT, "w");

  for (int i = 0; i < subjs.n_subj; i++) {
    if (i)
      fprintf(fp, "\n");
    for (int j = 0; j < cfg.n_qid - 1; j++)
      fprintf(fp, "%s,", data[i][j]);
    fprintf(fp, "%s", data[i][cfg.n_qid - 1]);
  }
  fprintf(fp, "\n");
  fclose(fp);
}
