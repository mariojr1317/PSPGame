#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include "audio.h"

PSP_MODULE_INFO("ArbitroGlobalPSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_GREEN   0xFF00FF00
#define COLOR_YELLOW  0xFF00FFFF
#define COLOR_RED     0xFF0000FF
#define COLOR_CYAN    0xFFFFFF00
#define COLOR_GRAY    0xFF888888
#define COLOR_MAGENTA 0xFFFF00FF

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
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}

struct Incidente {
    const char* equipo;
    const char* jugador;
    const char* accion;
    int respuestaCorrecta;
    const char* explicacion;
};

Incidente jugadasLMF[4] = {
    {"ALIANSA FC", "Fito Pelaya", "Reclamo airado pidiendo penalti.", 4, "Advertencia verbal del arbitro."},
    {"CD AGUILUCHO", "Serranito", "Barredida a destiempo sobre la banda.", 1, "Falta imprudente: Tarjeta Amarilla."},
    {"CD FASITO", "Coreas Jr", "Plancha directa al tobillo del rival.", 3, "Juego brusco grave: Tarjeta Roja Directa."},
    {"LA FIRPITA", "Vasquez Pro", "Cae en el area buscando engañar al juez.", 1, "Simulacion: Tarjeta Amarilla."}
};

void DibujarBorde() {
    pspDebugScreenSetTextColor(COLOR_CYAN);
    pspDebugScreenPrintf("+---------------------------------------------------+\n");
}

int main(void) {
    SetupCallbacks();
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    SceCtrlData pad;

    int estado = 0;
    int minuto = 1, reputacion = 100, fisico = 100, presion = 10;
    int aciertos = 0, errores = 0, amarillas = 0, rojas = 0;
    int jugadaIdx = 0;

    while (1) {
        pspDebugScreenClear();
        pspDebugScreenSetXY(0, 0);
        sceCtrlReadBufferPositive(&pad, 1);

        if (estado == 0) {
            DibujarBorde();
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("|     SIMULADOR ARBITRAL - GLOBAL & LMF EDITION     |\n");
            DibujarBorde();

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("\n PRESIONA [X] PARA INICIAR EL PARTIDO (LMF)\n\n");

            if (pad.Buttons & PSP_CTRL_CROSS) {
                sonarSilbato();
                minuto = 1;
                reputacion = 100;
                fisico = 100;
                aciertos = errores = amarillas = rojas = jugadaIdx = 0;
                estado = 1;
                sceKernelDelayThread(300000);
            }
        }
        else if (estado == 1) {
            pspDebugScreenSetTextColor(COLOR_GREEN);
            pspDebugScreenPrintf("[%02d'] ", minuto);
            pspDebugScreenSetTextColor(COLOR_CYAN);
            pspDebugScreenPrintf("REP: %d%% | ", reputacion);
            pspDebugScreenSetTextColor(COLOR_MAGENTA);
            pspDebugScreenPrintf("PRESION: %d%%\n", presion);

            DibujarBorde();

            Incidente j = jugadasLMF[jugadaIdx];

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("EQUIPO: ");
            pspDebugScreenSetTextColor(COLOR_GREEN);
            pspDebugScreenPrintf("%s\n", j.equipo);

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("JUGADOR: ");
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("%s\n", j.jugador);

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("JUGADA: %s\n\n", j.accion);

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf(" CONTROLES:\n");
            pspDebugScreenPrintf("  [X]        -> Tarjeta Amarilla\n");
            pspDebugScreenPrintf("  [O]        -> Siga / Ley de Ventaja\n");
            pspDebugScreenPrintf("  [CUADRADO] -> Tarjeta Roja Directa\n");

            int voto = 0;
            if (pad.Buttons & PSP_CTRL_CROSS) voto = 1;
            if (pad.Buttons & PSP_CTRL_CIRCLE) voto = 2;
            if (pad.Buttons & PSP_CTRL_SQUARE) voto = 3;

            if (voto > 0) {
                if (voto == j.respuestaCorrecta) {
                    sonarSilbato();
                    pspDebugScreenSetTextColor(COLOR_GREEN);
                    pspDebugScreenPrintf("\n [DECISION ACERTADA] %s\n", j.explicacion);
                    aciertos++;
                } else {
                    sonarError();
                    pspDebugScreenSetTextColor(COLOR_RED);
                    pspDebugScreenPrintf("\n [ERROR] %s protesto el cobro.\n", j.jugador);
                    errores++;
                    reputacion -= 15;
                }

                minuto += 22;
                jugadaIdx++;

                if (jugadaIdx >= 4 || minuto >= 90) {
                    sonarFinJuego();
                    estado = 3;
                }
                sceKernelDelayThread(1800000);
            }
        }
        else if (estado == 3) {
            DibujarBorde();
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("|            INFORME OFICIAL - FIN DE JUEGO        |\n");
            DibujarBorde();

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("\n  - Decisiones Correctas: %d/4\n", aciertos);
            pspDebugScreenPrintf("  - Reputacion Final: %d%%\n\n", reputacion);

            pspDebugScreenSetTextColor(COLOR_CYAN);
            pspDebugScreenPrintf(" Presiona [X] para volver al inicio.");

            if (pad.Buttons & PSP_CTRL_CROSS) {
                sonarMenu();
                estado = 0;
                sceKernelDelayThread(400000);
            }
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}
