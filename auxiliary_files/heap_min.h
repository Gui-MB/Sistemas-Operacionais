#ifndef HEAP_MIN_H
#define HEAP_MIN_H

#include "../processes/process_manager.h"

void heap_min_init(void);
void heap_min_insert(Process *p);
Process *heap_min_pop(void);

#endif