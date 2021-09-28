comic-ranz: main-sdl.o comic-ranz.o
	gcc main-sdl.o comic-ranz.o -lm -lc -lSDL2 -o comic-ranz

main-sdl.o:
	gcc -O2 -I/usr/include/SDL2 -D_REENTRANT -c ../../main-sdl.c -o main-sdl.o

comic-ranz.o:
	gcc -O2 -c ../../comic-ranz.c -o comic-ranz.o