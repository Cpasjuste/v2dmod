//
// Created by cpasjuste on 04/05/17.
//

#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <libk/string.h>

#include "v2dmod_log.h"

void log_write(const char *buffer) {

    sceIoMkdir(LOG_PATH, 6);

    SceUID fd = sceIoOpen(LOG_FILE,
                          SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
    if (fd < 0)
        return;

    sceIoWrite(fd, buffer, strlen(buffer));
    sceIoClose(fd);
}
