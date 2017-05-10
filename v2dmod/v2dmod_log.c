//
// Created by cpasjuste on 04/05/17.
//

#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <libk/string.h>

#ifdef ENABLE_LOGGING

#include <kuio.h>

#endif

#include "v2dmod_log.h"

SceUID fd;

void log_open() {
#ifdef ENABLE_LOGGING
    kuIoMkdir(LOG_PATH);
    kuIoOpen(LOG_FILE, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, &fd);
#endif
}

void log_write(const char *buffer) {
#ifdef ENABLE_LOGGING
    if (fd >= 0) {
        kuIoWrite(fd, buffer, strlen(buffer));
    }
#endif
}

void log_close() {
#ifdef ENABLE_LOGGING
    if (fd >= 0) {
        kuIoClose(fd);
    }
#endif
}
