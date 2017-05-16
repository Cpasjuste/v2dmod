#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <libk/string.h>
#include <libk/stdio.h>

#include "v2dmod.h"
#include "v2dmod_internal.h"
#include "v2dmod_config.h"
#include "menu/menu_main.h"
#include "menu/menu_drawing_helper.h"
#include "menu/menu_module_option.h"
#include "menu/menu_module_list.h"

#define printf V2D_LOG

#define stop_module(x) sceKernelStopUnloadModule(x, 0, NULL, 0, NULL, NULL)

extern V2DModule *modules;
extern int modules_count;

int draw_module_option_index = -1;
ItemList menuList;

int menu = MENU_MAIN;
bool draw_menu = false;
int selection_index = 0;

uint32_t controls[2];

char message[MAX_PATH];
int message_alpha = 0;

void load_start_module(char *path);
//int gxmRenderTargetCurrent;
//int gxmRenderTargetCount;

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
            draw_border(box);
            v2d_scale_rect(&box, -1);
            v2d_draw_rect_outline(box, bg, border, 2);
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

    //v2d_draw_font(400, 16, "kpool: %i", kpool_available());

    if (draw_menu) {
        switch (menu) {
            case MENU_MAIN:
                menu_main_draw(v2d_game_name, &menuList, selection_index);
                break;
            case MENU_MODULES:
                menu_modules_draw(v2d_game_name, modules, selection_index);
                break;
            case MENU_MODULE_OPTION:
                menu_module_option_draw(
                        &modules[draw_module_option_index], selection_index);
                break;
            case MENU_SETTINGS:
                break;
            default:
                break;
        }
    } else {
        for (int i = 0; i < modules_count; i++) {
            if (modules[i].modid > 0) {
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

    if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_LTRIGGER)) {
        draw_menu = true;
        return 1;
    }

    if (draw_menu) {

        controls[1] = ctrl->buttons & ~controls[0];

        switch (menu) {
            case MENU_MAIN:
                selection_index = menu_main_input(
                        controls[1], &menuList, selection_index);
                break;
            case MENU_MODULES:
                selection_index = menu_modules_input(
                        controls[1], modules, selection_index);
                break;
            case MENU_MODULE_OPTION:
                selection_index = menu_module_option_input(
                        controls[1], &modules[draw_module_option_index], selection_index);
                break;
            case MENU_SETTINGS:
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

    for (int i = 0; i < modules_count; i++) {
        if (modules[i].modid > 0) {
            if (modules[i].setFbCb != NULL) {
                modules[i].setFbCb(pParam, sync);
            }
        }
    }
}

void onInit() {

    //menuList = kpool_alloc(sizeof(ItemList));
    memset(&menuList, 0, sizeof(ItemList));
    strcpy(menuList.name[0], "Modules");
    strcpy(menuList.name[1], "Settings");
    menuList.count = MENU_COUNT - 1;

    // build module list from path
    modules = v2d_malloc(sizeof(V2DModule) * MAX_MODULES);
    //kpool_alloc(sizeof(V2DModule) * MAX_MODULES);
    memset(modules, 0, sizeof(V2DModule) * MAX_MODULES);

    ItemList files;
    v2d_get_file_list(MODULES_PATH, &files);
    for (int i = 0; i < files.count; i++) {
        char *ext = strrchr(files.name[i], '.');
        if (ext && strncmp(ext, "suprx", 5)) {
            strncpy(modules[modules_count].name, files.name[i], strlen(files.name[i]) - 6);
            snprintf(modules[modules_count].path, MAX_PATH, "%s%s", MODULES_PATH, files.name[i]);
            strcpy(modules[modules_count].desc, "No description available");
            modules_count++;
        }
    }

    v2d_draw_message("< v2dmod >");
}

void load_start_module(char *path) {

    sceKernelLoadStartModule(path, 0, NULL, 0, NULL, 0);

    V2DModule *module = v2d_get_module_by_path(path);
    if (module) {
        v2d_cfg_read(module);
        v2d_draw_message("%s loaded", module->name);
    }
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    /*
    // reset/update "kernel" pool
    kpool_reset();
    if (kpool_available() <= 0) {
        return SCE_KERNEL_START_FAILED;
    }
    */

    if (!v2d_start(onInit, onDraw, onDisplaySetFrameBuf, onControls)) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_end();

    if (modules) {
        v2d_free(modules);
    }

    return SCE_KERNEL_STOP_SUCCESS;
}

void module_exit(void) {

}
