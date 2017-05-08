//
// Created by cpasjuste on 04/05/17.
//

#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <libk/string.h>
#include <kuio.h>

#include "v2dmod_log.h"

void log_write(const char *buffer) {

    kuIoMkdir(LOG_PATH);

    SceUID fd;
    kuIoOpen(LOG_FILE, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, &fd);

    if (fd < 0)
        return;

    kuIoWrite(fd, buffer, strlen(buffer));
    kuIoClose(fd);

}
