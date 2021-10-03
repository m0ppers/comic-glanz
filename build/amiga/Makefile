comic-ranz: main-amiga.o comic-ranz.o
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc main-amiga.o comic-ranz.o -mcpu=68000 -lamiga -o comic-ranz

main-amiga.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -c ../../main-amiga.c -o main-amiga.o

comic-ranz.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -c ../../comic-ranz.c -o comic-ranz.o