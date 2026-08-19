#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

// Configuración del módulo para PSP
PSP_MODULE_INFO("Modo Carrera Arbitro", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Usar pspDebugScreenPrintf para imprimir texto fácilmente
#define printf pspDebugScreenPrintf

// Funciones obligatorias para poder salir del juego pulsando HOME
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
    if(thid >= 0) { sceKernelStartThread(thid, 0, 0); }
    return thid;
}

int main(void) {
    SetupCallbacks();
    pspDebugScreenInit(); // Inicia la pantalla de texto de PSP

    SceCtrlData pad; // Variable para leer los botones

    printf("\n");
    printf(" =====================================\n");
    printf("     MODO CARRERA ARBITRO LEYENDA\n");
    printf(" =====================================\n\n");
    printf(" Pulsa X para pitar el inicio del partido\n");
    printf(" Pulsa O (Circulo) para salir\n");

    while(1) {
        sceCtrlReadBufferPositive(&pad, 1); // Lee los controles

        if (pad.Buttons & PSP_CTRL_CROSS) {
            pspDebugScreenClear();
            printf("\n\n PIIIIII!\n\n");
            printf(" Has pitado el inicio del partido.\n");
            printf(" Min 15: Falta fuerte en el mediocampo.\n\n");
            printf(" Pulsa CUADRADO para sacar Tarjeta Amarilla.\n");
            sceKernelDelayThread(500000); // Pequeña pausa para no repetir rápido
        }

        if (pad.Buttons & PSP_CTRL_SQUARE) {
            pspDebugScreenClear();
            printf("\n\n Has sacado TARJETA AMARILLA.\n");
            printf(" El comite aprueba tu decision.\n");
            sceKernelDelayThread(500000);
        }

        if (pad.Buttons & PSP_CTRL_CIRCLE) {
            break; // Sale del bucle y cierra el juego
        }
    }

    sceKernelExitGame();
    return 0;
}
