//
// Created by cpasjuste on 12/05/17.
//

#include <psp2/types.h>
#include <libk/string.h>
#include <psp2/gxm.h>
#include "kuio.h"

/*
static void *kpool = NULL;
static unsigned int kpool_index = 0;

void kpool_reset() {

    kpool = kpool_get();

    if (kpool != NULL) {
        kpool_index = 0;
        sceGxmMapMemory(kpool, KPOOL_SIZE, SCE_GXM_MEMORY_ATTRIB_READ);
    }
}

size_t kpool_available() {

    if (kpool == NULL) {
        return 0;
    }

    return KPOOL_SIZE - kpool_index;
}

void *kpool_alloc(size_t size) {

    if (kpool == NULL) {
        return NULL;
    }

    if ((kpool_index + size) < KPOOL_SIZE) {
        void *addr = (void *) ((unsigned int) kpool + kpool_index);
        kpool_index += size;
        return addr;
    }

    return NULL;
}

void *kpool_memalign(unsigned int size, unsigned int alignment) {

    if (kpool == NULL) {
        return NULL;
    }

    unsigned int new_index = (kpool_index + alignment - 1) & ~(alignment - 1);
    if ((new_index + size) < KPOOL_SIZE) {
        void *addr = (void *) ((unsigned int) kpool + new_index);
        kpool_index = new_index + size;
        return addr;
    }

    return NULL;
}
*/