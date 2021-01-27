LOCAL_PATH := $(call my-dir)

define uniq
	$(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
endef

DEFS :=

TARGET := mame
OSD    := retro

CORE_DIR := $(LOCAL_PATH)/..
OBJ      := $(CORE_DIR)/src

LIBEMU   := $(OBJ)/libemu.a
LIBCPU   := $(OBJ)/libcpu.a
LIBDASM  := $(OBJ)/libdasm.a
LIBSOUND := $(OBJ)/libsound.a
LIBUTIL  := $(OBJ)/libutil.a
LIBOCORE := $(OBJ)/libocore.a
LIBOSD   := $(OBJ)/libosd.a


ifneq ($(filter $(TARGET_ARCH_ABI), arm64-v8a x86_64),)
  PTR64 := 1
  DEFS  += -DPTR64
endif

ifeq ($(filter $(TARGET_ARCH_ABI), x86 x86_64),)
  FORCE_DRC_C_BACKEND := 1
endif

include $(CORE_DIR)/Makefile.common

COREFLAGS := $(DEFS) $(INCFLAGS)
COREFLAGS += -DCRLF=2 -DINLINE="static inline" -Wno-c++11-narrowing -Wno-reserved-user-defined-literal -Wno-deprecated

# For testing only, remove before PR
COREFLAGS += -Wno-implicit-exception-spec-mismatch -Wno-inline-new-delete -Wno-tautological-undefined-compare

GIT_VERSION ?= " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

LIBS         := $(basename $(notdir $(filter %.a,$(OBJECTS))))
LIBS_UPPER   := $(shell echo $(LIBS) | tr '[:lower:]' '[:upper:]')
SOURCES_OBJ  := $(filter-out %.a,$(OBJECTS)) $(foreach LIB,$(LIBS_UPPER),$($(LIB)_OBJS))
SOURCES_UNIQ := $(call uniq,$(SOURCES_OBJ))
SOURCES_C    := $(SOURCES_UNIQ:%.o=%.c)

include $(CLEAR_VARS)
LOCAL_MODULE        := retro
LOCAL_SRC_FILES     := $(SOURCES_C)
LOCAL_CPPFLAGS      := -std=gnu++11 $(COREFLAGS)
LOCAL_CPP_FEATURES  := exceptions rtti
LOCAL_CPP_EXTENSION := .c
LOCAL_LDLIBS        := -lz

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON := true
endif

include $(BUILD_SHARED_LIBRARY)
