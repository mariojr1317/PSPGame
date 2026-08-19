#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

PSP_MODULE_INFO("ArbitroPSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

int main(void) {
    SetupCallbacks();
    pspDebugScreenInit();

    pspDebugScreenClear();
    pspDebugScreenPrintf("=====================================\n");
    pspDebugScreenPrintf("    SIMULADOR DE ARBITRO PSP v1.0    \n");
    pspDebugScreenPrintf("=====================================\n\n");
    pspDebugScreenPrintf(" ¡El juego inicio correctamente!\n\n");
    pspDebugScreenPrintf(" Presiona HOME para salir.\n");

    while (1) {
        sceDisplayWaitVblankStart();
    }

    return 0;
}
