# Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>
#
# Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
# Time-stamp: "2011-01-09, domenica 18:07 (CET) Massimo Lauria"

# ---------- Environment variables ------------
#
DEBUG=-DDEBUG -g
OPTIMIZATION=

#DEBUG=
#OPTIMIZATION=-O3

CC=gcc
#CC=cc

C_STANDARD=-std=c99
#C_STANDARD=-ansi
#C_STANDARD=-std=c89


TAGS=gtags
#TAGS=etags
#TAGS=ctags
TAGFILES=GPATH GRTAGS GSYMS GTAGS tags TAGS ID

CFLAGS=${OPTIMIZATION} ${DEBUG} -fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=


# --------- Project dependent rules ---------------
TARGET=pebble exposetypes
all: ${TARGET}


exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: pebble.o common.o dag.o pebbling.o dsbasic.o hashtable.o

test: exposetypes
	./exposetypes


# --------- Default rules -------------------------
clean:
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -fr *.dSYM
	@-rm -f ${TAGFILES}

tags:
	$(TAGS) -I

check-syntax:
	$(CC) ${CFLAGS} -o - -S ${CHK_SOURCES} >/dev/null

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

