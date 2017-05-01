[1mdiff --git a/v2dmod/main.c b/v2dmod/main.c[m
[1mindex 93a8605..e3dad70 100644[m
[1m--- a/v2dmod/main.c[m
[1m+++ b/v2dmod/main.c[m
[36m@@ -123,61 +123,50 @@[m [mvoid onDraw() {[m
     }[m
 }[m
 [m
[32m+[m[32muint32_t last_ctrls, press_ctrls;[m
[32m+[m
 int onControls(int port, SceCtrlData *ctrl, int count) {[m
 [m
     if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_START)) {[m
[31m-        draw_menu = !draw_menu;[m
[31m-        if (draw_menu) {[m
[31m-            if (fileList != NULL) {[m
[31m-                if (fileList->files != NULL) {[m
[31m-                    v2d_free(fileList->files);[m
[31m-                }[m
[31m-                v2d_free(fileList);[m
[31m-            }[m
[31m-            fileList = v2d_get_file_list("ux0:/tai/");[m
[31m-        }[m
[31m-        sceKernelDelayThread(CTRL_DELAY);[m
[32m+[m[32m        draw_menu = true;[m
         return 1;[m
     }[m
 [m
     if (draw_menu) {[m
 [m
[31m-        if (ctrl->buttons & SCE_CTRL_DOWN) {[m
[32m+[m[32m        press_ctrls = ctrl->buttons & ~last_ctrls;[m
[32m+[m
[32m+[m[32m        if (press_ctrls & SCE_CTRL_DOWN) {[m
             selection_index++;[m
             if (selection_index >= fileList->count)[m
                 selection_index = 0;[m
[31m-            sceKernelDelayThread(CTRL_DELAY);[m
[31m-            return 1;[m
[31m-        } else if (ctrl->buttons & SCE_CTRL_UP) {[m
[32m+[m[32m        } else if (press_ctrls & SCE_CTRL_UP) {[m
             selection_index--;[m
             if (selection_index < 0)[m
                 selection_index = fileList->count - 1;[m
[31m-            sceKernelDelayThread(CTRL_DELAY);[m
[31m-            return 1;[m
[31m-        } else if (ctrl->buttons & SCE_CTRL_CROSS) {[m
[32m+[m[32m        } else if (press_ctrls & SCE_CTRL_CROSS) {[m
             V2DModule *module = get_module_by_path(fileList->files[selection_index].path);[m
             if (module == NULL) {[m
                 start_module(fileList->files[selection_index].path);[m
[31m-                sceKernelDelayThread(CTRL_DELAY);[m
             } else {[m
                 v2d_unregister(module);[m
[31m-                sceKernelDelayThread(CTRL_DELAY);[m
             }[m
[31m-            return 1;[m
         } else if (ctrl->buttons & SCE_CTRL_CIRCLE) {[m
             draw_menu = false;[m
[31m-            sceKernelDelayThread(CTRL_DELAY);[m
[31m-            return 1;[m
         }[m
[32m+[m
[32m+[m[32m        last_ctrls = ctrl->buttons;[m
[32m+[m[32m        ctrl->buttons = 0;[m
     }[m
 [m
[31m-    return 0;[m
[32m+[m[32m    return 1;[m
 }[m
 [m
 void onInit() {[m
 [m
     modules = (V2DModule *) v2d_malloc(MAX_MODULES * sizeof(V2DModule));[m
     memset(modules, 0, MAX_MODULES * sizeof(V2DModule));[m
[32m+[m[32m    fileList = v2d_get_file_list("ux0:/tai/");[m
 }[m
 [m
 void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {[m
[1mdiff --git a/v2dmod/v2dmod.c b/v2dmod/v2dmod.c[m
[1mindex 1e328e8..36f180b 100644[m
[1m--- a/v2dmod/v2dmod.c[m
[1m+++ b/v2dmod/v2dmod.c[m
[36m@@ -58,15 +58,13 @@[m [mstatic void init() {[m
 [m
 static int controls(tai_hook_ref_t ref_hook, int port, SceCtrlData *ctrl, int count) {[m
 [m
[32m+[m[32m    int ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);[m
[32m+[m
     if (ctrlCb != NULL) {[m
[31m-        if (ctrlCb(port, ctrl, count) > 0) {[m
[31m-            TAI_CONTINUE(int, ref_hook, port, ctrl, count);[m
[31m-            memset(ctrl, 0, sizeof(SceCtrlData));[m
[31m-            return -1;[m
[31m-        }[m
[32m+[m[32m        ctrlCb(port, ctrl, count);[m
     }[m
 [m
[31m-    return TAI_CONTINUE(int, ref_hook, port, ctrl, count);[m
[32m+[m[32m    return ret;[m
 }[m
 [m
 int sceCtrlPeekBufferPositive_hook_func(int port, SceCtrlData *ctrl, int count) {[m
