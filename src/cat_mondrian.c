#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int MAX_ROW = 1024;
char *DATASET = "../datasets/adults.csv";
/* struct config parse_args(int argc, char **argv) { */
/*   char *attributes; */
/*   struct config args = {DEFAULT_REC, DEFAULT_ATT, DEFAULT_K, DEFAULT_DATA, */
/*                         DEFAULT_MAP}; */
/*   int c; */
/*   while ((c = getopt(argc, argv, ":k:r:a:f:")) != -1) { */
/*     switch (c) { */
/*     case 'k': // k_range */
/*       args.k = atoi(optarg); */
/*       break; */
/*     case 'r': // n_records */
/*       args.n_records = atoi(optarg); */
/*       break; */
/*     case 'a': // n_attributes */
/*       args.n_attributes = atoi(optarg); */
/*       break; */
/*     case 'd': // dataset path */
/*       args.map_path = optarg; */
/*       break; */
/*     case 'm': // map path */
/*       args.map_path = optarg; */
/*       break; */
/*     case '?': */
/*       fprintf(stderr, */
/*               "Usage: %s [OPTION...]\n" */
/*               "  -k <value>\t\tSet the K-anonimity\n" */
/*               "  -r <value>\t\tSet the number of records\n" */
/*               "  -a <value>\t\tSet number of attributes\n" */
/*               "  -f <value>\t\tSet the file path to anonimyze\n", */
/*               *argv); */
/*       exit(1); */
/*     case ':': */
/*       fprintf(stderr, "Error: option -%c requires an argument\n", optopt); */
/*       exit(1); */
/*     } */
/*   } */
/*   return args; */
/* } */

char *strip(char *str) {
  while (isspace((unsigned char)*str)) {
    str++;
  }
  return str;
}

struct category {
  int index;
  int n_values;
  char **values;
};

struct subject {
  char **attr_values;
  int n_attr_num;
  int *n_attr_num_indexes;
};

struct subjects {
  int n_subj;
  struct subject *subj;
};

struct config {
  int n_attr;
  char **attr_names;
  int n_qid;
  int *qid_indexes;
  struct category *cat;
};

void parse_dataset(struct config *cfg, struct subjects *subjs) {
  FILE *fp;
  char row[MAX_ROW];
  char *res;
  char *token;
  int idx_row = 0;
  int idx_token = 0;
  int digit = 0;

  fp = fopen("../datasets/adults.csv", "r");

  while (!feof(fp)) {
    res = fgets(row, MAX_ROW, fp);
    if (res != NULL) {
      /* printf("Row: %s\n", row); */
      idx_token = 0;

      if (idx_row == 0 || idx_row == 1) {
        if (idx_row == 0) {
          for (int i = 0; row[i]; i++)
            cfg->n_attr += (row[i] == ',');
          cfg->n_attr++;
          cfg->attr_names = (char **)malloc(cfg->n_attr * sizeof(char *));
          printf("Nº attributes: %d\n", cfg->n_attr);
        } else {
          for (int i = 0; row[i]; i++)
            cfg->n_qid += (row[i] == ',');
          cfg->n_qid++;
          cfg->qid_indexes = (int *)malloc(cfg->n_qid * sizeof(int));
          printf("\nNº QID: %d\n", cfg->n_qid);
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
            printf("attr: %s\n", cfg->attr_names[idx_token - 1]);
          } else {
            cfg->qid_indexes[idx_token++] = atoi(token);
            printf("qid: %d = %s\n", cfg->qid_indexes[idx_token - 1],
                   cfg->attr_names[cfg->qid_indexes[idx_token - 1]]);
          }
          token = strtok(NULL, ",");
        }
      } else {
        if (idx_row == 2)
          printf("\nLines:\n");
        subjs->n_subj++;
        subjs->subj = (struct subject *)realloc(
            subjs->subj, subjs->n_subj * sizeof(struct subject));
        subjs->subj[subjs->n_subj - 1].attr_values =
            (char **)malloc(cfg->n_attr * sizeof(char *));
        token = strtok(row, ",");
        while (token != NULL) {
          if (*token == ' ')
            token++;
          for (int idx = 0; token[idx]; idx++) {
            if ((token[idx] != '\0') && (digit = isdigit(token[idx])))
              continue;
            else
              break;
          }
          if (digit) {
            subjs->subj[subjs->n_subj - 1].n_attr_num++;
            subjs->subj[subjs->n_subj - 1].n_attr_num_indexes = (int *)realloc(
                subjs->subj[subjs->n_subj - 1].n_attr_num_indexes,
                subjs->subj[subjs->n_subj - 1].n_attr_num * sizeof(int));
            subjs->subj[subjs->n_subj - 1]
                .n_attr_num_indexes[subjs->subj[subjs->n_subj - 1].n_attr_num -
                                    1] = idx_token;
          }
          subjs->subj[subjs->n_subj - 1].attr_values[idx_token] = strdup(token);
          printf("attr: %s = %s\n", cfg->attr_names[idx_token],
                 subjs->subj[subjs->n_subj - 1].attr_values[idx_token]);
          token = strtok(NULL, ",");
          idx_token++;
        }
        printf("Numeric indexes: %d\n",
               subjs->subj[subjs->n_subj - 1].n_attr_num);
        if (idx_token != cfg->n_attr) {
          fprintf(stderr,
                  "Error: Invalid dataset. "
                  "Number of attributes in line %d does not "
                  "coincide with n_attr.",
                  idx_row);
          exit(1);
        }
        break;
      }
      idx_row++;
    }
  }
}

int main(int argc, char **argv) {
  struct config cfg = {0, NULL, 0, NULL, NULL};
  struct subjects subjs = {0, NULL};
  parse_dataset(&cfg, &subjs);
  printf("n_attr: %d, n_qid: %d\n", cfg.n_attr, cfg.n_qid);
  return 0;
}
