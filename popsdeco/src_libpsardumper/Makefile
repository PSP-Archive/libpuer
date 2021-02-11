TARGET = libpsardumper
OBJS = main.o libpsardumper.o pspdecrypt.o exports.o

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(PSPSDK)/kernelsdk/include ../source
CFLAGS = -O2 -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 271

BUILD_PRX = 1

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(PSPSDK)/kernelsdk/lib
LDFLAGS = 
LIBS = -lpspsemaphore -lpspmesgd_driver -lpspnwman_driver -lpsputilsforkernel -lpspsystemctrl_kernel

include $(PSPSDK)/lib/build_prx.mak

dist: all
	cp -f $(TARGET).prx ../../dist/PSP/GAME/500DC8ex/
	cp -f $(TARGET).prx ../../dist/PSP/GAME/popsdeco_rev1/
	mv -f $(TARGET).prx ../../dist/PSP/GAME/popsdeco_rev2/
