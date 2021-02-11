TARGET = autobranch
OBJS = main.o exports.o

#USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBPUER = ../../libpuer

LIBDIR =
LIBS = $(LIBPUER)/libpuer.a $(LIBPUER)/libpspsystemctrl_kernel.a -lpspkubridge
SPECS = $(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f $(TARGET).prx ../../dist/seplugins/
