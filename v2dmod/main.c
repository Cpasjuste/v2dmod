#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <libk/string.h>
#include <taihen.h>

#include "v2dmod.h"
#include "utils.h"
#include "v2dmod_utility.h"

#define printf V2D_LOG

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define start_module(x) sceKernelLoadStartModule(x, 0, NULL, 0, NULL, 0)
#define stop_module(x) sceKernelStopUnloadModule(x, 0, NULL, 0, NULL, NULL)

//static Color BLACK = {0, 0, 0, 255};
static Color WHITE = {255, 255, 255, 255};

static Color COLOR_FONT = {255, 255, 255, 200};
static Color COLOR_MENU = {102, 178, 255, 200};
static Color COLOR_MENU_BORDER = {255, 255, 255, 200};
static Color COLOR_SELECTION = {51, 153, 255, 200};
static Color COLOR_SELECTION_BORDER = {102, 255, 102, 200};

int v2d_start(void (*iCb)(), void (*dCb)(),
              void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
              int (*cCb)(int port, SceCtrlData *ctrl, int count));

void v2d_end();

int v2d_register(V2DModule *m);

int v2d_unregister(V2DModule *m);

/*
static bool is_module_loaded(V2DModule *m);
static int get_modules_count();
*/

static V2DModule *get_module_by_path(const char *path);

static int v2d_get_module_info_by_name(const char *name, tai_module_info_t *modinfo);

extern int gxmRenderTargetCurrent;
extern int gxmRenderTargetCount;

static V2DModule modules[MAX_MODULES];
static FileList file_list;
static bool drawing = false;
static bool draw_menu = false;
static int selection_index = 0;
static uint32_t controls[2];
//static int selection_page = 0;

void menu_load_module(int index) {

    if (draw_menu) {

        Rect rectWindow = {256, 128, SCREEN_WIDTH - 4 * 128, SCREEN_HEIGHT - 256};
        v2d_draw_rect_outline(rectWindow, COLOR_MENU, COLOR_MENU_BORDER, 3);
        v2d_scale_rect(&rectWindow, -6);

        int font_height = 25 + 2;
        int max_lines = rectWindow.h / font_height;
        int page = index / max_lines;

        rectWindow.h = font_height;

        for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

            if (i >= file_list.count)
                break;

            Rect r = rectWindow;
            r.x += 4;
            r.w -= 8;

            // set highlight
            if (i == index) {
                v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
                v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, file_list.file[i]);
            } else {
                v2d_draw_font_advanced(r, COLOR_FONT, false, true, file_list.file[i]);
            }

            rectWindow.y += font_height;
        }
    }
}

void onDraw() {

    if (drawing) {
        return;
    }

    drawing = true;
    v2d_set_draw_color(WHITE);
    //v2d_draw_font(330, 8, "gxmRenderTarget: %i (%i)", gxmRenderTargetCurrent, gxmRenderTargetCount);

    if (draw_menu) {
        menu_load_module(selection_index);
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

    drawing = false;
}

int onControls(int port, SceCtrlData *ctrl, int count) {

    if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_START)) {
        draw_menu = true;
        return 1;
    }

    if (draw_menu) {

        controls[1] = ctrl->buttons & ~controls[0];

        if (controls[1] & SCE_CTRL_DOWN) {
            selection_index++;
            if (selection_index >= file_list.count)
                selection_index = 0;
        } else if (controls[1] & SCE_CTRL_UP) {
            selection_index--;
            if (selection_index < 0)
                selection_index = file_list.count - 1;
        } else if (controls[1] & SCE_CTRL_CROSS) {
            V2DModule *module = get_module_by_path(file_list.file[selection_index]);
            if (module == NULL) {
                start_module(file_list.file[selection_index]);
            } else {
                v2d_unregister(module);
            }
        } else if (ctrl->buttons & SCE_CTRL_CIRCLE) {
            draw_menu = false;
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

void onInit() {

    //modules = (V2DModule *) sce_malloc(MAX_MODULES * sizeof(V2DModule));
    //memset(modules, 0, MAX_MODULES * sizeof(V2DModule));
    //file_list = sce_malloc(sizeof(FileList));

    v2d_get_file_list("ux0:/tai/v2dmod/modules/", &file_list);
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

/*
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
*/

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    if (!v2d_start(onInit, onDraw, onDisplaySetFrameBuf, onControls)) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_end();

    /*
    if (modules != NULL) {
        sce_free(modules);
    }
    if (file_list != NULL) {
        sce_free(file_list);
    }
    */

    return SCE_KERNEL_STOP_SUCCESS;
}

void module_exit(void) {

}
