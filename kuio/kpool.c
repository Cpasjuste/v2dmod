//
// Created by cpasjuste on 12/05/17.
//

/*
#include <vitasdkkern.h>
#include <libk/string.h>
#include "include/kuio.h"

#define ALIGN(x, a)    (((x) + ((a) - 1)) & ~((a) - 1))

static void *kpool = NULL;
static SceUID kpool_uid = -1;

void kpool_create() {

    kpool_uid = ksceKernelAllocMemBlock("v2d-kpool",
                                        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
                                        KPOOL_SIZE,
                                        0);
    if (kpool_uid >= 0) {
        ksceKernelGetMemBlockBase(kpool_uid, &kpool);
    }
}

void kpool_destroy() {

    if (kpool != NULL && kpool_uid >= 0) {
        ksceKernelFreeMemBlock(kpool_uid);
    }
}

void *kpool_get() {
    return kpool;
}
*/