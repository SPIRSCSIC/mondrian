#ifndef MONDRIAN_H
#define MONDRIAN_H

#include "common.h"


extern config cfg;
extern subjects subjs;
extern quasi qi;
extern partitions parts;

void anonymize_strict(partition *);
void anonymize_relaxed(partition *);
void mondrian_init();
void mondrian();

#endif
