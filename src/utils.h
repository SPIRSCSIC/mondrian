#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int **order;
  dictionary *dict;
} quasi;

typedef struct {
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

extern int MAX_ROW;
extern char *DATASET;
extern config cfg;
extern subjects subjs;
extern quasi qi;
extern partitions parts;

int string_in_list(char **, int, char *);

int int_in_list(int *, int, int);

void add_to_partition(partition *, int *, int);

void addn_to_partition(partition *, int, int **, int *);

void add_partition(partition *);

double normalized_width(partition *, int);

void parse_dataset();

int compare(const void *, const void *);

void free_mem();

void increase_dict_value(dictionary *, int);

#endif
