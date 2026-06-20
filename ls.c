#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256
#endif


int show_all = 0;
int do_long = 0;

typedef struct __attribute__((packed)) {
    char          name[256];
    char          usr[LOGIN_NAME_MAX];
    char          grp[LOGIN_NAME_MAX]; 
    char          timebuf[64];
    char          modes[11];
    unsigned long link;
    long          mem;
} entry_info;

void mode_string(mode_t mode, char *str) {
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
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

void print_long(entry_info *entry,
                int link_size,
                int usr_size,
                int grp_size,
                int mem_size) {

    // later on, see if I can store these variables within the entry_info struct so that I don't have to recalculate
    int slk_size = snprintf(NULL, 0, "%lu", (unsigned long)entry->link);
    int user_size = (int)strlen(entry->usr);
    int group_size = (int)strlen(entry->grp);
    int memory_size = snprintf(NULL, 0, "%ld", (long)entry->mem);

    char link[link_size + 1];
    sprintf(link, "%lu", (unsigned long)entry->link);
    memset(link + slk_size, ' ', link_size - slk_size);
    link[link_size] = '\0';

    char usr[usr_size + 1];
    sprintf(usr, "%s", entry->usr);
    memset(usr + user_size, ' ', usr_size - user_size);
    usr[usr_size] = '\0';

    char grp[grp_size + 1];
    sprintf(grp, "%s", entry->grp);
    memset(grp + group_size, ' ', grp_size - group_size);
    grp[grp_size] = '\0';

    char mem[mem_size + 1];
    char memory_string[memory_size+1];
    sprintf(memory_string, "%ld", (long)entry->mem);
    memset(mem, ' ', mem_size);
    memcpy(mem + (mem_size - memory_size), memory_string, memory_size);
    mem[mem_size] = '\0';

    printf(
        "%s %s %s %s %s %s %s\n", 
        entry->modes, 
        link,
        usr, 
        grp, 
        mem,
        entry->timebuf,
        entry->name
    );
} // all I have to do next is to make this read them in alphabetical order (or do that outside this function)

void print_long_old(const char *dir, const char *name,
                int link_size,
                int usr_size,
                int grp_size,
                int mem_size) {
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);    //using snprintf because sprintf stores the formatted string in the given variable
                                                                // but snprintf does that with a specified maximum length and we don't want to go over
    struct stat st; // initialize a stat struct
    if (lstat(fullpath, &st) < 0) { // use lstat which fills &st with the stats at fullpath. returns 0 for success and -1 for error
        perror(name);
        return;
    }
    char modes[11]; // defining these buffers then filling them with a helper function later is pretty common
    mode_string(st.st_mode, modes);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    const char *user = pw ? pw->pw_name : "?"; // the turnary also works on null, so if pw is null then it will assign ? to user
    const char *group = gr ? gr->gr_name : "?";

    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

    char link[link_size + 1];
    int slk_size = snprintf(NULL, 0, "%lu", (unsigned long)st.st_nlink);
    sprintf(link, "%lu", (unsigned long)st.st_nlink);
    memset(link + slk_size, ' ', link_size - slk_size);
    link[link_size] = '\0';

    char usr[usr_size + 1];
    int user_size = strlen(user);
    sprintf(usr, "%s", user);
    memset(usr + user_size, ' ', usr_size - user_size);
    usr[usr_size] = '\0';

    char grp[grp_size + 1];
    int group_size = strlen(group);
    sprintf(grp, "%s", group);
    memset(grp + group_size, ' ', grp_size - group_size);
    grp[grp_size] = '\0';

    char mem[mem_size + 1];
    int memory_size = snprintf(NULL, 0, "%ld", (long)st.st_size);
    char memory_string[memory_size+1];
    sprintf(memory_string, "%ld", (long)st.st_size);
    memset(mem, ' ', mem_size);
    memcpy(mem + (mem_size - memory_size), memory_string, memory_size);
    mem[mem_size] = '\0';


    printf(
        "%s %s %s %s %s %s %s\n", 
        modes, 
        link,
        usr, 
        grp, 
        mem,
        timebuf,
        name
    );
} // goal: have every output in print long be the same number of characters

int main (int argc, char *argv[]) {

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
} // next plan of attack: get the sorting down, reduce from 2 loops to 1, cache sys calls
  // for example, I will spend most of the time checking the same user over and over so 
  // save which user corresponds to which uid  
  // also change where it checks for the -a flag, because there is no need to do stat calls
  // if the user does not want . files printed

/*
Implementation plan for caching:
Create a struct with the info from each directory
Instead of split print_long into two helper functions
read_stats and print_long, where read_stats takes an 
entry and path and returns an info struct and print_long
takes an array of info structs and prints them in alphabetical order or smth
*/  