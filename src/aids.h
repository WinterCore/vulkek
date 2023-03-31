#ifndef AIDS
#define AIDS
#include <stdio.h>

typedef struct FileData {
    const char *data;
    const size_t length;
} FileData;

FileData read_file(const char *path);
#endif
