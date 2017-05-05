//
// Created by cpasjuste on 04/05/17.
//

#ifndef V2DMOD_LOG_H
#define V2DMOD_LOG_H

#include <libk/stdio.h>

#define LOG_PATH "ux0:tai/"
#define LOG_FILE LOG_PATH "v2dmod.log"

void log_write(const char *buffer);

#define V2D_LOG(...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
        log_write(buffer); \
    } while (0)

#endif //V2DMOD_LOG_H
