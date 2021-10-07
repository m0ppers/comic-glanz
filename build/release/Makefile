comic-glanz: main-sdl.o comic-glanz.o
	gcc -g main-sdl.o comic-glanz.o -lm -lc -lSDL2 -o comic-glanz

main-sdl.o:
	gcc -g -I/usr/include/SDL2 -D_REENTRANT -c ../../main-sdl.c -o main-sdl.o

comic-glanz.o:
	gcc -g -c ../../comic-glanz.c -o comic-glanz.o