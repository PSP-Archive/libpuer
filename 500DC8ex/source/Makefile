TARGET = 500DC8ex
OBJS = main.o util.o md5.o pspPSAR.o exports.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c
LIBPUER = ../../libpuer

LIBDIR = ../lib
LDFLAGS = 
LIBS = $(LIBPUER)/libpuer.a -lpsppower

PSP_FW_VERSION = 271

PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_TITLE = 5.00 DC8 Extreme

BUILD_PRX = 1
ENCRYPT = 0

SPECS=$(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f EBOOT.PBP ../../dist/PSP/GAME/500DC8ex/
