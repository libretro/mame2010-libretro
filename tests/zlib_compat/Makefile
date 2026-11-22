# Checks the zlib-format paths in src/lib/util against streams zlib wrote,
# and the crc32 and SHA-1 they use, run by hand:
#   make check
#   make check SANITIZER=address,undefined

ROOT     := ../..
LRC      := $(ROOT)/src/osd/retro/libretro-common
UTIL     := $(ROOT)/src/lib/util
CC       ?= cc

INCLUDES := -I$(ROOT)/src/osd -I$(ROOT)/src/osd/retro -I$(ROOT)/src/lib -I$(UTIL) -I$(LRC)/include
CFLAGS   := -std=gnu89 -O2 -g -Wall -DCRLF=2 -DHAVE_THREADS $(INCLUDES)
# src/lib/util, written for the core's INLINE
UTILFLAGS = $(CFLAGS) -DINLINE="static __inline__"

UTIL_SRC := corefile unzip png chd md5 avcomp huffman unicode bitmap palette astring
LRC_SRC  := $(LRC)/encodings/encoding_deflate.c $(LRC)/encodings/encoding_crc32.c \
            $(LRC)/features/features_cpu.c $(LRC)/time/rtime.c $(LRC)/compat/compat_strl.c \
            $(LRC)/hash/lrc_hash.c $(LRC)/streams/file_stream.c $(LRC)/vfs/vfs_implementation.c \
            $(LRC)/file/file_path.c $(LRC)/file/file_path_io.c $(LRC)/string/stdstring.c \
            $(LRC)/string/rstrtod.c $(LRC)/encodings/encoding_utf.c $(LRC)/compat/compat_posix_string.c \
            $(LRC)/compat/compat_strcasestr.c $(LRC)/compat/fopen_utf8.c \
            $(LRC)/rthreads/rthreads.c $(LRC)/rthreads/retro_eventcount.c \
            $(LRC)/rthreads/tpool.c $(ROOT)/src/osd/retro/retrothread.c
OBJS     := zlib_compat_test.o $(UTIL_SRC:%=util_%.o) $(notdir $(LRC_SRC:.c=.o))
LIBS     := -lm
ifneq ($(OS),Windows_NT)
LIBS     += -lpthread
endif

ifneq ($(SANITIZER),)
CFLAGS   += -fsanitize=$(SANITIZER) -fno-omit-frame-pointer
LIBS     += -fsanitize=$(SANITIZER)
endif

vpath %.c $(sort $(dir $(LRC_SRC)))

zlib_compat_test: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

zlib_compat_test.o: zlib_compat_test.c fixtures.h
	$(CC) $(UTILFLAGS) -c $< -o $@

util_%.o: $(UTIL)/%.c
	$(CC) $(UTILFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

check: zlib_compat_test
	./zlib_compat_test

clean:
	rm -f zlib_compat_test zlib_compat_test.exe *.o zc_*

.PHONY: check clean
