#!/usr/bin/python
import xml.etree.ElementTree as ET
import re
import json
from array import array

ns = {'svg': 'http://www.w3.org/2000/svg'}
doc = ET.parse("comic-ranz_path.svg")
ranz = open('ranz.bin', 'wb')


def consume_whitespace(d):
    return d.strip()


def consume_number(d):
    exp = re.compile("(-?\d+(\.\d+)?)")
    result = exp.match(d)

    if result is None:
        raise "no number found! {}".format(d)

    return d[result.span()[1]:], float(result.group(0))


def consume_comma(d):
    if d[0] != ",":
        raise "Comma expected {}".format(d)
    return d[1:]


def consume_type(d, pos):
    type = d[0]
    if type == "m":
        d = consume_whitespace(d[1:])
        d, x = consume_number(d)
        d = consume_whitespace(d)
        d = consume_comma(d)
        d, y = consume_number(d)

        return d, {"type": "m", "x": x, "y": y}
    elif type == "q":
        qs = []
        while True:
            try:
                d = consume_whitespace(d[1:])
                d, cx = consume_number(d)
                print(cx)
                d = consume_whitespace(d)
                d = consume_comma(d)
                d, cy = consume_number(d)
                d = consume_whitespace(d)
                d, x = consume_number(d)
                d = consume_whitespace(d)
                d = consume_comma(d)
                d, y = consume_number(d)
                qs.append({"cx": cx, "cy": cy, "x": x, "y": y})
            except:
                print("GEIL")
                break
        print("HMM", json.dumps(qs, indent=2))
        return d, {"type": "q", "qs": qs}
    elif type == "z":
        d = d[1:]
        return d, {"type": "z"}
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


def write_path(segments):
    pos = start = {"x": 0, "y": 0}
    for segment in segments:
        if segment["type"] == "m":
            pos["x"] = segment["x"]
            pos["y"] = segment["y"]
            array('B', [ord("m")]).tofile(ranz)
            array('f', [pos["x"], pos["y"]]).tofile(ranz)
        elif segment["type"] == "q":
            array('B', [ord("q")]).tofile(ranz)
            array('B', [len(segment["qs"])]).tofile(ranz)
            for q in segment["qs"]:
                array('f', [q["cx"], q["cy"],
                            q["x"], q["y"]]).tofile(ranz)
                pos["x"] = q["x"]
                pos["y"] = q["y"]
        elif segment["type"] == "z":
            array('B', [ord("l")]).tofile(ranz)
            array('f', [start["x"], start["y"]]).tofile(ranz)
            break


for path in doc.findall(".//svg:path", ns):
    segments = parse_path(path.attrib["d"])
    write_path(segments)
    break
