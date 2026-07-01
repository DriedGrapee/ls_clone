#ifndef LISTING_H
#define LISTING_H

/*
 * A collected directory listing. `items` holds `count` owned pointers:
 * entry_info* when `is_long` is set (collected for -l), otherwise char*
 * (owned copies of the names). The *_max fields are the long-format column
 * widths and are only meaningful when `is_long` is set.
 */
typedef struct {
    void **items;
    int    count;
    int    capacity;
    int    is_long;
    int    slk_max;
    int    usr_max;
    int    grp_max;
    int    mem_max;
} listing;

/*
 * Read directory `path` into `*out`. With `show_all`, dot-files are included.
 * In long mode (`do_long`) each item is a fully populated entry_info and the
 * column widths are computed; otherwise each item is an owned copy of the
 * name. Returns 0 on success, or 1 on failure (cause already reported and
 * `*out` left empty). On success the caller must release `*out` with
 * free_listing().
 */
int read_directory(const char *path, int show_all, int do_long, listing *out);

/* Sort the listing in place, by name, using locale-aware collation. */
void sort_listing(listing *list);

/* Print the listing; `reverse` walks the sorted order backwards. */
void print_listing(const listing *list, int reverse);

/* Free every item and the backing array; safe on an empty/zeroed listing. */
void free_listing(listing *list);

#endif /* LISTING_H */
