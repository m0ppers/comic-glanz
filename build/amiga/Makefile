aa-glanz.adf: aa-glanz
	xdftool aa-glanz.adf format ``aa-glanz`` + write aa-glanz  + boot install + makedir s + write startup-sequence s/startup-sequence + list

aa-glanz: comic-glanz
	/home/mop/projects/Shrinkler/build/native/Shrinkler comic-glanz aa-glanz

comic-glanz: main-amiga.o comic-glanz.o
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc main-amiga.o comic-glanz.o -mcpu=68000 -mcrt=nix13 -lamiga -o comic-glanz

main-amiga.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -O2 -fomit-frame-pointer -c ../../main-amiga.c -o main-amiga.o

comic-glanz.o:
	/home/mop/amiga-toolchain/bin/m68k-amigaos-gcc -Os -fomit-frame-pointer -c ../../comic-glanz.c -o comic-glanz.o