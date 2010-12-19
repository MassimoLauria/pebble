# Copyright (C) 2010 by >>>NAME<<< <>>>EMAIL<<<>
#
# Created   : ">>>TIME<<< >>>NAME<<<"
# Time-stamp: "2010-12-19, domenica 17:02 (CET) Massimo Lauria"

# ---------- Environment variables ------------
#
CC=gcc
#CC=cc

C_STANDARD=-std=c99
#C_STANDARD=-ansi
#C_STANDARD=-std=c89

TAGS=gtags
#TAGS=etags
#TAGS=ctags
TAGFILES=GPATH GRTAGS GSYMS GTAGS tags TAGS ID

CFLAGS=-fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=

DEBUG=-DDEBUG -g
#DEBUG=

# --------- Project dependent rules ---------------
TARGET=pebble exposetypes
all: tags ${TARGET}


exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: pebble.o dag.o pebbling.o dsbasic.o hashtable.o

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

