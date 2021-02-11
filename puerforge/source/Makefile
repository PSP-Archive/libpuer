TARGET = puerforge
OBJS = main.o rpl.o exports.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c
LIBPUER = ../../libpuer

LIBDIR = ../lib
LDFLAGS = 
LIBS = $(LIBPUER)/libpuer.a -lstdc++ -lm -lpsppower -lpspsystemctrl_kernel -lpspkubridge

PSP_FW_VERSION = 271

PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_TITLE = PuerForge
#PSP_EBOOT_ICON= ICON0.png

BUILD_PRX = 1
#ENCRYPTion isn't compatible with kernel mode EBOOT
#ENCRYPT = 1
#USE_USER_LIBS = 1

SPECS = $(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f EBOOT.PBP ../../dist/PSP/GAME/puerforge/
