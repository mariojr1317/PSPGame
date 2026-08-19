#include <pspaudio.h>
#include <math.h>

// Función para sintetizar pitidos estilo retro de 8-bit/16-bit sin archivos externos
void emitirPitido(int frecuenciaHz, int duracionMs) {
    int canal = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, 512, PSP_AUDIO_FORMAT_STEREO);
    if (canal < 0) return;

    int sampleRate = 44100;
    int totalMuestras = (sampleRate * duracionMs) / 1000;
    short buffer[512 * 2]; // Buffer Stereo PCM 16-bit

    double fase = 0.0;
    double incremento = 2.0 * 3.14159265358979323846 * frecuenciaHz / sampleRate;

    int enviadas = 0;
    while (enviadas < totalMuestras) {
        int tamanoBloque = (totalMuestras - enviadas > 512) ? 512 : (totalMuestras - enviadas);
        for (int i = 0; i < tamanoBloque; i++) {
            // Genera onda cuadrada estilo sintetizador 8-bit
            short valor = (sin(fase) > 0) ? 10000 : -10000; 
            buffer[i * 2]     = valor; // Canal Izquierdo
            buffer[i * 2 + 1] = valor; // Canal Derecho
            fase += incremento;
        }
        sceAudioOutputPannedBlocking(canal, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, buffer);
        enviadas += tamanoBloque;
    }

    sceAudioChRelease(canal);
}
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>

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
    int respuestaCorrecta; // 1: Amarilla, 2: Siga, 3: Roja, 4: Advertencia, 5: VAR
    const char* explicacion;
};

// BANCO DE JUGADAS POR TORNEO
Incidente jugadasLMF[4] = {
    {"ALIANSA FC", "Fito Pelaya", "Reclamo airado pidiendo penalti en el area rival.", 4, "Advertencia verbal del arbitro principal."},
    {"CD AGUILUCHO", "Serranito", "Barredida a destiempo sobre la banda izquierda.", 1, "Falta imprudente: Tarjeta Amarilla."},
    {"CD FASITO", "Coreas Jr", "Plancha directa al tobillo del volante central.", 3, "Juego brusco grave: Tarjeta Roja Directa."},
    {"LA FIRPITA", "Vasquez Pro", "Cae en el area buscando engañar al juez.", 1, "Simulacion: Tarjeta Amarilla."}
};

Incidente jugadasUCL[4] = {
    {"REAL MADRIZ", "Vini Chiquito", "Protesta efusiva tras no cobrarse un tiro de esquina.", 1, "Conducta antideportiva: Tarjeta Amarilla."},
    {"FC BARSA", "Lamine Tamal", "Desborde limpio con choque hombro a hombro.", 2, "Contacto permitido en disputa. ¡Siga!"},
    {"MANCHEGO CITY", "Erling Jaland", "Codazo involuntario en salto aereo por el balon.", 2, "Falta comun sin amonestacion."},
    {"BAYERN MUNCHEN", "Harry Keops", "Remate potente que roza el travesaño e impacta la linea.", 5, "Decision milimetrica: Revision en el VAR."}
};

Incidente jugadasMundial[4] = {
    {"LA SELECTA", "Magico Gonzalez Jr", "Pase de rabona interceptado con la mano voluntaria.", 1, "Mano no natural: Tarjeta Amarilla."},
    {"ARGENTINIA", "Lio Menci", "Tiro libre directo que pega en la barrera adelantada.", 4, "Advertencia y repeticion del cobro."},
    {"BRAZUCA FC", "Neimar Jr", "Simulacion de falta rodando sobre el cesped.", 1, "Engaño al arbitro: Tarjeta Amarilla."},
    {"ESPAÑITA", "Rodri Hernandez", "Entrada violenta cortando avance prometedor.", 3, "Falta grave: Tarjeta Roja Directa."}
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

    int estado = 0; // 0: Menu Ligas, 1: Partido, 2: VAR, 3: Final
    int torneoSeleccionado = 0; // 0: LMF El Salvador, 1: UCL, 2: Mundial
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
            pspDebugScreenPrintf("\n SELECCIONA EL TORNEO A PITAR:\n\n");

            pspDebugScreenSetTextColor(torneoSeleccionado == 0 ? COLOR_GREEN : COLOR_GRAY);
            pspDebugScreenPrintf("  [X] -> PRIMERA DIVISION EL SALVADOR (LMF)\n");

            pspDebugScreenSetTextColor(torneoSeleccionado == 1 ? COLOR_GREEN : COLOR_GRAY);
            pspDebugScreenPrintf("  [O] -> UEFA CHAMPIONS LEAGUE (UCL PRO)\n");

            pspDebugScreenSetTextColor(torneoSeleccionado == 2 ? COLOR_GREEN : COLOR_GRAY);
            pspDebugScreenPrintf("  [CUADRADO] -> COPA DEL MUNDO DE NACIONES\n\n");

            pspDebugScreenSetTextColor(COLOR_CYAN);
            pspDebugScreenPrintf(" Presiona [TRIANGULO] para saltar a la cancha.");

            if (pad.Buttons & PSP_CTRL_CROSS) torneoSeleccionado = 0;
            if (pad.Buttons & PSP_CTRL_CIRCLE) torneoSeleccionado = 1;
            if (pad.Buttons & PSP_CTRL_SQUARE) torneoSeleccionado = 2;

            if (pad.Buttons & PSP_CTRL_TRIANGLE) {
                minuto = 1;
                reputacion = 100;
                fisico = 100;
                presion = 15;
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
            pspDebugScreenSetTextColor(fisico < 40 ? COLOR_RED : COLOR_GREEN);
            pspDebugScreenPrintf("FISICO: %d%% | ", fisico);
            pspDebugScreenSetTextColor(COLOR_MAGENTA);
            pspDebugScreenPrintf("PRESION: %d%%\n", presion);

            DibujarBorde();

            Incidente j;
            if (torneoSeleccionado == 0) j = jugadasLMF[jugadaIdx];
            else if (torneoSeleccionado == 1) j = jugadasUCL[jugadaIdx];
            else j = jugadasMundial[jugadaIdx];

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("TORNEO: ");
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("%s\n", torneoSeleccionado == 0 ? "LMF EL SALVADOR" : (torneoSeleccionado == 1 ? "UCL CHAMPIONS" : "COPA DEL MUNDO"));

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
            pspDebugScreenPrintf(" CONTROLES DE SANCION:\n");
            pspDebugScreenPrintf("  [X]        -> Tarjeta Amarilla\n");
            pspDebugScreenPrintf("  [O]        -> Siga / Ley de Ventaja\n");
            pspDebugScreenPrintf("  [CUADRADO] -> Tarjeta Roja Directa\n");
            pspDebugScreenPrintf("  [L]        -> Advertencia Verbal\n");
            pspDebugScreenPrintf("  [TRIANGULO]-> Monitor VAR\n\n");

            int voto = 0;
            if (pad.Buttons & PSP_CTRL_CROSS) voto = 1;
            if (pad.Buttons & PSP_CTRL_CIRCLE) voto = 2;
            if (pad.Buttons & PSP_CTRL_SQUARE) voto = 3;
            if (pad.Buttons & PSP_CTRL_LTRIGGER) voto = 4;
            if (pad.Buttons & PSP_CTRL_TRIANGLE) voto = 5;

            if (voto > 0) {
                if (voto == 5) {
                    estado = 2;
                } else {
                    if (voto == j.respuestaCorrecta) {
                        pspDebugScreenSetTextColor(COLOR_GREEN);
                        pspDebugScreenPrintf("\n [DECISION ACERTADA] %s\n", j.explicacion);
                        aciertos++;
                        if (voto == 1) amarillas++;
                        if (voto == 3) rojas++;
                    } else {
                        pspDebugScreenSetTextColor(COLOR_RED);
                        pspDebugScreenPrintf("\n [ERROR] %s protesto el cobro.\n", j.jugador);
                        errores++;
                        reputacion -= 15;
                        presion += 12;
                    }

                    fisico -= 8;
                    minuto += 22;
                    jugadaIdx++;

                    if (jugadaIdx >= 4 || minuto >= 90) estado = 3;
                    sceKernelDelayThread(1800000);
                }
            }
        }
        else if (estado == 2) {
            DibujarBorde();
            pspDebugScreenSetTextColor(COLOR_CYAN);
            pspDebugScreenPrintf("|           SISTEMA DE REVISION VAR EN VIVO         |\n");
            DibujarBorde();

            Incidente j;
            if (torneoSeleccionado == 0) j = jugadasLMF[jugadaIdx];
            else if (torneoSeleccionado == 1) j = jugadasUCL[jugadaIdx];
            else j = jugadasMundial[jugadaIdx];

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("\n Trazando lineas y analizando jugada de ");
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("%s...\n\n", j.jugador);

            pspDebugScreenSetTextColor(COLOR_GREEN);
            pspDebugScreenPrintf(" RESOLUCION VAR: %s\n\n", j.explicacion);

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf(" Presiona [X] para aplicar la sancion final.");

            if (pad.Buttons & PSP_CTRL_CROSS) {
                aciertos++;
                minuto += 22;
                jugadaIdx++;
                estado = (jugadaIdx >= 4 || minuto >= 90) ? 3 : 1;
                sceKernelDelayThread(400000);
            }
        }
        else if (estado == 3) {
            DibujarBorde();
            pspDebugScreenSetTextColor(COLOR_YELLOW);
            pspDebugScreenPrintf("|            INFORME OFICIAL - FIN DE JUEGO        |\n");
            DibujarBorde();

            pspDebugScreenSetTextColor(COLOR_WHITE);
            pspDebugScreenPrintf("\n DESEMPEÑO EN EL TORNEO:\n");
            pspDebugScreenPrintf("  - Decisiones Correctas: %d/4\n", aciertos);
            pspDebugScreenPrintf("  - Tarjetas Amarillas: %d | Rojas: %d\n", amarillas, rojas);
            pspDebugScreenPrintf("  - Reputacion Final: %d%%\n\n", reputacion);

            if (reputacion >= 80) {
                pspDebugScreenSetTextColor(COLOR_GREEN);
                pspDebugScreenPrintf(" ACTA COMITE: Designado para la Gran Final del Torneo.\n");
            } else {
                pspDebugScreenSetTextColor(COLOR_RED);
                pspDebugScreenPrintf(" ACTA COMITE: Suspendido por malas decisiones.\n");
            }

            pspDebugScreenSetTextColor(COLOR_CYAN);
            pspDebugScreenPrintf("\n Presiona [X] para volver a la seleccion de ligas.");

            if (pad.Buttons & PSP_CTRL_CROSS) {
                estado = 0;
                sceKernelDelayThread(400000);
            }
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}
