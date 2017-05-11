//
// Created by cpasjuste on 30/04/17.
//

#ifndef V2DMOD_UTILITY_H
#define V2DMOD_UTILITY_H

#include "v2dmod_drawing.h"

#define MAX_MODULES 20
#define MAX_ITEMS 64
#define MAX_PATH 256
#define DISPLAY_WIDTH 960
#define DISPLAY_HEIGHT 544

typedef struct ItemList {
    char name[MAX_ITEMS][MAX_PATH];
    int count;
} ItemList;

void *v2d_malloc(size_t size);

void v2d_free(void *p);

void v2d_get_file_list(const char *path, ItemList *fileList);

#endif //V2DMOD_V2DMOD_UTILITY_H
