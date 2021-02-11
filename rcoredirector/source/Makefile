TARGET = rcoredirector
OBJS = main.o exports.o

#USE_KERNEL_LIBC = 1
USE_USER_LIBS = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBPUER = ../../libpuer

LIBDIR =
LIBS = $(LIBPUER)/libpuer.a -lpspsystemctrl_user -lpsppower
SPECS = $(LIBPUER)/specs
include $(LIBPUER)/puer.mak

dist: all
	mv -f $(TARGET).prx ../../dist/seplugins/
