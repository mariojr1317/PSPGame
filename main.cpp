#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

// Macros obligatorios para la arquitectura de PSP
PSP_MODULE_INFO("ArbitroPSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

// Callback para permitir salir al menú principal de la consola
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

int main() {
    // Inicializar la pantalla de depuración en texto
    pspDebugScreenInit();
    SetupCallbacks();

    // Bucle principal
    while (1) {
        pspDebugScreenSetXY(0, 0);
        printf("=====================================\n");
        printf("    SIMULADOR DE ARBITRO PSP v1.0    \n");
        printf("=====================================\n\n");
        printf(" ¡El juego inicio correctamente!\n\n");
        printf(" Presiona HOME o la tecla asignada\n");
        printf(" en PPSSPP para salir.\n");

        // Pausa necesaria para evitar pantallazo negro y sincronizar fps
        sceDisplayWaitVblankStart();
    }

    return 0;
}
