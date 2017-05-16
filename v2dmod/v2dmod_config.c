//
// Created by cpasjuste on 15/05/17.
//

#include <psp2/io/fcntl.h>
#include <kuio.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include "v2dmod.h"
#include "v2dmod_config.h"

int v2d_cfg_read(V2DModule *module) {

    if (module == NULL) {
        return -1;
    }

    SceUID fd;
    char *token;
    char cfg_path[MAX_PATH];
    char buffer[64 * 20]; // 20 options max

    snprintf(cfg_path, MAX_PATH, "%s%s.cfg", CFG_PATH, module->name);
    kuIoOpen(cfg_path, SCE_O_RDONLY, &fd);
    if (fd < 0) {
        return -1;
    }

    kuIoRead(fd, buffer, 64 * 20);
    kuIoClose(fd);

    for (int i = 0; i < MAX_OPTIONS; i++) {

        const char *name = module->options[i].name;
        size_t len = strlen(name);

        if (len <= 0) {
            break;
        }

        token = strstr(buffer, name);
        if (token) {
            char c = token[len + 2];
            module->options[i].index = c - 48;
        }
    }

    return 0;
}

int v2d_cfg_write(V2DModule *module) {

    if (module == NULL) {
        return -1;
    }

    char cfg_path[MAX_PATH];
    snprintf(cfg_path, MAX_PATH, "%s%s.cfg", CFG_PATH, module->name);
    kuIoMkdir(CFG_PATH);

    SceUID fd;
    kuIoOpen(cfg_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, &fd);
    if (fd < 0) {
        return -1;
    }

    char tmp[64];
    for (int i = 0; i < MAX_OPTIONS; i++) {
        if (strlen(module->options[i].name) <= 0) {
            break;
        }

        snprintf(tmp, 64, "\"%s\" %i\n",
                 module->options[i].name, module->options[i].index);
        kuIoWrite(fd, tmp, strlen(tmp));
    }

    kuIoClose(fd);

    return 0;
}
