#include "listing.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <locale.h>

int show_all = 0;
int do_long = 0;
int print_reverse = 0;

static struct option long_options[] =
{
    {"long", no_argument, NULL, 'l'},
    {"all", no_argument, NULL, 'a'},
    {"reverse", no_argument, NULL, 'r'},
    {0, 0, 0, 0}
};

int main (int argc, char *argv[]) {

    setlocale(LC_COLLATE, "");   // honor the user's locale for strcoll

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int opt;
    int option_index = 0;

    while((opt = getopt_long(argc, argv, "alr", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                continue;
            case 'l':
                do_long = 1;
                continue;
            case 'r':
                print_reverse = 1;
                continue;
            default:
                fprintf(stderr, "usage: %s [-alr] [path]\n", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";  // default to cwd if no path given

    listing list;
    if (read_directory(path, show_all, do_long, &list) != 0) {
        return 1;
    }

    sort_listing(&list);
    print_listing(&list, print_reverse);
    free_listing(&list);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Total time elapsed: %.6f seconds\n", time_taken);

    return 0;
} // next plan of attack: cache sys calls -- most time is spent re-checking the same
  // user/group, so cache which name corresponds to which uid/gid
