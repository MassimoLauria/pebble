# Copyright (C) 2010, 2011, 2012, 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>
#
# Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
# Time-stamp: "2014-06-02, 09:33 (EDT) Massimo Lauria"

# ---------- BUILD FLAGS ----------------------
RELEASE=0

CONFIG_HASHSIZE=0x07FFFF
PRINT_STATS_INTERVAL=0    # set to 0 to disable it

BUILDFLAGS=	-DPRINT_STATS_INTERVAL=${PRINT_STATS_INTERVAL}
BUILDFLAGS+=-DCONFIG_HASHSIZE=${CONFIG_HASHSIZE}


# ---------- Environment variables ------------
#

ifeq ($(RELEASE),1)
DEBUG=-DNDEBUG
PROFILE=
OPTIMIZATION=-O3
else
DEBUG=-g
PROFILE=-pg
OPTIMIZATION=
endif


# Variant flags for the various pebbling tools
ifdef BLACK_WHITE_PEBBLING
VARIANTFLAGS+=-DBLACK_WHITE_PEBBLING=${BLACK_WHITE_PEBBLING}
endif

ifdef REVERSIBLE_PEBBLING
VARIANTFLAGS+=-DREVERSIBLE_PEBBLING=${REVERSIBLE_PEBBLING}
endif


CC=gcc
#CC=cc

C_STANDARD=-std=c99
#C_STANDARD=-ansi
#C_STANDARD=-std=c89

TAGS=gtags # etags ctags
TAGFILES=GPATH GRTAGS GSYMS GTAGS tags TAGS ID

CFLAGS=${OPTIMIZATION} ${DEBUG} ${PROFILE} ${BUILDFLAGS} ${VARIANTFLAGS} -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=${DEBUG} ${PROFILE}


# --------- Project dependent rules ---------------
NAME=pebble
TARGET=bwpebble pebble revpebble exposetypes
TIME=$(shell date +%Y.%m.%d-%H.%M)

# Source files which compilation does not depend on the pebbling variant
SRCS=pebble.c \
	 common.c \
	 kthparser.c \
	 dag.c \
     dsbasic.c \
     hashtable.c \
     timedflags.c \
     statistics.c 

OBJS=$(SRCS:.c=.o)

# Source files which compilation depends on the pebbling variant
SRCS_V=bfs.c \
	   pebbling.c \
       config.c

OBJS_V=$(SRCS_V:.c=.o)


.PHONY: all clean clean check-syntax tags


all: $(TARGET)


exposetypes: exposetypes.c
	@-$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: $(OBJS)
	@-echo "Pebbling tool [$@]"
	@-rm -f $(OBJS_V)
	@-make  $(OBJS_V) BLACK_WHITE_PEBBLING=0 REVERSIBLE_PEBBLING=0
	@-$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+ $(OBJS_V)


bwpebble: $(OBJS)
	@-echo "Black white pebbling tool [$@]"
	@-rm -f $(OBJS_V)
	@-make  $(OBJS_V) BLACK_WHITE_PEBBLING=1 REVERSIBLE_PEBBLING=0
	@-$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+ $(OBJS_V)


revpebble: $(OBJS)
	@-echo "Reversible pebbling tool [$@]"
	@-rm -f $(OBJS_V)
	@-make  $(OBJS_V) BLACK_WHITE_PEBBLING=0 REVERSIBLE_PEBBLING=1
	@$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+ $(OBJS_V)


timedflags.o:timedflags.c
	@-$(CC) ${OPTIMIZATION} ${DEBUG} ${BUILDFLAGS} -Winline -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall -c $< -o $@

test: exposetypes
	./exposetypes


# --------- Default rules -------------------------
clean:
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -fr *.dSYM
	@-rm -f ${TAGFILES}

tags:
	$(TAGS) -I .

check-syntax:
	$(CC) ${CFLAGS} -o - -S ${CHK_SOURCES} >/dev/null

pkg:
	@make clean
	@echo "Packaging $(NAME).$(TIME).tar.gz"
	@cd .. && \
	tar cfz $(NAME).$(TIME).tar.gz $(NAME) 2> /dev/null


# ------- Build rules
%.o: %.c
	@-$(CC) ${CFLAGS} -c $< -o $@

