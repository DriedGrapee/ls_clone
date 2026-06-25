#include "entry.h"
#include "mode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

/* qsort comparator: order two entry_info pointers by name (locale-aware).
   qsort passes the *addresses* of array elements, and each element is an
   `entry_info *`, so `a`/`b` are really `entry_info * const *` -- dereference
   once to recover the struct pointer. */
static int entry_name_cmp(const void *a, const void *b) {
    const entry_info *ea = *(const entry_info *const *)a;
    const entry_info *eb = *(const entry_info *const *)b;
    return strcoll(ea->name, eb->name);
}

void sort_entries(entry_info **entries, size_t n) {
    qsort(entries, n, sizeof *entries, entry_name_cmp);
}

int add_entry(entry_info *entries[], const char *dir, const char *name, int *number_of_entries) {

    // add overflow protection

    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    struct stat st;
    if (lstat(fullpath, &st) < 0) {
        perror(name);
        return 1;
    }

    entry_info *e = malloc(sizeof(entry_info));
    if (!e) { perror("malloc"); return 1; }
    entries[*number_of_entries] = e;

    mode_string(st.st_mode, e->modes);

    e->link = (unsigned long)st.st_nlink;
    e->mem  = (long)st.st_size;

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    snprintf(e->usr, sizeof(e->usr), "%s", pw ? pw->pw_name : "?");  // copy, don't borrow
    snprintf(e->grp, sizeof(e->grp), "%s", gr ? gr->gr_name : "?");
    snprintf(e->name, sizeof(e->name), "%s", name);

    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(e->timebuf, sizeof e->timebuf, "%b %e %H:%M", tm);

    (*number_of_entries)++;

    return 0;
}

void get_max_length(entry_info *entries[], int *number_of_entries,
                    int *slk_max_len,
                    int *usr_max_len,
                    int *grp_max_len,
                    int *mem_max_len) {

    int slk_size = snprintf(NULL, 0, "%lu", (unsigned long)entries[*number_of_entries - 1]->link);
    int usr_size = (int)strlen(entries[*number_of_entries - 1]->usr);
    int grp_size = (int)strlen(entries[*number_of_entries - 1]->grp);
    int mem_size = snprintf(NULL, 0, "%ld", (long)entries[*number_of_entries - 1]->mem);

    if (slk_size > *slk_max_len) {
        *slk_max_len = slk_size;
    }
    if (usr_size > *usr_max_len) {
        *usr_max_len = usr_size;
    }
    if (grp_size > *grp_max_len) {
        *grp_max_len = grp_size;
    }
    if (mem_size > *mem_max_len) {
        *mem_max_len = mem_size;
    }
}
