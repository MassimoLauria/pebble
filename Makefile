# Copyright (C) 2010, 2011, 2012 by Massimo Lauria <lauria.massimo@gmail.com>
#
# Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
# Time-stamp: "2012-05-20, 02:12 (CEST) Massimo Lauria"

# ---------- BUILD FLAGS ----------------------
BLACK_PEBBLES=1
WHITE_PEBBLES=1
PRINT_RUNNING_STATS=0

BUILDFLAGS=	-DBLACK_PEBBLES=${BLACK_PEBBLES} \
			-DWHITE_PEBBLES=${WHITE_PEBBLES} \
			-DPRINT_RUNNING_STATS=${PRINT_RUNNING_STATS}
# ---------- Environment variables ------------
#
# DEBUG=-DDEBUG -g
# PROFILE=-pg
# OPTIMIZATION=

DEBUG=
#PROFILE=-pg
OPTIMIZATION=-O3

# DEBUG+=-DHASHTABLE_DEBUG


CC=gcc
#CC=cc

C_STANDARD=-std=c99
#C_STANDARD=-ansi
#C_STANDARD=-std=c89


TAGS=gtags
#TAGS=etags
#TAGS=ctags
TAGFILES=GPATH GRTAGS GSYMS GTAGS tags TAGS ID

CFLAGS=${OPTIMIZATION} ${DEBUG} ${PROFILE} ${BUILDFLAGS} -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=${DEBUG} ${PROFILE}


# --------- Project dependent rules ---------------
TARGET=pebble exposetypes
all: ${TARGET}


exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: pebble.o \
		common.o \
		dag.o    \
		pebbling.o \
		dsbasic.o  \
		hashtable.o \
		timedflags.o \
		statistics.o \
		bfs.o

timedflags.o:timedflags.c
	$(CC) ${OPTIMIZATION} ${DEBUG} ${BUILDFLAGS} -Winline -finline-functions -fno-builtin --pedantic --pedantic-errors -Wall -c $< -o $@

test: exposetypes
	./exposetypes


# --------- Default rules -------------------------
clean:
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -fr *.dSYM
#	@-rm -f ${TAGFILES}

tags:
	$(TAGS) -I .

check-syntax:
	$(CC) ${CFLAGS} -o - -S ${CHK_SOURCES} >/dev/null

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

