#include "entry.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <getopt.h>

int show_all = 0;
int do_long = 0;

int compare_char_ptr(const void* s1, const void* s2) {
    return strcoll(*(const char **)s1, *(const char **)s2);
}

static struct option long_options[] =
{
    {"long", no_argument, NULL, 'l'},
    {"all", no_argument, NULL, 'a'},
    {0, 0, 0, 0}
};

int main (int argc, char *argv[]) {

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int opt;
    int digit_optind = 0;
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;

    while((opt = getopt_long(argc, argv, "al", long_options, &option_index)) != -1) {
        switch (opt) {
            case '0':
            case '1':
            case '2':
                if (digit_optind != 0 && digit_optind != this_option_optind)
                    printf("digits occur in two different argv-elements.\n");
                digit_optind = this_option_optind;
                printf("option %c\n", opt);
                break;
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

    int slk_max_len = 0;
    int usr_max_len = 0;
    int grp_max_len = 0;
    int mem_max_len = 0;
    int number_of_entries = 0;
    int max_entries = 8;

    void **entries = malloc(max_entries * sizeof(*entries));
    if (entries == NULL) return 1;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        if (number_of_entries == max_entries) {
            max_entries += 4;
            void **temp = realloc(entries, max_entries * sizeof(*entries));

            if (temp == NULL) {
                fprintf(stderr, "Reallocation failed! Old memory is still intact.\n");
                free(entries);
                return 1;
            }
            entries = temp;
        }
        if (!do_long) {
            entries[number_of_entries] = entry->d_name;
            number_of_entries++;
        } else {
            if (add_entry((entry_info **)entries, path, entry->d_name, &number_of_entries) != 0) {
                // handle error
                printf("Got an error from add_entry");
            }
            get_max_length((entry_info **)entries, &number_of_entries,
                            &slk_max_len,
                            &usr_max_len,
                            &grp_max_len,
                            &mem_max_len);
        }
    }
    if (do_long) {
        sort_entries((entry_info **)entries, number_of_entries);
        for (int i = 0; i < number_of_entries; ++i) {
            print_long(entries[i], slk_max_len, usr_max_len, grp_max_len, mem_max_len);
            free(entries[i]);
        }
    } else {
        qsort(entries, number_of_entries, sizeof(entry), compare_char_ptr);
        for (int i = 0; i < number_of_entries; ++i) {
            printf("%s\n", (char *)entries[i]);
        }
    }

    free(entries);
    closedir(dir); // if this was not done the program would leak memory

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Total time elapsed: %.6f seconds\n", time_taken);

    return 0;
} // next plan of attack: get the sorting down, cache sys calls
  // for example, I will spend most of the time checking the same user over and over so
  // save which user corresponds to which uid