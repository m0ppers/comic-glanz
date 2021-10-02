comic-ranz: main-sdl.o comic-ranz.o
	gcc -g main-sdl.o comic-ranz.o -lm -lc -lSDL2 -o comic-ranz

main-sdl.o:
	gcc -g -I/usr/include/SDL2 -D_REENTRANT -c ../../main-sdl.c -o main-sdl.o

comic-ranz.o:
	gcc -g -c ../../comic-ranz.c -o comic-ranz.o