#ifndef MONDRIAN_H
#define MONDRIAN_H

#include "utils.h"

extern config cfg;
extern subjects subjs;
extern quasi qi;
extern partitions parts;

int choose_dimension(partition *);
void mondrian_init();
void partition_init(partition *, int **, int, int *, int *);
frequency *find_median(partition *, int);
void anonymize_strict(partition *part);
void mondrian();

#endif
