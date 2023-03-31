#ifndef AIDS
#define AIDS
#include <stdio.h>

typedef struct Kyle {
    const char *data;
    const size_t length;
} Kyle;

Kyle kyle_from_file(const char *path);
#endif
