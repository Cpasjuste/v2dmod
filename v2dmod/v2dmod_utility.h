//
// Created by cpasjuste on 30/04/17.
//

#ifndef V2DMOD_UTILITY_H
#define V2DMOD_UTILITY_H

#define MAX_MODULES 10
#define MAX_FILES 64
#define MAX_PATH 256
#define DISPLAY_WIDTH 960
#define DISPLAY_HEIGHT 544

typedef struct FileList {
    char file[MAX_FILES][MAX_PATH];
    int count;
} FileList;

void *v2d_malloc(size_t size);
void v2d_free(void *p);
void v2d_get_file_list(const char *path, FileList *fileList);

#endif //V2DMOD_V2DMOD_UTILITY_H
