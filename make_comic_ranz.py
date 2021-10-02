#!/usr/bin/python
import xml.etree.ElementTree as ET
import re
import json
from array import array

ns = {'svg': 'http://www.w3.org/2000/svg'}
doc = ET.parse("comic-ranz_path_opt.svg")
ranz = open('ranz.bin', 'wb')


def consume_whitespace(d):
    return d.strip()


def consume_number(d):
    exp = re.compile("(-?\d+(\.\d+)?)")
    result = exp.match(d)

    if result is None:
        raise "no number found! {}".format(d)

    return d[result.span()[1]:], int(result.group(0))


def consume_type(d, pos):
    type = d[0]
    print("Processing type: {} {}".format(type, d))
    if type == "M":
        d = d[1:]
        d = consume_whitespace(d)
        d, x = consume_number(d)
        d = consume_whitespace(d)
        d, y = consume_number(d)
        return d, {"type": "M", "x": x, "y": y}
    elif type == "Q":
        qs = []
        d = d[1:]
        while True:
            try:
                d = consume_whitespace(d)
                d, cx = consume_number(d)
                d = consume_whitespace(d)
                d, cy = consume_number(d)
                d = consume_whitespace(d)
                d, x = consume_number(d)
                d = consume_whitespace(d)
                d, y = consume_number(d)
                qs.append({"cx": cx, "cy": cy, "x": x, "y": y})
            except:
                break
        return d, {"type": "Q", "qs": qs}
    elif type == "z":
        d = d[1:]
        return d, {"type": "z"}
    elif type == "L":
        d = d[1:]
        ls = []
        while True:
            try:
                d = consume_whitespace(d)
                d, x = consume_number(d)
                d = consume_whitespace(d)
                d, y = consume_number(d)
                ls.append({"x": x, "y": y})
            except:
                break
        return d, {"type": "L", "ls": ls}
    elif type == "V":
        d = d[1:]
        vs = []
        while True:
            try:
                d = consume_whitespace(d)
                d, y = consume_number(d)
                vs.append(y)
            except:
                break
        return d, {"type": "V", "vs": vs}
    elif type == "H":
        d = d[1:]
        hs = []
        while True:
            try:
                d = consume_whitespace(d)
                d, x = consume_number(d)
                hs.append(x)
            except:
                break
        return d, {"type": "H", "hs": hs}
    else:
        raise Exception("Unknown type {} {}".format(type, d))


def parse_path(d):
    segments = []
    print("GO")
    while len(d) > 0:
        d = consume_whitespace(d)
        d, segment = consume_type(d, {"x": 0, "y": 0})
        segments.append(segment)

    return segments


def amigaaaaa(words):
    ret = []
    for word in words:
        ret.append((word & 0xff00) >> 8)
        ret.append(word & 0xff)
    return ret


def write_path(segments):
    start = {"x": 0, "y": 0}
    for segment in segments:
        if segment["type"] == "M":
            start["x"] = segment["x"]
            start["y"] = segment["y"]
            array('B', [ord("M")]).tofile(ranz)
            array('B', amigaaaaa([segment["x"],
                                  segment["y"]])).tofile(ranz)
        elif segment["type"] == "Q":
            array('B', [ord("Q")]).tofile(ranz)
            array('B', [len(segment["qs"])]).tofile(ranz)
            for q in segment["qs"]:
                array('B', amigaaaaa([q["cx"], q["cy"],
                                      q["x"], q["y"]])).tofile(ranz)
        elif segment["type"] == "z":
            array('B', [ord("L")]).tofile(ranz)
            array('B', [1]).tofile(ranz)
            array('B', amigaaaaa([start["x"],
                                  start["y"]])).tofile(ranz)
        elif segment["type"] == "L":
            array('B', [ord("L")]).tofile(ranz)
            array('B', [len(segment["ls"])]).tofile(ranz)
            for l in segment["ls"]:
                array('B', amigaaaaa([l["x"], l["y"]])).tofile(ranz)
        elif segment["type"] == "H":
            array('B', [ord("H")]).tofile(ranz)
            array('B', [len(segment["hs"])]).tofile(ranz)
            for x in segment["hs"]:
                array('B', amigaaaaa([x])).tofile(ranz)
        elif segment["type"] == "V":
            array('B', [ord("V")]).tofile(ranz)
            array('B', [len(segment["vs"])]).tofile(ranz)
            for y in segment["vs"]:
                array('B', amigaaaaa([y])).tofile(ranz)


def remove_implicit_line_to(d):
    return re.sub("M(\d+) (\d+) (\d+)", r'M\1 \2 L\3', d)


num = 0
for path in doc.findall(".//svg:path", ns):
    d = path.attrib["d"]
    d = remove_implicit_line_to(d)
    segments = parse_path(d)
    write_path(segments)
    num += 1
