#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <libk/string.h>
#include <taihen.h>
#include <libk/stdio.h>
#include <libk/stdarg.h>

#include "v2dmod.h"
#include "v2dmod_internal.h"

#define printf V2D_LOG

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define MODULES_PATH "ux0:/tai/v2dmod/modules/"

#define start_module(x) sceKernelLoadStartModule(x, 0, NULL, 0, NULL, 0)
#define stop_module(x) sceKernelStopUnloadModule(x, 0, NULL, 0, NULL, NULL)

#define MENU_MAIN       0
#define MENU_MODULES    1
#define MENU_LOAD       2
#define MENU_SETTINGS   3
#define MENU_COUNT      4

int menu = MENU_MAIN;

V2DModule modules[MAX_MODULES];
bool draw_menu = false;
int selection_index = 0;
uint32_t controls[2];

char message[MAX_PATH];
int message_alpha = 0;

ItemList itemList;
ItemList menuList;

Rect menuRect = {256, 128, SCREEN_WIDTH - 4 * 128, SCREEN_HEIGHT - 256};

//int gxmRenderTargetCurrent;
//int gxmRenderTargetCount;

void draw_list(Rect rectWindow, ItemList *items, int selection) {

    v2d_draw_rect_outline(rectWindow, COLOR_MENU, COLOR_MENU_BORDER, 3);
    v2d_scale_rect(&rectWindow, -6);

    if (items == NULL) {
        return;
    }

    int font_height = 25 + 2;
    int max_lines = rectWindow.h / font_height;
    int page = selection / max_lines;

    rectWindow.h = font_height;

    for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

        if (i >= items->count)
            break;

        Rect r = rectWindow;
        r.x += 4;
        r.w -= 8;

        // set highlight
        if (i == selection) {
            v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
            v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, items->name[i]);
        } else {
            v2d_draw_font_advanced(r, COLOR_FONT, false, true, items->name[i]);
        }

        rectWindow.y += font_height;
    }
}

void draw_module_list(Rect rectWindow, int selection) {

    v2d_draw_rect_outline(rectWindow, COLOR_MENU, COLOR_MENU_BORDER, 3);
    v2d_scale_rect(&rectWindow, -6);

    int font_height = 25 + 2;
    int max_lines = rectWindow.h / font_height;
    int page = selection / max_lines;

    rectWindow.h = font_height;

    for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

        if (i >= MAX_MODULES)
            break;

        if (!modules[i].loaded)
            continue;

        Rect r = rectWindow;
        r.x += 4;
        r.w -= 8;

        // set highlight
        if (i == selection) {
            v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
            v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, modules[i].name);
        } else {
            v2d_draw_font_advanced(r, COLOR_FONT, false, true, modules[i].name);
        }

        rectWindow.y += font_height;
    }
}

void menu_main_draw() {

    draw_list(menuRect, &menuList, selection_index);
}

void menu_main_input(uint32_t buttons) {

    if (buttons & SCE_CTRL_DOWN) {
        selection_index++;
        if (selection_index >= menuList.count)
            selection_index = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection_index--;
        if (selection_index < 0)
            selection_index = menuList.count - 1;
    } else if (buttons & SCE_CTRL_CROSS) {
        menu = selection_index + 1;
        selection_index = 0;
        if (menu == MENU_LOAD) {
            memset(&itemList, 0, sizeof(menuList));
            v2d_get_file_list(MODULES_PATH, &itemList);
        }
    } else if (buttons & SCE_CTRL_CIRCLE) {
        selection_index = 0;
        draw_menu = false;
    }
}

void menu_modules_draw() {

    draw_module_list(menuRect, selection_index);
}

void menu_modules_input(uint32_t buttons) {

    if (buttons & SCE_CTRL_DOWN) {
        selection_index++;
        if (selection_index >= v2d_get_module_count())
            selection_index = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection_index--;
        if (selection_index < 0)
            selection_index = v2d_get_module_count() - 1;
    } else if (buttons & SCE_CTRL_CROSS) {

    } else if (buttons & SCE_CTRL_CIRCLE) {
        selection_index = 0;
        menu = MENU_MAIN;
    }
}

void menu_load_draw() {

    draw_list(menuRect, &itemList, selection_index);
}

void menu_load_input(uint32_t buttons) {

    if (buttons & SCE_CTRL_DOWN) {
        selection_index++;
        if (selection_index >= itemList.count)
            selection_index = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection_index--;
        if (selection_index < 0)
            selection_index = itemList.count - 1;
    } else if (buttons & SCE_CTRL_CROSS) {
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "%s%s", MODULES_PATH, itemList.name[selection_index]);
        V2DModule *module = v2d_get_module_by_path(path);
        if (module == NULL) {
            start_module(path);
        } else {
            v2d_draw_message("module already loaded");
        }
    } else if (buttons & SCE_CTRL_CIRCLE) {
        selection_index = 0;
        menu = MENU_MAIN;
    }
}

void draw_message_info() {

    if (strlen(message) > 0) {
        if (message_alpha < 200) {
            message_alpha += 2;
            Color bg = COLOR_MENU;
            bg.a = message_alpha;
            Color border = COLOR_MENU_BORDER;
            border.a = message_alpha;
            Color font = COLOR_FONT;
            font.a = message_alpha;
            int width = v2d_get_font_width(message) + 16;
            Rect box = {SCREEN_WIDTH / 2 - width / 2, SCREEN_HEIGHT - 64, width, 32};
            v2d_draw_rect_outline(box, bg, border, 3);
            v2d_scale_rect(&box, -6);
            v2d_draw_font_advanced(box, font, true, true, message);
        } else {
            message[0] = '\0';
            message_alpha = 0;
        }
    }
}

void onDraw() {

    v2d_set_draw_color(COLOR_WHITE);

    if (draw_menu) {
        switch (menu) {
            case MENU_MAIN:
                menu_main_draw();
                break;
            case MENU_MODULES:
                menu_modules_draw();
                break;
            case MENU_LOAD:
                menu_load_draw();
                break;
            case MENU_SETTINGS:
                menu_main_draw();
                break;
            default:
                break;
        }
    } else {
        for (int i = 0; i < MAX_MODULES; i++) {
            if (modules[i].loaded) {
                if (modules[i].drawCb != NULL) {
                    modules[i].drawCb();
                    v2d_set_draw_color(COLOR_WHITE);
                }
            }
        }
    }

    draw_message_info();
}

int onControls(int port, SceCtrlData *ctrl, int count) {

    if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_START)) {
        draw_menu = true;
        return 1;
    }

    if (draw_menu) {

        controls[1] = ctrl->buttons & ~controls[0];

        switch (menu) {
            case MENU_MAIN:
                menu_main_input(controls[1]);
                break;
            case MENU_MODULES:
                menu_modules_input(controls[1]);
                break;
            case MENU_LOAD:
                menu_load_input(controls[1]);
                break;
            case MENU_SETTINGS:
                menu_main_input(controls[1]);
                break;
            default:
                break;
        }

        /*
        // DEBUG
        if ((ctrl->buttons & SCE_CTRL_RTRIGGER) && press_ctrls & SCE_CTRL_DOWN) {
            gxmRenderTargetCurrent++;
            if (gxmRenderTargetCurrent >= gxmRenderTargetCount)
                gxmRenderTargetCurrent = 0;
        } else if ((ctrl->buttons & SCE_CTRL_RTRIGGER) && press_ctrls & SCE_CTRL_UP) {
            gxmRenderTargetCurrent--;
            if (gxmRenderTargetCurrent < 0)
                gxmRenderTargetCurrent = gxmRenderTargetCount - 1;
        }
        // DEBUG
        */

        controls[0] = ctrl->buttons;
        return 1;
    }

    return 0;
}

void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].loaded) {
            if (modules[i].setFbCb != NULL) {
                modules[i].setFbCb(pParam, sync);
            }
        }
    }
}

void onInit() {

    memset(&menuList, 0, sizeof(menuList));
    strcpy(menuList.name[0], "Modules");
    strcpy(menuList.name[1], "Load");
    strcpy(menuList.name[2], "Settings");
    menuList.count = MENU_COUNT - 1;

    for (int i = 0; i < MAX_MODULES; i++) {
        memset(&modules[i], 0, sizeof(V2DModule));
        modules[i].loaded = false;
    }

    v2d_draw_message("v2dmod loaded");

}

void v2d_draw_message(const char *fmt, ...) {

    memset(message, 0, MAX_PATH);
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, MAX_PATH, fmt, args);
    va_end(args);
}

int v2d_register(V2DModule *m) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (!modules[i].loaded) {
            strncpy(modules[i].name, m->name, 27);
            strncpy(modules[i].desc, m->desc, 64);
            modules[i].initCb = m->initCb;
            modules[i].drawCb = m->drawCb;
            modules[i].setFbCb = m->setFbCb;
            modules[i].loaded = true;
            tai_module_info_t mi;
            mi.size = sizeof(tai_module_info_t);
            if (taiGetModuleInfo(m->name, &mi) == 0) {
                modules[i].modid = mi.modid;
                SceKernelModuleInfo info;
                info.size = (sizeof(SceKernelModuleInfo));
                int res = sceKernelGetModuleInfo(modules[i].modid, &info);
                if (res >= 0) {
                    strncpy(modules[i].path, info.path, 256);
                }
            }
            draw_message_info("module loaded: %s", modules[i].name);
            return 1;
        }
    }

    return -1;
}

int v2d_unregister(V2DModule *m) {

    if (m != NULL) {
        stop_module(m->modid);
        memset(m, 0, sizeof(V2DModule));
        m->loaded = false;
        return 1;
    }

    return -1;
}

int v2d_get_module_count() {

    int count = 0;

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].loaded) {
            count++;
        }
    }

    return count;
}

V2DModule *v2d_get_module_by_path(const char *path) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].loaded && strcmp(modules[i].path, path) == 0) {
            return &modules[i];
        }
    }

    return NULL;
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    if (!v2d_start(onInit, onDraw, onDisplaySetFrameBuf, onControls)) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_end();

    return SCE_KERNEL_STOP_SUCCESS;
}

void module_exit(void) {

}
