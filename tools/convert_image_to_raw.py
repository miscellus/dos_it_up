#!/usr/bin/env python3
import sys
from PIL import Image

if len(sys.argv) != 3:
    print("usage: extract_indices.py IN_IMAGE OUT_RAW", file=sys.stderr)
    raise SystemExit(2)

inp, out = sys.argv[1], sys.argv[2]
im = Image.open(inp)

if im.mode != "P":
    print("error: image is not palette (mode 'P')", file=sys.stderr)
    raise SystemExit(1)

with open(out, "wb") as f:
    f.write(im.tobytes())
