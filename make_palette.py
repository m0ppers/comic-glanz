#!/usr/bin/python

import json

f = open('palette.json',)
data = json.load(f)

amiga = open('palette_amiga.h', 'w')
sdl = open('palette_sdl.h', 'w')
sdl.write("static uint16_t palette[] = {\n")
register = 0x180
for color in data:
    amiga.write("0x{:x}, {},\n".format(register, color))
    sdl.write("{},\n".format(color))
    register += 2
sdl.write("};\n")
