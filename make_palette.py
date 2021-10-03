#!/usr/bin/python

import json
import re
import xml.etree.ElementTree as ET

ns = {'svg': 'http://www.w3.org/2000/svg'}
doc = ET.parse("palette.svg")


def shift_bits(match):
    ehbr = '{:x}'.format(int(match.group(1), 16) >> 1)
    ehbg = '{:x}'.format(int(match.group(2), 16) >> 1)
    ehbb = '{:x}'.format(int(match.group(3), 16) >> 1)
    return '0x' + ehbr + ehbg + ehbb


def convert_to_ehb(color):
    ehb = re.sub("0x([a-f0-9])([a-f0-9])([a-f0-9])", shift_bits, color)

    print("{} => {}".format(color, ehb))
    return ehb


def check_ehbs(color, colors):
    print(color)
    ehb = convert_to_ehb(color)
    print(color, ehb)
    for existing in colors:
        existing_ehb = convert_to_ehb(existing)
        if color == existing_ehb:
            return existing
        if ehb == existing_ehb:
            return existing
    return None


# half black and white first
colors = ["0x111", "0xfff"]
for rect in doc.findall(".//svg:rect", ns):
    style = rect.attrib["style"]
    match = re.search("fill:#([0-9a-f]{6})", style)
    r = match.groups(0)[0][0:2]
    g = match.groups(0)[0][2:4]
    b = match.groups(0)[0][4:6]

    if r[0] != r[1]:
        raise Exception("R channel broken {}".format(match.groups(0)[0]))

    if g[0] != g[1]:
        raise Exception("G channel broken {}".format(match.groups(0)[0]))

    if b[0] != b[1]:
        raise Exception("B channel broken {}".format(match.groups(0)[0]))

    color = "0x{}{}{}".format(r[0], g[0], b[0])
    if color in colors:
        raise Exception("Duplicate color {}".format(color))

    ehb_match = check_ehbs(color, colors)
    if ehb_match is not None:
        raise Exception(
            "Color {} matches computes EHB color value of {}".format(color, ehb_match))

    colors.append(color)

amiga = open('palette_amiga.h', 'w')
sdl = open('palette_sdl.h', 'w')
sdl.write("static uint16_t palette[] = {\n")

register = 0x180
for color in colors:
    amiga.write("0x{:x}, {},\n".format(register, color))
    sdl.write("{},\n".format(color))
    register += 2
sdl.write("};\n")
