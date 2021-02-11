TARGET = usbenabler
OBJS = main.o exports.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c
LIBPUER = ../../libpuer

LIBDIR = ../lib
LDFLAGS = 
LIBS = $(LIBPUER)/libpuer.a -lpspusb -lpspctrl -lpspusbstor -lpspusbdevice -lpspsystemctrl_user

PSP_FW_VERSION = 271

BUILD_PRX = 1
USE_USER_LIBS = 1

SPECS = $(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f usbenabler.prx ../../dist/seplugins/
