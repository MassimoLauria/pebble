


all:
	gcc -g -ansi -pedantic -pedantic-errors -Werror -Wall -o pebble pebble.c 


check-syntax:
	gcc -o nul -S ${CHK_SOURCES}


clean:
	rm pebble nul