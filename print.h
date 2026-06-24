#ifndef PRINT_H
#define PRINT_H

#include "entry.h"

/*
 * Print a single entry in `ls -l` long format, right-justifying the link
 * count and size and left-justifying the user and group, each padded to the
 * supplied column width.
 */
void print_long(entry_info *entry,
                int link_size,
                int usr_size,
                int grp_size,
                int mem_size);

#endif /* PRINT_H */
