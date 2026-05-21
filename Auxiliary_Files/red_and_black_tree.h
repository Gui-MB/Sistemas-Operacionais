#ifndef RED_AND_BLACK_TREE_H
#define RED_AND_BLACK_TREE_H

#include "processes.h"

void rb_init(void);
void rb_destroy(void);
void rb_insert(Process *p);
Process *rb_minimum_process(void);

#endif
