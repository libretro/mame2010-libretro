# Stress test for src/osd/retro/retrothread.c, run by hand:
#   make check              plain build
#   make check SANITIZER=address,undefined
#   make check SANITIZER=thread
#   make check THREADS=0    the single-threaded fallback

ROOT     := ../..
LRC      := $(ROOT)/src/osd/retro/libretro-common
CC       ?= cc
THREADS  ?= 1

CFLAGS   := -std=gnu89 -O2 -g -Wall -DCRLF=2
CFLAGS   += -I$(ROOT)/src/osd -I$(ROOT)/src/osd/retro -I$(LRC)/include
SOURCES  := osd_thread_test.c $(ROOT)/src/osd/retro/retrothread.c
LIBS     :=

ifeq ($(THREADS), 1)
CFLAGS   += -DHAVE_THREADS
SOURCES  += $(LRC)/rthreads/rthreads.c $(LRC)/rthreads/retro_eventcount.c \
            $(LRC)/rthreads/tpool.c
ifneq ($(OS),Windows_NT)
LIBS     += -lpthread
endif
endif

ifneq ($(SANITIZER),)
CFLAGS   += -fsanitize=$(SANITIZER) -fno-omit-frame-pointer
LIBS     += -fsanitize=$(SANITIZER)
endif

osd_thread_test: $(SOURCES) $(ROOT)/src/osd/osdcore.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LIBS)

check: osd_thread_test
	./osd_thread_test

clean:
	rm -f osd_thread_test osd_thread_test.exe

.PHONY: check clean
