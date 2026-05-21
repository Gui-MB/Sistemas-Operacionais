#ifndef CFS_H
#define CFS_H

#include "../auxiliary_files/processes.h"

void cfs_init(void);
void cfs_destroy(void);
int get_next_cfs(int current_time);

#endif