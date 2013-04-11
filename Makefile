# Copyright (C) 2010, 2011, 2012, 2013 by Massimo Lauria <lauria.massimo@gmail.com>
#
# Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
# Time-stamp: "2013-04-12, 01:47 (CEST) Massimo Lauria"

# ---------- BUILD FLAGS ----------------------
PRINT_RUNNING_STATS=0

BUILDFLAGS=	-DPRINT_RUNNING_STATS=${PRINT_RUNNING_STATS}
# ---------- Environment variables ------------
#
DEBUG=-g
PROFILE=-pg
OPTIMIZATION=

# DEBUG=-DNDEBUG
# #PROFILE=-pg
# OPTIMIZATION=-O

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
TARGET=bwpebble pebble exposetypes
all: ${TARGET}


exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

bwpebble: pebble.o \
		  common.o \
		  kthparser.o \
		  dag.o	 \
		  dsbasic.o  \
		  hashtable.o \
		  timedflags.o \
		  statistics.o \
		  pebblingbw.o \
		  bfsbw.o
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+

pebble: pebble.o \
		common.o \
		kthparser.o \
		dag.o	 \
		dsbasic.o  \
		hashtable.o \
		timedflags.o \
		statistics.o \
		pebblingb.o \
		bfsb.o
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $+


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

tags:
	$(TAGS) -I .

check-syntax:
	$(CC) ${CFLAGS} -o - -S ${CHK_SOURCES} >/dev/null

bfsbw.o: bfs.c
	$(CC) -DBLACK_PEBBLES=1 -DWHITE_PEBBLES=1 ${CFLAGS} -c $< -o $@

pebblingbw.o: pebbling.c
	$(CC) -DBLACK_PEBBLES=1 -DWHITE_PEBBLES=1 ${CFLAGS} -c $< -o $@

bfsb.o: bfs.c
	$(CC) -DBLACK_PEBBLES=1 -DWHITE_PEBBLES=0 ${CFLAGS} -c $< -o $@

pebblingb.o: pebbling.c
	$(CC) -DBLACK_PEBBLES=1 -DWHITE_PEBBLES=0 ${CFLAGS} -c $< -o $@


%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

