TARGET=pebble exposetypes

# Config variables
CC=gcc
#C_STANDARD=-std=c89 -ansi
C_STANDARD=-std=c99

CFLAGS=-DDEBUG -g -fno-builtin --pedantic --pedantic-errors -Wall ${C_STANDARD}
LDFLAGS=-g


# ------ DEFAULT RULES --------------
all: ${TARGET}

clean:
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -fr *.dSYM

check-syntax:
	$(CC) ${CFLAGS} -o - -S ${CHK_SOURCES} >/dev/null

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

# ------ CUSTOM RULES ---------------
exposetypes: exposetypes.c
	$(CC) $(LDFLAGS) ${CFLAGS} -o $@  $<

pebble: pebble.o dag.o pebbling.o dsbasic.o hashtable.o

test: exposetypes
	./exposetypes
