


all:
	gcc -ansi -pedantic -Wall -o pebble pebble.c 


check-syntax:
	gcc -o nul -S ${CHK_SOURCES}


clean:
	rm pebble nul