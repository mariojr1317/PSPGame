#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h> // Librería para los botones de la PSP

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

    // Configurar la lectura de los botones
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    SceCtrlData pad;
    int estado_juego = 0; // 0 = Menú de inicio, 1 = Partido en curso

    while (1) {
        pspDebugScreenClear();
        pspDebugScreenSetXY(0, 0);

        // Leer qué botón se presiona
        sceCtrlReadBufferPositive(&pad, 1);

        if (estado_juego == 0) {
            // Pantalla de Inicio
            pspDebugScreenPrintf("=====================================\n");
            pspDebugScreenPrintf("    SIMULADOR DE ARBITRO PSP v1.0    \n");
            pspDebugScreenPrintf("=====================================\n\n");
            pspDebugScreenPrintf(" [!] Presiona [X] o [O] para INICIAR\n");

            // Si presiona X o Círculo, entra al juego
            if ((pad.Buttons & PSP_CTRL_CROSS) || (pad.Buttons & PSP_CTRL_CIRCLE)) {
                estado_juego = 1;
                sceKernelDelayThread(300000); // Pequeña pausa de 300ms para evitar toques dobles
            }
        } 
        else if (estado_juego == 1) {
            // Pantalla del Partido
            pspDebugScreenPrintf("=====================================\n");
            pspDebugScreenPrintf("      PARTIDO EN CURSO - MINUTO 15   \n");
            pspDebugScreenPrintf("=====================================\n\n");
            pspDebugScreenPrintf(" JUGADA: Entrada fuerte cerca del area.\n\n");
            pspDebugScreenPrintf("  [X] -> Cobrar Falta y Sacar Tarjeta\n");
            pspDebugScreenPrintf("  [O] -> Dar Ley de la Ventaja (Siga)\n\n");

            if (pad.Buttons & PSP_CTRL_CROSS) {
                pspDebugScreenPrintf(" -> decision: ¡Pitas falta! Tarjeta Amarilla al #4.\n");
                sceKernelDelayThread(500000);
            } 
            else if (pad.Buttons & PSP_CTRL_CIRCLE) {
                pspDebugScreenPrintf(" -> decision: ¡Mueves los brazos! Ley de ventaja aplicada.\n");
                sceKernelDelayThread(500000);
            }
        }

        // Sincronizar los fotogramas
        sceDisplayWaitVblankStart();
    }

    return 0;
}
