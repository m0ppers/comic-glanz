aa-glanz.adf: aa-glanz
	xdftool aa-glanz.adf format ``aa-glanz`` + write aa-glanz  + boot install + list

aa-glanz: comic-glanz
	/home/mop/projects/Shrinkler/build/native/Shrinkler comic-glanz aa-glanz

comic-glanz: main-amiga.o comic-ranz.o
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc main-amiga.o comic-ranz.o -mcpu=68000 -mcrt=nix13 -lamiga -o comic-glanz

main-amiga.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -c ../../main-amiga.c -o main-amiga.o

comic-ranz.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -c ../../comic-ranz.c -o comic-ranz.o