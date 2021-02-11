# PSP Software Development Kit - http://www.pspdev.org
# -----------------------------------------------------------------------
# Licensed under the BSD license, see LICENSE in PSPSDK root for details.
#
# build.mak - Base makefile for projects using PSPSDK.
#
# Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
# Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
# Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
#
# $Id: build.mak 771 2005-07-24 10:43:54Z tyranid $

# Note: The PSPSDK make variable must be defined before this file is included.
PSPSDK=$(shell psp-config --pspsdk-path)
PSPKERNELSDK ?= $(PSPSDK)/kernelsdk

### In libpuer, you must use exports.exp even for EBOOT. Be careful.
### PrxEncrypter source code is available in procfw/contrib/PrxEncrypter/.
### fix-relocations source code is available in fix-relocations.c.

#ifeq ($(PSPSDK),)
#$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
#endif

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-gcc
GDC      = psp-gdc
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip
MKSFO    = mksfo
PACK_PBP = pack-pbp
FIXUP    = psp-fixup-imports
ENC		 = PrxEncrypter
FIXRELO  = fix-relocations

# Add in PSPSDK includes and libraries.
INCDIR   := $(INCDIR) . $(PSPSDK)/include $(PSPKERNELSDK)/include
LIBDIR   := $(LIBDIR) . $(PSPSDK)/lib $(PSPKERNELSDK)/lib

CFLAGS   := -G0 -march=allegrex -mabi=eabi -fno-builtin-printf $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
ASFLAGS  := $(CFLAGS) $(ASFLAGS)

LDFLAGS  := $(addprefix -L,$(LIBDIR)) -specs=$(SPECS) -Wl,-q,-T$(PSPSDK)/lib/linkfile.prx $(LDFLAGS)

ifeq ($(PSP_FW_VERSION),)
PSP_FW_VERSION=401
endif

CFLAGS += -D_PSP_FW_VERSION=$(PSP_FW_VERSION) -DPSP=1
ifeq ($(ENCRYPT),1)
CFLAGS += -DENCRYPT=1
endif

# Objective-C selection. All Objective C code must be linked against libobjc.a
ifeq ($(USE_OBJC),1)
LIBS     := $(LIBS) -lobjc
endif

# Library selection.  By default we link with Newlib's libc.  Allow the
# user to link with PSPSDK's libc if USE_PSPSDK_LIBC is set to 1.
ifeq ($(USE_KERNEL_LIBC),1)
# Use the PSP's kernel libc
PSPSDK_LIBC_LIB = 
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
ifeq ($(USE_PSPSDK_LIBC),1)
# Use the pspsdk libc
PSPSDK_LIBC_LIB = -lpsplibc
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
# Use newlib (urgh)
PSPSDK_LIBC_LIB = -lc
endif
endif

# Link with following default libraries.  Other libraries should be specified in the $(LIBS) variable.
# TODO: This library list needs to be generated at configure time.
ifeq ($(USE_KERNEL_LIBS),1)
#really damn, need to link -lpspuser due to _sbrk()'s sceKernelMaxFreeMemSize() dependency.
PSPSDK_LIBS = -lpspdebug -lpspdisplay_driver -lpspctrl_driver -lpspsdk -lpspumd_driver
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel -lpspuser -lpsputility
else
ifeq ($(USE_USER_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk -lpspumd
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser
else
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk -lpspumd
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser -lpspkernel
endif
endif

ifneq ($(TARGET_LIB),)
FINAL_TARGET = $(TARGET_LIB)
else
FINAL_TARGET = $(TARGET).prx
endif

all: $(FINAL_TARGET) $(PSP_EBOOT)

$(TARGET_LIB): $(OBJS)
	$(AR) crus $@ $(OBJS)
	$(RANLIB) $@

#basic rules until prx
$(TARGET).elf: $(OBJS)
	$(LINK.c) $^ $(LIBS) -o $@
	$(FIXUP) $@

%.prx: %.elf
	psp-prxgen $< $@

%.o: %.m
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.d
	$(GDC) $(DFLAGS) -c -o $@ $<

%.c: %.exp
	psp-build-exports -b $< > $@

#ifneq ($(PSP_EBOOT),)
clean:
	$(RM) -f $(TARGET).prx $(TARGET).elf exports.c $(OBJS) $(PSP_EBOOT_SFO)
#else
#clean:
#	$(RM) -f $(TARGET).elf $(OBJS)
#endif

rebuild: clean all

#EBOOT
# Define the overridable parameters for EBOOT.PBP
ifndef PSP_EBOOT_TITLE
PSP_EBOOT_TITLE = $(TARGET)
endif

ifndef PSP_EBOOT_SFO
PSP_EBOOT_SFO = PARAM.SFO
endif

ifndef PSP_EBOOT_ICON
PSP_EBOOT_ICON = NULL
endif

ifndef PSP_EBOOT_ICON1
PSP_EBOOT_ICON1 = NULL
endif

ifndef PSP_EBOOT_UNKPNG
PSP_EBOOT_UNKPNG = NULL
endif

ifndef PSP_EBOOT_PIC1
PSP_EBOOT_PIC1 = NULL
endif

ifndef PSP_EBOOT_SND0
PSP_EBOOT_SND0 = NULL
endif

ifndef PSP_EBOOT_PSAR
PSP_EBOOT_PSAR = NULL
endif

#ifndef PSP_EBOOT
#PSP_EBOOT = EBOOT.PBP
#endif

$(PSP_EBOOT_SFO):
	$(MKSFO) '$(PSP_EBOOT_TITLE)' $@

$(PSP_EBOOT): $(TARGET).prx $(PSP_EBOOT_SFO)
ifeq ($(ENCRYPT), 1)
	$(FIXRELO) $(TARGET).prx
	$(ENC) $(TARGET).prx data.psp
	$(PACK_PBP) $(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0)  data.psp $(PSP_EBOOT_PSAR)
		$(RM) -f data.psp
else
	$(PACK_PBP) $(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0)  $(TARGET).prx $(PSP_EBOOT_PSAR)
endif
