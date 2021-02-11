TARGET = pspshutdown
OBJS = main.o exports.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c
LIBPUER = ../../libpuer

LIBDIR = ../lib
LDFLAGS = 
LIBS = $(LIBPUER)/libpuer.a -lpsppower

PSP_FW_VERSION = 271

PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_TITLE = PspShutdown
#PSP_EBOOT_ICON= ICON0.png

BUILD_PRX = 1
ENCRYPT = 1
USE_USER_LIBS = 1

SPECS = $(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f EBOOT.PBP ../../dist/PSP/GAME/pspshutdown/
