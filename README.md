# comic-glanz

Amiga OCS executable graphics entry for deadline 2021

3052 bytes

## Building

To build the amiga version you will need bebbos gcc toolchain:

https://github.com/bebbo/amiga-gcc

Setting up cross toolchains with meson and/or cmake was not straightforward
(for example they pass unsupported linker args by default) so I just
added plain Makefiles.

These again are dead simple but don't properly handle changes.

`build/release` contains a SDL version that should properly compile
and run under linux.
`build/amiga` contains the amiga version.

Just run `make -B` to rebuild from scratch all the time
(in either build/release or build/amiga). It won't take too long.

You will have to adjust paths in the Makefiles.

## Creating the intermediate files

The executable is basically a path renderer and gets its info from a prepared SVG.

### Palette

The palette is being extracted from `palette.svg` and outputs `palette_(amiga|sdl).h`.

### Content

The real content is located in `comic-glanz_path.svg`.

To generate glanz.h which is what the path renderer will show execute the following:

```
svgo --config svgo.js -i comic-glanz_path.svg -o comic-glanz_path_opt.svg && ./make_comic_glanz.py && xxd -i glanz.bin glanz.h
```

There will be much console spam (I didn't care :) ).

`svgo` can be obtained from here: https://github.com/svg/svgo

You can edit the original SVG file using inkscape and add new content. However be
aware that you can only use quadratic beziers and every path must have the same
style etc. so that svgo can merge the path. Also the individual segments must
be close together (int8_t) as this is the binary format that is being produced.

You will likely have to manually craft parts of the SVG.
