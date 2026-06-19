#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>


int show_all = 0;
int do_print_long = 0;

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

void get_max_length(const char *dir, const char *name,
                    int *slk_max_len,
                    int *usr_max_len, 
                    int *grp_max_len,
                    int *mem_max_len) {
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    struct stat st;
    if (lstat(fullpath, &st) < 0) {
        perror(name);
        return;
    }

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    const char *user = pw ? pw->pw_name : "?"; // the turnary also works on null, so if pw is null then it will assign ? to user
    const char *group = gr ? gr->gr_name : "?";

    int slk_size = snprintf(NULL, 0, "%lu", (unsigned long)st.st_nlink);
    int mem_size = snprintf(NULL, 0, "%ld", (long)st.st_size);

    if (slk_size > *slk_max_len) {
        *slk_max_len = slk_size;
    }
    if ((int)strlen(user) > *usr_max_len) {
        *usr_max_len = strlen(user);
    }
    if ((int)strlen(group) > *grp_max_len) {
        *grp_max_len = strlen(group);
    }
    if (mem_size > *mem_max_len) {
        *mem_max_len = mem_size;
    }
}

void print_long(const char *dir, const char *name,
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
  // do this by adding whitespace to the ends of users and groups so that all users and groups are the same length
  // then add whitespace to the front of the size value

int main (int argc, char *argv[]) {
    int opt;
    int slk_max_len = 0;
    int usr_max_len = 0;
    int grp_max_len = 0;
    int mem_max_len = 0;

    while((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                continue;
            case 'l':
                do_print_long = 1;
                continue;
            default:
                fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";  // ternary operator so as to 
                                                    //not assign null to path if no input is given

    // error handling on opening input directory
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        get_max_length(path, entry->d_name,
                       &slk_max_len,
                       &usr_max_len,
                       &grp_max_len,
                       &mem_max_len);
    }

    
    closedir(dir); 
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        } 
        if (do_print_long) {
            print_long(path, entry->d_name, slk_max_len, usr_max_len, grp_max_len, mem_max_len);
        } else {
            printf("%s\n", entry->d_name); // the -> replaces (*entry).d_name 
        }
    }

    closedir(dir); // if this was not done the program would leak memory

    return 0;
}