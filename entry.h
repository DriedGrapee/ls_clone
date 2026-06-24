#ifndef ENTRY_H
#define ENTRY_H

#include <limits.h>

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256
#endif

typedef struct __attribute__((packed)) {
    char          name[256];
    char          usr[LOGIN_NAME_MAX];
    char          grp[LOGIN_NAME_MAX];
    char          timebuf[64];
    char          modes[11];
    unsigned long link;
    long          mem;
} entry_info;

/*
 * Stat `dir/name`, populate a freshly allocated entry_info, and store it at
 * `entries[*number_of_entries]`, then increment `*number_of_entries`.
 * Returns 0 on success or 1 on failure. The allocated entry is owned by the
 * caller, who is responsible for freeing it.
 */
int add_entry(entry_info *entries[], const char *dir, const char *name,
              int *number_of_entries);

/*
 * Grow the running column widths so they can accommodate the most recently
 * added entry (the last one currently in `entries`).
 */
void get_max_length(entry_info *entries[], int *number_of_entries,
                    int *slk_max_len,
                    int *usr_max_len,
                    int *grp_max_len,
                    int *mem_max_len);

#endif /* ENTRY_H */
