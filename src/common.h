#ifndef UTILS_H
#define UTILS_H

typedef struct {
  int qid_index;
  int n_values;
  char **values;
} category;

typedef struct {
  int n_subj;
  int **attr_values;
} subjects;

typedef struct {
  int n_attr;
  char **attr_names;
  int n_attr_num;
  int *n_attr_num_indexes;
  int n_qid;
  int *qid_indexes;
  category *cat;
} config;

typedef struct {
  int key;
  int value;
} tuple;

// this does not take into account duplicated keys
typedef struct {
  int n_tuple;
  tuple *tuple;
} dictionary;

typedef struct {
  int *range;
  int *n_order;
  int **order;
  int **original; // have n_subj length
  dictionary *dict;
} quasi;

typedef struct {
  int id;
  int *low;
  int *high;
  int n_member;
  int **member;
  int n_allow;
  int *allow;
} partition;

typedef struct {
  int n_part;
  partition *part;
} partitions;

typedef struct {
  int n_elem;
  int *elements;
} tracker;

typedef struct {
  int split;
  int next;
  int low;
  int high;
} frequency;

typedef struct {
  int n_cat;
  char **cat;
} range;

extern int MAX_ROW;
extern char *DATASET;
extern char *OUTPUT;
extern char *MODE;
extern char *TEST;
extern int ANON;
extern int RES;
extern int GL_K;
extern config cfg;
extern subjects subjs;
extern quasi qi;
extern partitions parts;
extern tracker added;
extern tracker freed;
extern partitions created;

void parse_dataset();
void partition_init(partition *, int **, int, int *, int *);
void store_in_created(partition *);
void store_in_parts(partition *);
void add_to_partition(partition *, int *);
void addn_to_partition(partition *, int, int **);
void add_to_list(int ***, int *, int *);
double normalized_width(partition *, int);
frequency *find_median(partition *, int);
int choose_dimension(partition *);
int compare(const void *, const void *);
int string_in_list(char **, int, char *);
int int_in_list(int *, int, int);
void dict_value_inc(dictionary *, int);
int dict_sum(dictionary *);
int dict_value(dictionary *, int);
int * dict_keys(dictionary *);
double power(double, int);
range * range_categories(int, int, int);
char * deanonymized_range(int, int, int);
char * anonymized_range(int, int, int);
void write_to_file(char ***);
void usage(int);
void free_partition(partition *);
void free_mem();

#endif
