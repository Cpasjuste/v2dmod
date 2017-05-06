#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/io/dirent.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <psp2/kernel/threadmgr.h>
#include <taihen.h>

#include "v2dmod.h"
#include "utils.h"
#include "v2dmod_utility.h"
#include "v2dmod_log.h"

char debug_msg[512];

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define CTRL_DELAY (1000 * 150)

#define start_module(x) sceKernelLoadStartModule(x, 0, NULL, 0, NULL, 0)
#define stop_module(x) sceKernelStopUnloadModule(x, 0, NULL, 0, NULL, NULL)

static Color BLACK = {0, 0, 0, 255};
static Color WHITE = {255, 255, 255, 255};

static Color COLOR_FONT = {255, 255, 255, 200};
static Color COLOR_MENU = {102, 178, 255, 200};
static Color COLOR_MENU_BORDER = {255, 255, 255, 200};
static Color COLOR_SELECTION = {51, 153, 255, 200};
static Color COLOR_SELECTION_BORDER = {102, 255, 102, 200};

void v2d_start(void (*iCb)(), void (*dCb)(),
               void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
               int (*cCb)(int port, SceCtrlData *ctrl, int count));

void v2d_end();

int v2d_register(V2DModule *m);

int v2d_unregister(V2DModule *m);

static bool is_module_loaded(V2DModule *m);

static int get_modules_count();

static V2DModule *get_module_by_path(const char *path);

static int v2d_get_module_info_by_name(const char *name, tai_module_info_t *modinfo);

V2DModule *modules = NULL;

FileList *fileList = NULL;

bool draw_menu = false;

int selection_index = 0;
int selection_page = 0;

void menu_load_module() {

    if (draw_menu) {

        Rect rectWindow = {256, 128, SCREEN_WIDTH - 4 * 128, SCREEN_HEIGHT - 256};
        v2d_draw_rect_outline(rectWindow, COLOR_MENU, COLOR_MENU_BORDER, 3);

        int font_height = 25;
        int max_lines = rectWindow.h / font_height;
        selection_page = selection_index / max_lines;

        Rect rectText = rectWindow;
        rectText.x += 1;
        rectText.w -= 2;
        rectText.h = font_height;

        for (int i = selection_page * max_lines; i < selection_page * max_lines + max_lines; i++) {

            if (i >= fileList->count)
                break;

            Rect r = rectText;
            r.x += 6;
            r.w -= 6;

            // set highlight
            if (i == selection_index) {
                v2d_draw_rect_outline(rectText, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
                v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, fileList->files[i].path);
            } else {
                v2d_draw_font_advanced(r, COLOR_FONT, false, true, fileList->files[i].path);
            }

            rectText.y += font_height;
        }
    }
}

void onDraw() {

    v2d_set_draw_color(WHITE);


    //v2d_draw_font(100, 0, "modules: %i", get_modules_count());

    /*
    v2d_draw_font(100, 0, "modules: %i", get_modules_count());
    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].modinfo.modid > 0) {
            v2d_draw_font(100, (i + 1) * 20, "module: %s (0x%08X) %s",
                          modules[i].modinfo.name, modules[i].modinfo.modid, modules[i].path);
        }
    }
    v2d_draw_font(400, 520, "%s", debug_msg);
    */

    if (draw_menu) {
        menu_load_module();
    } else {
        for (int i = 0; i < MAX_MODULES; i++) {
            if (modules[i].modinfo.modid > 0) {
                if (modules[i].drawCb != NULL) {
                    modules[i].drawCb();
                    v2d_set_draw_color(WHITE);
                }
            }
        }
    }
}

uint32_t last_ctrls, press_ctrls;

int onControls(int port, SceCtrlData *ctrl, int count) {

    if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_START)) {
        draw_menu = true;
        return 1;
    }

    if (draw_menu) {

        press_ctrls = ctrl->buttons & ~last_ctrls;

        if (press_ctrls & SCE_CTRL_DOWN) {
            selection_index++;
            if (selection_index >= fileList->count)
                selection_index = 0;
        } else if (press_ctrls & SCE_CTRL_UP) {
            selection_index--;
            if (selection_index < 0)
                selection_index = fileList->count - 1;
        } else if (press_ctrls & SCE_CTRL_CROSS) {
            V2DModule *module = get_module_by_path(fileList->files[selection_index].path);
            if (module == NULL) {
                start_module(fileList->files[selection_index].path);
            } else {
                v2d_unregister(module);
            }
        } else if (ctrl->buttons & SCE_CTRL_CIRCLE) {
            draw_menu = false;
        }

        last_ctrls = ctrl->buttons;
        ctrl->buttons = 0;
    }

    return 1;
}

void onInit() {

    modules = (V2DModule *) v2d_malloc(MAX_MODULES * sizeof(V2DModule));
    memset(modules, 0, MAX_MODULES * sizeof(V2DModule));
    fileList = v2d_get_file_list("ux0:/tai/");
}

void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].modinfo.modid > 0) {
            if (modules[i].setFbCb != NULL) {
                modules[i].setFbCb(pParam, sync);
            }
        }
    }
}

int v2d_register(V2DModule *m) {

    if (m == NULL) {
        return -1;
    }

    v2d_get_module_info_by_name(m->name, &m->modinfo);

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].modinfo.modid <= 0) {
            modules[i] = *m;
            SceKernelModuleInfo info;
            info.size = (sizeof(SceKernelModuleInfo));
            int res = sceKernelGetModuleInfo(modules[i].modinfo.modid, &info);
            if (res >= 0) {
                strncpy(modules[i].path, info.path, 256);
            }

            return 1;
        }
    }

    return -1;
}

int v2d_unregister(V2DModule *m) {

    if (m != NULL) {
        stop_module(m->modinfo.modid);
        memset(m, 0, sizeof(V2DModule));
        return 1;
    }

    return -1;
}

static V2DModule *get_module_by_path(const char *path) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (strcmp(path, modules[i].path) == 0) {
            return &modules[i];
        }
    }

    return NULL;
}

static int v2d_get_module_info_by_name(const char *name, tai_module_info_t *modinfo) {
    if (modinfo != NULL) {
        memset(modinfo, 0, sizeof(tai_module_info_t));
        modinfo->size = sizeof(tai_module_info_t);
        return taiGetModuleInfo(name, modinfo);
    }
    return -1;
}

static bool is_module_loaded(V2DModule *m) {

    tai_module_info_t info;
    info.size = sizeof(tai_module_info_t);
    return taiGetModuleInfo(m->modinfo.name, &info) >= 0;
}

static int get_modules_count() {

    int count = 0;

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].modinfo.modid > 0) {
            count++;
        }
    }

    return count;
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    v2d_start(onInit, onDraw, onDisplaySetFrameBuf, onControls);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_end();

    v2d_free(modules);
    if (fileList != NULL) {
        v2d_free(fileList);
    }

    return SCE_KERNEL_STOP_SUCCESS;
}

void module_exit(void) {

}
