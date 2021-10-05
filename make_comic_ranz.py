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


def consume_type(d, context):
    type = d[0]
    print("Processing type: {} {}".format(type, d))
    if type == "M":
        d = d[1:]
        d = consume_whitespace(d)
        d, x = consume_number(d)
        d = consume_whitespace(d)
        d, y = consume_number(d)
        context["start_x"] = x
        context["start_y"] = y
        old_x = context["x"]
        old_y = context["y"]
        context["x"] = x
        context["y"] = y
        return d, {"type": "m", "x": x - old_x, "y": y - old_y}
    elif type == "m":
        d = d[1:]
        d = consume_whitespace(d)
        d, x = consume_number(d)
        d = consume_whitespace(d)
        d, y = consume_number(d)
        context["x"] = context["x"] + x
        context["y"] = context["y"] + y
        context["start_x"] = context["x"]
        context["start_y"] = context["y"]
        print("DER START {} {} {} {}".format(
            context["start_x"], context["start_y"], x, y))
        return d, {"type": "m", "x": x, "y": y}
    elif type == "q":
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
                context["x"] = x + context["x"]
                context["y"] = y + context["y"]
                qs.append({"cx": cx, "cy": cy, "x": x, "y": y})
            except:
                break
        return d, {"type": "q", "qs": qs}
    elif type == "z":
        d = d[1:]
        old_x = context["x"]
        old_y = context["y"]
        context["x"] = context["start_x"]
        context["y"] = context["start_y"]
        lx = context["start_x"] - old_x
        ly = context["start_y"] - old_y
        if lx == 0 and ly == 0:
            return d, None
        return d, {"type": "l", "ls": [{"x": lx, "y": ly}]}
    elif type == "l":
        d = d[1:]
        ls = []
        while True:
            try:
                d = consume_whitespace(d)
                d, x = consume_number(d)
                d = consume_whitespace(d)
                d, y = consume_number(d)
                context["x"] = x + context["x"]
                context["y"] = y + context["y"]
                ls.append({"x": x, "y": y})
            except:
                break
        return d, {"type": "l", "ls": ls}
    elif type == "L":
        d = d[1:]
        ls = []
        while True:
            try:
                d = consume_whitespace(d)
                d, x = consume_number(d)
                d = consume_whitespace(d)
                d, y = consume_number(d)
                context["x"] = x
                context["y"] = y
                ls.append({"x": x - context["x"], "y": y - context["y"]})
            except:
                break
        return d, {"type": "l", "ls": ls}
    elif type == "v":
        d = d[1:]
        vs = []
        while True:
            try:
                d = consume_whitespace(d)
                d, y = consume_number(d)
                vs.append(y)
                context["y"] = y + context["y"]
            except:
                break
        return d, {"type": "v", "vs": vs}
    elif type == "h":
        d = d[1:]
        hs = []
        while True:
            try:
                d = consume_whitespace(d)
                d, x = consume_number(d)
                hs.append(x)
                context["x"] = x + context["x"]
            except:
                break
        return d, {"type": "h", "hs": hs}
    else:
        raise Exception("Unknown type {} {}".format(type, d))


def parse_path(d):
    segments = []
    print("GO")
    context = {"start_x": 0, "start_y": 0, "x": 0, "y": 0}
    while len(d) > 0:
        d = consume_whitespace(d)
        d, segment = consume_type(d, context)
        if segment is not None:
            segments.append(segment)

    return segments


def amigaaaaa(words):
    # print(words)
    return words


def write_path(segments):
    start = {"x": 0, "y": 0}
    for segment in segments:
        print(segment)
        if segment["type"] == "m":
            print(segment)
            start["x"] = segment["x"]
            start["y"] = segment["y"]
            array('B', [ord("m")]).tofile(ranz)
            array('b', amigaaaaa([segment["x"],
                                  segment["y"]])).tofile(ranz)
        elif segment["type"] == "q":
            array('B', [ord("q")]).tofile(ranz)
            array('B', [len(segment["qs"])]).tofile(ranz)
            for q in segment["qs"]:
                array('b', amigaaaaa([q["cx"], q["cy"],
                                      q["x"], q["y"]])).tofile(ranz)
        elif segment["type"] == "l":
            array('B', [ord("l")]).tofile(ranz)
            array('B', [len(segment["ls"])]).tofile(ranz)
            for l in segment["ls"]:
                array('b', amigaaaaa([l["x"], l["y"]])).tofile(ranz)
        elif segment["type"] == "h":
            array('B', [ord("h")]).tofile(ranz)
            array('B', [len(segment["hs"])]).tofile(ranz)
            for x in segment["hs"]:
                array('b', amigaaaaa([x])).tofile(ranz)
        elif segment["type"] == "v":
            array('B', [ord("v")]).tofile(ranz)
            array('B', [len(segment["vs"])]).tofile(ranz)
            for y in segment["vs"]:
                array('b', amigaaaaa([y])).tofile(ranz)


def remove_implicit_line_to(d):
    s = re.sub("M(-?\d+)([-|\s]\d+)([-|\s]\d+)", r'M \1 \2 L\3', d)
    s = re.sub("m(-?\d+)([-|\s]\d+)([-|\s]\d+)", r'm \1 \2 l\3', s)
    return s


num = 0
pattern = re.compile('(-?\d+)')
for path in doc.findall(".//svg:path", ns):
    d = path.attrib["d"]
    d = remove_implicit_line_to(d)
    segments = parse_path(d)
    write_path(segments)
    num += 1
