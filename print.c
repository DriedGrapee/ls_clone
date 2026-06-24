#include "print.h"

#include <stdio.h>
#include <string.h>

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
