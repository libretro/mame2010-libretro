###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################
NATIVE :=0

UNAME=$(shell uname -a)

ifeq ($(platform),)
platform = unix
ifeq ($(UNAME),)
   platform = win
else ifneq ($(findstring MINGW,$(UNAME)),)
   platform = win
else ifneq ($(findstring Darwin,$(UNAME)),)
   platform = osx
else ifneq ($(findstring win,$(UNAME)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(UNAME),)
EXE_EXT = .exe
   system_platform = win
else ifneq ($(findstring Darwin,$(UNAME)),)
   system_platform = osx
else ifneq ($(findstring MINGW,$(UNAME)),)
   system_platform = win
endif

# CR/LF setup: use both on win32/os2, CR only on everything else
DEFS = -DCRLF=2 -DDISABLE_MIDI=1
# Default to something reasonable for all platforms
ARFLAGS = -cr

TARGET_NAME := mame2010
EXE = 
LIBS = 

#-------------------------------------------------
# compile flags
# CCOMFLAGS are common flags
# CONLYFLAGS are flags only used when compiling for C
# CPPONLYFLAGS are flags only used when compiling for C++
# COBJFLAGS are flags only used when compiling for Objective-C(++)
#-------------------------------------------------

# start with empties for everything
CCOMFLAGS = -DDISABLE_MIDI
CONLYFLAGS =
COBJFLAGS =
CPPONLYFLAGS =
# LDFLAGS are used generally; LDFLAGSEMULATOR are additional
# flags only used when linking the core emulator
LDFLAGS =
LDFLAGSEMULATOR =

# uncomment next line to build zlib as part of MAME build
#BUILD_ZLIB = 1

# uncomment next line to build PortMidi as part of MAME/MESS build
#BUILD_MIDILIB = 1
VRENDER ?= soft

PLATCFLAGS += -D__LIBRETRO__
CCOMFLAGS  += -D__LIBRETRO__

fpic := 
ALIGNED = 0

ifeq ($(VRENDER),opengl)  
	PLATCFLAGS += -DHAVE_GL
	CCOMFLAGS  += -DHAVE_GL
endif

UNAME=$(shell uname -m)

ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
PTR64 = 1
endif
ifeq ($(firstword $(filter amd64,$(UNAME))),amd64)
PTR64 = 1
endif
ifeq ($(firstword $(filter ppc64,$(UNAME))),ppc64)
PTR64 = 1
endif
ifneq (,$(findstring mingw64-w64,$(PATH)))
PTR64=1
endif
ifneq (,$(findstring Power,$(UNAME)))
BIGENDIAN=1
endif
ifneq (,$(findstring ppc,$(UNAME)))
BIGENDIAN=1
endif

CORE_DIR = .

# UNIX
ifeq ($(platform), unix)
   TARGETLIB := $(TARGET_NAME)_libretro.so
	TARGETOS=linux  
   fpic = -fPIC
   SHARED := -shared -Wl,--version-script=src/osd/retro/link.T
   CCOMFLAGS += -fsigned-char -finline  -fno-common -fno-builtin -fweb -frename-registers -falign-functions=16 -fsingle-precision-constant
	ALIGNED=1
   PLATCFLAGS += -fstrict-aliasing -fno-merge-constants 
ifeq ($(VRENDER),opengl)  
   LIBS += -lGL
endif
LDFLAGS += $(SHARED)
   NATIVELD = g++
   NATIVELDFLAGS = -Wl,--warn-common -lstdc++
   NATIVECC = g++
   NATIVECFLAGS = -std=gnu99
   CC_AS = gcc 
   CC = g++
   AR = @ar
   LD = g++ 
   CCOMFLAGS += $(PLATCFLAGS) -ffast-math  
   LIBS += -lstdc++ -lpthread 

# Android
else ifeq ($(platform), android)
EXTRA_RULES = 1
ARM_ENABLED = 1
   TARGETLIB := $(TARGET_NAME)_libretro.so
	TARGETOS=linux  
   fpic = -fPIC
   SHARED := -shared -Wl,--version-script=src/osd/retro/link.T
	CC_AS = @arm-linux-androideabi-gcc
	CC = @arm-linux-androideabi-g++
	AR = @arm-linux-androideabi-ar
	LD = @arm-linux-androideabi-g++ 
	ALIGNED=1
	FORCE_DRC_C_BACKEND = 1
	CCOMFLAGS += -mstructure-size-boundary=32 -mthumb-interwork -falign-functions=16 -fsigned-char -finline  -fno-common -fno-builtin -fweb -frename-registers -falign-functions=16 -fsingle-precision-constant
	PLATCFLAGS += -march=armv7-a -mfloat-abi=softfp -fstrict-aliasing -fno-merge-constants -DSDLMAME_NO64BITIO -DANDTIME -DRANDPATH
	PLATCFLAGS += -DANDROID
	LDFLAGS += -Wl,--fix-cortex-a8 -llog $(SHARED)
	NATIVELD = g++
	NATIVELDFLAGS = -Wl,--warn-common -lstdc++
	NATIVECC = g++
	NATIVECFLAGS = -std=gnu99 
	CCOMFLAGS += $(PLATCFLAGS) -ffast-math  
	LIBS += -lstdc++ 

# OS X
else ifeq ($(platform), osx)
   TARGETLIB := $(TARGET_NAME)_libretro.dylib
	TARGETOS = macosx
   fpic = -fPIC
LDFLAGSEMULATOR +=  -stdlib=libc++
   SHARED := -dynamiclib
   CC = c++ -stdlib=libc++
   LD = c++ -stdlib=libc++
   NATIVELD = c++ -stdlib=libc++
   NATIVECC = c++
	LDFLAGS +=  $(SHARED)
   CC_AS = clang
   AR = @ar
ifeq ($(COMMAND_MODE),"legacy")
ARFLAGS = -crs
endif

# iOS
else ifneq (,$(findstring ios,$(platform)))

   TARGETLIB := $(TARGET_NAME)_libretro_ios.dylib
   TARGETOS = macosx
   EXTRA_RULES = 1
   ARM_ENABLED = 1
   fpic = -fPIC
   SHARED := -dynamiclib
   PTR64 = 0

ifeq ($(IOSSDK),)
IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
endif

   CC = c++ -arch armv7 -isysroot $(IOSSDK)
   CCOMFLAGS += -DSDLMAME_NO64BITIO -DIOS
   CFLAGS += -DIOS
   CXXFLAGS += -DIOS
   NATIVELD = $(CC) -stdlib=libc++
   LDFLAGS +=  $(SHARED)
   LD = $(CC)

# QNX
else ifeq ($(platform), qnx)
   TARGETLIB := $(TARGET_NAME)_libretro_$(platform).so
	TARGETOS=linux  
   fpic = -fPIC
   SHARED := -shared -Wl,--version-script=src/osd/retro/link.T

   CC = qcc -Vgcc_ntoarmv7le
   AR = qcc -Vgcc_ntoarmv7le
   CFLAGS += -D__BLACKBERRY_QNX__
	LIBS += -lstdc++ -lpthread

# PS3
else ifeq ($(platform), ps3)
   TARGETLIB := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# PS3 (SNC)
else ifeq ($(platform), sncps3)
   TARGETLIB := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# Lightweight PS3 Homebrew SDK
else ifeq ($(platform), psl1ght)
   TARGETLIB := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CFLAGS += -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# PSP
else ifeq ($(platform), psp1)
	TARGETLIB := $(TARGET_NAME)_libretro_$(platform).a
	CC = psp-g++$(EXE_EXT)
	AR = psp-ar$(EXE_EXT)
	CFLAGS += -DPSP -G0
	STATIC_LINKING = 1
	LIBS += -lstdc++ -lpthread

# Xbox 360
else ifeq ($(platform), xenon)
   TARGETLIB := $(TARGET_NAME)_libretro_xenon360.a
   CC = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   CFLAGS += -D__LIBXENON__ -m32 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# Nintendo Game Cube
else ifeq ($(platform), ngc)
   TARGETLIB := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# Nintendo Wii
else ifeq ($(platform), wii)
   TARGETLIB := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -DBLARGG_BIG_ENDIAN=1 -D__ppc__
	STATIC_LINKING = 1
	BIGENDIAN=1
	LIBS += -lstdc++ -lpthread

# ARM
else ifneq (,$(findstring armv,$(platform)))
   TARGETLIB := $(TARGET_NAME)_libretro.so
   SHARED := -shared -Wl,--no-undefined
   fpic = -fPIC
   CC = g++
ifneq (,$(findstring cortexa8,$(platform)))
   CFLAGS += -marm -mcpu=cortex-a8
   ASFLAGS += -mcpu=cortex-a8
else ifneq (,$(findstring cortexa9,$(platform)))
   CFLAGS += -marm -mcpu=cortex-a9
   ASFLAGS += -mcpu=cortex-a9
endif
   CFLAGS += -marm
ifneq (,$(findstring neon,$(platform)))
   CFLAGS += -mfpu=neon
   ASFLAGS += -mfpu=neon
   HAVE_NEON = 1
endif
ifneq (,$(findstring softfloat,$(platform)))
   CFLAGS += -mfloat-abi=softfp
   ASFLAGS += -mfloat-abi=softfp
else ifneq (,$(findstring hardfloat,$(platform)))
   CFLAGS += -mfloat-abi=hard
   ASFLAGS += -mfloat-abi=hard
endif
   CFLAGS += -DARM
	LIBS += -lstdc++ -lpthread

else ifeq ($(platform), wincross)
   TARGETLIB := $(TARGET_NAME)_libretro.dll
	TARGETOS = win32
	CC ?= g++
	LD ?= g++
	NATIVELD = $(LD)
	CC_AS ?= gcc

	SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=src/osd/retro/link.T
	CCOMFLAGS +=-D__WIN32__ -D__WIN32_LIBRETRO__
ifeq ($(VRENDER),opengl)  
	LIBS += -lopengl32
endif
	LDFLAGS +=   $(SHARED)
	EXE = .exe
	DEFS = -DCRLF=3

# Windows
else
   TARGETLIB := $(TARGET_NAME)_libretro.dll
	TARGETOS = win32
	CC = g++
	LD = g++
	NATIVELD = $(LD)
	CC_AS = gcc
	SHARED := -shared -static-libgcc -static-libstdc++ -Wl,--version-script=src/osd/retro/link.T
ifneq ($(MDEBUG),1)
	SHARED += -s
endif
CCOMFLAGS += -D__WIN32__ -D__WIN32_LIBRETRO__
ifeq ($(VRENDER),opengl)  
	LIBS += -lopengl32
endif
	LDFLAGS +=   $(SHARED)
	EXE = .exe
	DEFS = -DCRLF=3
	DEFS += -DX64_WINDOWS_ABI
endif



ifeq ($(ALIGNED),1)
	PLATCFLAGS += -DALIGN_INTS -DALIGN_SHORTS 
endif

CCOMFLAGS += $(fpic)
LDFLAGS   += $(fpic)

###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################

TARGET=mame
SUBTARGET=mame
OSD=retro


NOWERROR = 1

#-------------------------------------------------
# specify core target: mame, mess, etc.
# specify subtarget: mame, mess, tiny, etc.
# build rules will be included from
# src/$(TARGET)/$(SUBTARGET).mak
#-------------------------------------------------

#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build rules will be included from
# src/osd/$(OSD)/$(OSD).mak
#-------------------------------------------------

CROSS_BUILD_OSD = retro

#-------------------------------------------------
# configure name of final executable
#-------------------------------------------------

# uncomment and specify prefix to be added to the name
# PREFIX =

# uncomment and specify suffix to be added to the name
# SUFFIX =



#-------------------------------------------------
# specify architecture-specific optimizations
#-------------------------------------------------

# uncomment and specify architecture-specific optimizations here
# some examples:
#   ARCHOPTS = -march=pentiumpro  # optimize for I686
#   ARCHOPTS = -march=core2       # optimize for Core 2
#   ARCHOPTS = -march=native      # optimize for local machine (auto detect)
#   ARCHOPTS = -mcpu=G4           # optimize for G4
# note that we leave this commented by default so that you can
# configure this in your environment and never have to think about it
# ARCHOPTS =



#-------------------------------------------------
# specify program options; see each option below
# for details
#-------------------------------------------------

# uncomment the force the universal DRC to always use the C backend
# you may need to do this if your target architecture does not have
# a native backend
# FORCE_DRC_C_BACKEND = 1

#-------------------------------------------------
# specify build options; see each option below 
# for details
#-------------------------------------------------


# specify optimization level or leave commented to use the default
# (default is OPTIMIZE = 3 normally, or OPTIMIZE = 0 with symbols)
OPTIMIZE = 3


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################

#-------------------------------------------------
# platform-specific definitions
#-------------------------------------------------

# utilities
MD = -mkdir$(EXE_EXT)
RM = @rm -f
OBJDUMP = @objdump

#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

# reset all internal prefixes/suffixes
SUFFIX64 =
SUFFIXDEBUG =
SUFFIXPROFILE =

# 64-bit builds get a '64' suffix
ifeq ($(PTR64),1)
SUFFIX64 = 64
endif

# add an EXE suffix to get the final emulator name
EMULATOR = $(TARGET)

#-------------------------------------------------
# source and object locations
#-------------------------------------------------

# build the targets in different object dirs, so they can co-exist
OBJ = obj/$(PREFIX)$(OSD)$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXPROFILE)

#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------
# map the INLINE to something digestible by GCC
DEFS += -DINLINE="static inline"

# define MSB_FIRST if we are a big-endian target
ifdef BIGENDIAN
DEFS       += -DMSB_FIRST
PLATCFLAGS += -DMSB_FIRST
endif

# define PTR64 if we are a 64-bit target
ifeq ($(PTR64),1)
DEFS += -DPTR64
endif

DEFS += -DNDEBUG 

# need to ensure FLAC functions are statically linked
DEFS += -DFLAC__NO_DLL

# CFLAGS is defined based on C or C++ targets
# (remember, expansion only happens when used, so doing it here is ok)
CFLAGS = $(CCOMFLAGS) $(CPPONLYFLAGS)

# we compile C-only to C89 standard with GNU extensions
# we compile C++ code to C++98 standard with GNU extensions
#CONLYFLAGS += -std=gnu89
ifeq ($(platform), osx)
CONLYFLAGS += -ansi
else
CONLYFLAGS += -std=gnu89
endif
CPPONLYFLAGS += -x c++ -std=gnu++98
COBJFLAGS += -x objective-c++

# this speeds it up a bit by piping between the preprocessor/compiler/assembler
CCOMFLAGS += -pipe

ifeq ($(MDEBUG),1)
CCOMFLAGS +=  -O0 -g
else
# add the optimization flag
CCOMFLAGS += -O$(OPTIMIZE)
endif

# add the error warning flag
ifndef NOWERROR
CCOMFLAGS += -Werror
endif

# if we are optimizing, include optimization options
ifneq ($(OPTIMIZE),0)
CCOMFLAGS += -fno-strict-aliasing $(ARCHOPTS)
endif

# add a basic set of warnings
CCOMFLAGS += \
	-Wall \
	-Wcast-align \
	-Wundef \
	-Wformat-security \
	-Wwrite-strings \
	-Wno-sign-compare \
	-Wno-conversion

# warnings only applicable to C compiles
CONLYFLAGS += \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wstrict-prototypes

# warnings only applicable to OBJ-C compiles
COBJFLAGS += \
	-Wpointer-arith 

# warnings only applicable to C++ compiles
CPPONLYFLAGS += \
	-Woverloaded-virtual

ifneq (,$(findstring clang,$(CC)))
CCOMFLAGS += \
	-Wno-cast-align \
	-Wno-tautological-compare \
	-Wno-constant-logical-operand \
	-Wno-format-security \
	-Wno-shift-count-overflow \
	-Wno-self-assign-field
endif

#-------------------------------------------------
# define the standard object directory; other
# projects can add their object directories to
# this variable
#-------------------------------------------------

OBJDIRS = $(OBJ) $(OBJ)/$(TARGET)/$(SUBTARGET)


#-------------------------------------------------
# define standard libarires for CPU and sounds
#-------------------------------------------------
LIBEMU   = $(OBJ)/libemu.a
LIBCPU   = $(OBJ)/libcpu.a
LIBDASM  = $(OBJ)/libdasm.a
LIBSOUND = $(OBJ)/libsound.a
LIBUTIL  = $(OBJ)/libutil.a
LIBOCORE = $(OBJ)/libocore.a
LIBOSD   = $(OSDOBJS)

#-------------------------------------------------
# 'default' target needs to go here, before the
# include files which define additional targets
#-------------------------------------------------

default: maketree buildtools emulator

all: default tools

tests: maketree jedutil$(EXE_EXT) chdman$(EXE_EXT)

7Z_LIB = $(OBJ)/lib7z.a

#-------------------------------------------------
# defines needed by multiple make files
#-------------------------------------------------

BUILDSRC = $(CORE_DIR)/src/build
BUILDOBJ = $(OBJ)/build
BUILDOUT = $(BUILDOBJ)

include Makefile.common

# combine the various definitions to one
CCOMFLAGS += $(INCFLAGS)
CDEFS = $(DEFS)

#-------------------------------------------------
# primary targets
#-------------------------------------------------

emulator: maketree $(BUILD) $(EMULATOR)

buildtools: maketree $(BUILD)

ifeq ($(NATIVE),1)
	mkdir prec-build
	cp -R $(OBJ)/build/* prec-build/
	$(RM) -r $(OBJ)/osd
	$(RM) -r $(OBJ)/lib/util
	$(RM) -r $(OBJ)/libutil.a
	$(RM) -r $(OBJ)/libocore.a
endif

tools: maketree $(TOOLS)

maketree: $(sort $(OBJDIRS))

clean: $(OSDCLEAN)
	@echo Deleting object tree $(OBJ)...
	$(RM) -r obj/*
	@echo Deleting $(EMULATOR)...
	$(RM) $(EMULATOR)
	@echo Deleting $(TOOLS)...
	$(RM) $(TOOLS)
	@echo Deleting dependencies...
	$(RM) depend_mame.mak
	$(RM) depend_mess.mak
	$(RM) depend_ume.mak

checkautodetect:
	@echo TARGETOS=$(TARGETOS)
	@echo PTR64=$(PTR64)
	@echo BIGENDIAN=$(BIGENDIAN)
	@echo UNAME="$(UNAME)"

tests: $(REGTESTS)


#-------------------------------------------------
# directory targets
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) -p $@

ifeq ($(EXTRA_RULES), 1)
ifeq ($(NATIVE),0)
$(OBJ)/build/makedep:
	cp -R prec-build/makelist $(OBJ)/build

$(OBJ)/build/makelist:
	cp -R prec-build/makelist $(OBJ)/build
endif
endif

#-------------------------------------------------
# executable targets and dependencies
#-------------------------------------------------
$(EMULATOR): $(OBJECTS)
	@echo Linking $(TARGETLIB)
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $(TARGETLIB)

#endif
#-------------------------------------------------
# generic rules
#-------------------------------------------------

ifeq ($(ARM_ENABLED), 1)
CFLAGS += -DARM_ENABLED
$(LIBCOOBJ)/armeabi_asm.o:
	$(CC_AS) -c $(CORE_DIR)/src/osd/retro/libco/armeabi_asm.S -o $(LIBCOOBJ)/armeabi_asm.o
endif

$(OBJ)/%.o: $(CORE_DIR)/src/%.c | $(OSPREBUILD)
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(OBJ)/%.c | $(OSPREBUILD)
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.pp: $(CORE_DIR)/src/%.c | $(OSPREBUILD)
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: $(CORE_DIR)/src/%.c | $(OSPREBUILD)
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(DRIVLISTOBJ): $(DRIVLISTSRC)
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(DRIVLISTSRC): $(CORE_DIR)/src/$(TARGET)/$(SUBTARGET).lst $(MAKELIST_TARGET)
	@echo Building driver list $<...
	@$(MAKELIST) $< >$@

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
