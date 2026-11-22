# Checks src/lib/util/chd.c over libretro-common's rchd against images
# MAME 0.139 wrote, run by hand:
#   make check
#   make check SANITIZER=address,undefined
#   make check SANITIZER=thread

ROOT     := ../..
LRC      := $(ROOT)/src/osd/retro/libretro-common
UTIL     := $(ROOT)/src/lib/util
CC       ?= cc

INCLUDES := -I$(ROOT)/src/osd -I$(ROOT)/src/osd/retro -I$(ROOT)/src/lib -I$(UTIL) -I$(LRC)/include
CHD_DEFS := -DHAVE_RCHD -DHAVE_RCHD_DEFLATE -DHAVE_RCHD_LZMA -DHAVE_RCHD_FLAC -DHAVE_RFLAC \
            -DHAVE_RZSTD -DHAVE_RCHD_ZSTD
CFLAGS   := -std=gnu89 -O2 -g -Wall -DCRLF=2 -DHAVE_THREADS $(CHD_DEFS) $(INCLUDES)
# src/lib/util, written for the core's INLINE
UTILFLAGS = $(CFLAGS) -DINLINE="static __inline__"

UTIL_SRC := chd corefile unicode astring bitmap palette
LRC_SRC  := $(LRC)/formats/chd/rchd.c $(LRC)/formats/7z/r7z_lzma.c $(LRC)/formats/flac/rflac.c \
            $(LRC)/encodings/encoding_huffman.c $(LRC)/encodings/encoding_rzstd.c \
            $(LRC)/encodings/encoding_xxh64.c $(LRC)/encodings/encoding_deflate.c \
            $(LRC)/encodings/encoding_crc32.c $(LRC)/encodings/encoding_utf.c \
            $(LRC)/features/features_cpu.c $(LRC)/time/rtime.c $(LRC)/hash/lrc_hash.c \
            $(LRC)/streams/file_stream.c $(LRC)/vfs/vfs_implementation.c \
            $(LRC)/file/file_path.c $(LRC)/file/file_path_io.c $(LRC)/string/stdstring.c \
            $(LRC)/string/rstrtod.c $(LRC)/compat/compat_strl.c $(LRC)/compat/compat_posix_string.c \
            $(LRC)/compat/compat_strcasestr.c $(LRC)/compat/fopen_utf8.c \
            $(LRC)/rthreads/rthreads.c $(LRC)/rthreads/retro_eventcount.c \
            $(LRC)/rthreads/tpool.c $(ROOT)/src/osd/retro/retrothread.c
OBJS     := chd_test.o $(UTIL_SRC:%=util_%.o) $(notdir $(LRC_SRC:.c=.o))
LIBS     := -lm
ifneq ($(OS),Windows_NT)
LIBS     += -lpthread
endif

ifneq ($(SANITIZER),)
CFLAGS   += -fsanitize=$(SANITIZER) -fno-omit-frame-pointer
LIBS     += -fsanitize=$(SANITIZER)
endif

vpath %.c $(sort $(dir $(LRC_SRC)))

chd_test: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

chd_test.o: chd_test.c chd_fixtures.h
	$(CC) $(UTILFLAGS) -c $< -o $@

util_%.o: $(UTIL)/%.c
	$(CC) $(UTILFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

check: chd_test
	./chd_test

clean:
	rm -f chd_test chd_test.exe *.o chd_*.chd

.PHONY: check clean
