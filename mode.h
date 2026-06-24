#ifndef MODE_H
#define MODE_H

#include <sys/stat.h>

/*
 * Write the 10-character "drwxr-xr-x" style permission string for `mode`
 * into `str`, followed by a null terminator. `str` must have room for at
 * least 11 bytes.
 */
void mode_string(mode_t mode, char *str);

#endif /* MODE_H */
