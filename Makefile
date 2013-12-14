# Copyright (C) 2010, 2011, 2012, 2013 by Massimo Lauria <lauria.massimo@gmail.com>
#
# Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
# Time-stamp: "2013-12-14, 18:11 (CET) Massimo Lauria"

# ---------- BUILD FLAGS ----------------------
RELEASE=0

BLACK_PEBBLES=1
WHITE_PEBBLES=0 
REVERSIBLE=0

CONFIG_HASHSIZE=0x07FFFF
PRINT_STATS_INTERVAL=30   # set to 0 to disable it

BUILDFLAGS=	-DPRINT_STATS_INTERVAL=${PRINT_STATS_INTERVAL}
BUILDFLAGS+=-DBLACK_PEBBLES=${BLACK_PEBBLES}
BUILDFLAGS+=-DWHITE_PEBBLES=${WHITE_PEBBLES}
BUILDFLAGS+=-DREVERSIBLE=${REVERSIBLE}
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


# DEBUG+=-DHASHTABLE_DEBUG


CC=gcc
#CC=cc

C_STANDARD=-std=c99
#C_STANDARD=-ansi
#C_STANDARD=-std=c89


TAGS=gtags # etags ctags
TAGFILES=GPATH GRTAGS GSYMS GTAGS tags TAGS ID

CFLAGS=${OPTIMIZATION} ${DEBUG} ${PROFILE} ${BUILDFLAGS} -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=${DEBUG} ${PROFILE}


# --------- Project dependent rules ---------------
NAME=pebble
TARGET=bwpebble pebble revpebble exposetypes
TIME=$(shell date +%Y.%m.%d-%H.%M)

SRCS=pebble.c \
	 common.c \
	 kthparser.c \
	 dag.c \
     dsbasic.c \
     hashtable.c \
     timedflags.c \
     statistics.c \
	 pebbling.c \
	 bfs.c

OBJS=$(SRCS:.c=.o)


.PHONY: all clean clean clean_variant check-syntax tags $(TARGET:=_build)


all: $(TARGET:=_build)


exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: $(OBJS)
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+

bwpebble: $(OBJS)
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+

revpebble: $(OBJS)
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+


pebble_build: clean_variant $(OBJS)
	make pebble BLACK_PEBBLES=1 WHITE_PEBBLES=0 REVERSIBLE=0

bwpebble_build: clean_variant $(OBJS)
	make bwpebble BLACK_PEBBLES=1 WHITE_PEBBLES=1 REVERSIBLE=0

revpebble_build: clean_variant $(OBJS)
	make revpebble BLACK_PEBBLES=1 WHITE_PEBBLES=0 REVERSIBLE=1


timedflags.o:timedflags.c
	$(CC) ${OPTIMIZATION} ${DEBUG} ${BUILDFLAGS} -Winline -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall -c $< -o $@

test: exposetypes
	./exposetypes


# --------- Default rules -------------------------
clean:
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -fr *.dSYM
	@-rm -f ${TAGFILES}

clean_variant:
	rm -f bfs.o pebbling.o


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
	$(CC) ${CFLAGS} -c $< -o $@

