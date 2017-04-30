//
// Created by cpasjuste on 30/04/17.
//

#ifndef V2DMOD_UTILITY_H
#define V2DMOD_UTILITY_H

#define MAX_MODULES 10
#define MAX_FILES 64
#define MAX_PATH 512
#define DISPLAY_WIDTH 960
#define DISPLAY_HEIGHT 544

typedef struct File {
    char path[MAX_PATH];
} File;

typedef struct FileList {
    File *files;
    int count;
} FileList;

void *v2d_malloc(size_t size);
void v2d_free(void *p);
FileList *v2d_get_file_list(const char *path);

#endif //V2DMOD_V2DMOD_UTILITY_H
