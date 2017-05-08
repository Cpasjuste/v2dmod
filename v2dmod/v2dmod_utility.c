//
// Created by cpasjuste on 30/04/17.
//

#include <psp2/kernel/modulemgr.h>
#include <psp2/io/dirent.h>
#include <libk/string.h>
#include <libk/stdio.h>

#include "utils.h"
#include "v2dmod_utility.h"

void v2d_get_file_list(const char *path, FileList *fileList) {

    if(fileList == NULL) {
        return;
    }

    memset(fileList, 0, sizeof(FileList));
    fileList->count = 0;

    SceUID dfd = sceIoDopen(path);
    if (dfd < 0)
        return;

    int res = 0;

    do {

        SceIoDirent dir;
        memset(&dir, 0, sizeof(SceIoDirent));

        res = sceIoDread(dfd, &dir);
        if (res > 0) {

            if (SCE_S_ISREG(dir.d_stat.st_mode)) {
                if (fileList->count < MAX_FILES) {
                    snprintf(fileList->file[fileList->count], MAX_PATH, "%s%s", path, dir.d_name);
                    fileList->count++;
                }
            }
        }
    } while (res > 0);
}

void *v2d_malloc(size_t size) {

    void *ptr = NULL;

    SceUID m = sceKernelAllocMemBlock("v2d", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, ALIGN(size, 4 * 1024), NULL);
    if (m >= 0) {
        sceKernelGetMemBlockBase(m, &ptr);
    } else {
        m = sceKernelAllocMemBlock("v2d", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, ALIGN(size, 256 * 1024), NULL);
        if (m >= 0) {
            sceKernelGetMemBlockBase(m, &ptr);
        }
    }
    return ptr;
}

void v2d_free(void *p) {
    SceUID m = sceKernelFindMemBlockByAddr(p, 1);
    if (m >= 0) sceKernelFreeMemBlock(m);
}
