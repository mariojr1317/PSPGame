TARGET = ArbitroPSP
OBJS = main.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Juego Arbitro PSP

PSPBIN = $(shell psp-config --pspdev-path)/bin
include $(PSPBIN)/build.mak
