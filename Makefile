
CFLAGS=-std=c99 -pedantic -pedantic-errors -Werror -Wall



all: pebble exposetypes

pebble: pebble.c
	gcc ${CFLAGS} -o $@  $<

exposetypes: exposetypes.c
	gcc ${CFLAGS} -o $@  $<

check-syntax:
	gcc ${CFLAGS} -S ${CHK_SOURCES} -o - >/dev/null


clean:
	rm pebble exposetypes


test: exposetypes
	./exposetypes