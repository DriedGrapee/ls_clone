#include "listing.h"
#include "entry.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/* qsort comparator: order two entry_info pointers by name (locale-aware).
   qsort passes the *addresses* of array elements, and each element is an
   `entry_info *`, so `a`/`b` are really `entry_info * const *` -- dereference
   once to recover the struct pointer. */
static int entry_name_cmp(const void *a, const void *b) {
    const entry_info *ea = *(const entry_info *const *)a;
    const entry_info *eb = *(const entry_info *const *)b;
    return strcoll(ea->name, eb->name);
}

/* qsort comparator for an array of C strings. Each element is a `char *`, so
   the addresses qsort passes are `char *const *` -- dereference once. */
static int name_cmp(const void *a, const void *b) {
    const char *sa = *(const char *const *)a;
    const char *sb = *(const char *const *)b;
    return strcoll(sa, sb);
}

void free_listing(listing *list) {
    for (int i = 0; i < list->count; ++i) {
        free(list->items[i]);
    }
    free(list->items);
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

int read_directory(const char *path, int show_all, int do_long, listing *out) {
    out->items    = NULL;
    out->count    = 0;
    out->capacity = 0;
    out->is_long  = do_long;
    out->slk_max  = 0;
    out->usr_max  = 0;
    out->grp_max  = 0;
    out->mem_max  = 0;

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    out->capacity = 8;
    out->items = malloc(out->capacity * sizeof(*out->items));
    if (out->items == NULL) {
        out->capacity = 0;
        closedir(dir);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        if (out->count == out->capacity) {
            int new_capacity = out->capacity + 4;
            void **temp = realloc(out->items, new_capacity * sizeof(*temp));
            if (temp == NULL) {
                fprintf(stderr, "Reallocation failed! Old memory is still intact.\n");
                closedir(dir);
                free_listing(out);
                return 1;
            }
            out->items    = temp;
            out->capacity = new_capacity; 
        }

        if (!do_long) {
            // Own the name: readdir reuses entry->d_name's storage on later calls.
            char *copy = strdup(entry->d_name);
            if (copy == NULL) {
                perror("strdup");
                closedir(dir);
                free_listing(out);
                return 1;
            }
            out->items[out->count++] = copy;
        } else {
            if (add_entry((entry_info **)out->items, path, entry->d_name, &out->count) != 0) {
                printf("Got an error from add_entry");
                continue;
            }
            get_max_length((entry_info **)out->items, &out->count,
                            &out->slk_max, &out->usr_max,
                            &out->grp_max, &out->mem_max);
        }
    }

    closedir(dir);
    return 0;
}

void sort_listing(listing *list) {
    if (list->is_long) {
        qsort(list->items, list->count, sizeof *list->items, entry_name_cmp);
    } else {
        qsort(list->items, list->count, sizeof *list->items, name_cmp);
    }
}

void print_listing(const listing *list, int reverse) {
    for (int k = 0; k < list->count; ++k) {
        int i = reverse ? list->count - 1 - k : k;
        if (list->is_long) {
            print_long(list->items[i], list->slk_max, list->usr_max,
                       list->grp_max, list->mem_max);
        } else {
            printf("%s\n", (char *)list->items[i]);
        }
    }
}
