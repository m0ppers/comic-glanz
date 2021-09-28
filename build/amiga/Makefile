comic-ranz: main-amiga.o comic-ranz.o
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc main-amiga.o comic-ranz.o -lamiga -o comic-ranz

main-amiga.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -mcpu=68000 -msoft-float -c ../../main-amiga.c -o main-amiga.o

comic-ranz.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -mcpu=68000 -msoft-float -c ../../comic-ranz.c -o comic-ranz.o