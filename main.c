#include "entry.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>

int show_all = 0;
int do_long = 0;

int main (int argc, char *argv[]) {

    setlocale(LC_COLLATE, "");   // honor the user's locale for strcoll, like real ls

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int opt;

    while((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                continue;
            case 'l':
                do_long = 1;
                continue;
            default:
                fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";  // ternary operator so as to
                                                              //not assign null to path if no input is given

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;

    if (do_long) {
        int slk_max_len = 0;
        int usr_max_len = 0;
        int grp_max_len = 0;
        int mem_max_len = 0;
        int number_of_entries = 0;
        int max_entries = 8;
        entry_info **entries = malloc(max_entries * sizeof(*entries));

        if (entries == NULL) return 1;
        while ((entry = readdir(dir)) != NULL) {
            if (!show_all && entry->d_name[0] == '.') {
                continue;
            }

            if (number_of_entries == max_entries) {
                max_entries += 4;
                entry_info **temp = realloc(entries, max_entries * sizeof(*entries));

                if (temp == NULL) {
                    fprintf(stderr, "Reallocation failed! Old memory is still intact.\n");
                    free(entries);
                    return 1;
                }
                entries = temp;
            }

            if (add_entry(entries, path, entry->d_name, &number_of_entries) != 0) {
                // handle error
                printf("Got an error from add_entry");
            }
            get_max_length(entries, &number_of_entries,
                            &slk_max_len,
                            &usr_max_len,
                            &grp_max_len,
                            &mem_max_len);

        }
        // sort entries alphabetically by name (locale-aware, via the entry module)
        sort_entries(entries, number_of_entries);   // int -> size_t is a safe widening

        for (int i = 0; i < number_of_entries; ++i) {
            print_long(entries[i], slk_max_len, usr_max_len, grp_max_len, mem_max_len);
            free(entries[i]);
        }
        free(entries);
    } else {
        while ((entry = readdir(dir)) != NULL) {
            if (!show_all && entry->d_name[0] == '.') {
                continue;
            }
            printf("%s\n", entry->d_name); // the -> replaces (*entry).d_name
        }
    }

    closedir(dir); // if this was not done the program would leak memory

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Total time elapsed: %.6f seconds\n", time_taken);

    return 0;
} // next plan of attack: get the sorting down, cache sys calls
  // for example, I will spend most of the time checking the same user over and over so
  // save which user corresponds to which uid