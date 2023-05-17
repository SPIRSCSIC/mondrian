#include "utils.h"

int MAX_ROW = 1024;
char *DATASET = "../datasets/adults.csv";

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

void add_to_partition(partition *part, int *record, int length) {
  part->n_member++;
  part->member = (int **)realloc(part->member, part->n_member * sizeof(int *));
  int *tmp = (int *)malloc(length * sizeof(int));
  memcpy(tmp, record, length);
  part->member[part->n_member - 1] = tmp;
}

void addn_to_partition(partition *part, int n_records, int **records,
                       int *lengths) {
  for (int i = 0; i < n_records; i++) {
    add_to_partition(part, records[i], lengths[i]);
  }
}

void add_partition(partition *part) {
  parts.n_part++;
  parts.part =
      (partition *)realloc(parts.part, parts.n_part * sizeof(partition));
  parts.part[parts.n_part - 1] = *part;
}

double normalized_width(partition *part, int index) {
  printf("part->high: %d, part->low: %d, order[high]: %d, order[low]: %d\n",
         qi.order[index][part->high[index]], part->high[index],
         qi.order[index][part->low[index]], part->low[index]);
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
            if (int_in_list(cfg.qid_indexes, cfg.n_qid, idx_token) > -1) {
              cfg.cat[idx_attr].qid_index = idx_token;
              if (digit) {
                if (idx_row == 2) {
                  cfg.n_attr_num++;
                  cfg.n_attr_num_indexes = (int *)realloc(
                      cfg.n_attr_num_indexes, cfg.n_attr_num * sizeof(int));
                  cfg.n_attr_num_indexes[cfg.n_attr_num - 1] = idx_token;
                } else {
                  if (int_in_list(cfg.n_attr_num_indexes, cfg.n_attr_num,
                                  idx_token) == -1) {
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
  free(qi.range);
  for (int i = 0; i < cfg.n_qid; i++) {
    free(qi.order[i]);
    free(qi.dict[i].tuple);
  }
  free(qi.order);
  free(qi.dict);
  for (int i = 0; i < subjs.n_subj; i++) {
    free(subjs.attr_values[i]);
  }
  free(subjs.attr_values);
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
  for (int i = 0; i < cfg.n_qid; i++) {
    free(cfg.cat[i].values);
  }
  free(cfg.cat);
  free(cfg.attr_names);
  free(cfg.qid_indexes);
}

void increase_dict_value(dictionary *dict, int key) {
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
