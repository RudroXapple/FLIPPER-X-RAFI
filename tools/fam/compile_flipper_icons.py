#!/usr/bin/env python3
"""Flipper icon compiler — with debug output."""
import io, os, re, sys
from pathlib import Path
from PIL import Image, ImageOps

def png_to_payload(path):
    with Image.open(path) as img:
        with io.BytesIO() as out:
            bw = ImageOps.invert(img.convert("1"))
            bw.save(out, format="XBM")
            xbm = out.getvalue().decode().strip()
    lines = xbm.splitlines()
    width = int(lines[0].split(" ")[2])
    height = int(lines[1].split(" ")[2])
    data = "".join(lines[2:]).replace(" ", "").split("=")[1][1:-2]
    hex_str = data.replace(",", " ").replace("0x", "")
    raw = bytearray.fromhex(hex_str)
    return width, height, "{" + "".join(f"0x{b:02x}," for b in bytearray([0]) + raw) + "}"

def sanitize(name):
    return re.sub(r"[^A-Za-z0-9_]", "_", name)

def load_global():
    h = Path(__file__).resolve().parents[2] / "components/assets/assets_icons.h"
    if not h.exists(): return set()
    return set(re.findall(r"extern const Icon ([A-Za-z0-9_]+);", h.read_text()))

def compile_icons(input_dir, output_dir, filename, debug=False):
    output_dir.mkdir(parents=True, exist_ok=True)
    global_icons = load_global()
    icons = []
    c_path = output_dir / f"{filename}.c"
    h_path = output_dir / f"{filename}.h"
    skipped = {"global": 0, "no_pngs": 0, "mixed_size": 0, "error": 0}

    with c_path.open("w") as f:
        f.write(f'#include "{filename}.h"\n#include <gui/icon_i.h>\n\n')

        for dirpath, dirnames, filenames in os.walk(input_dir):
            dirnames.sort()
            folder = Path(dirpath).name
            if folder.startswith("."): continue

            pngs = sorted([f for f in filenames if f.endswith(".png")])
            if not pngs:
                skipped["no_pngs"] += 1
                continue

            all_frame_style = all(re.match(r"^frame_\d+$", p.rsplit(".", 1)[0]) for p in pngs)
            has_frame_rate = "frame_rate" in filenames

            if debug and ("MainMenu" in dirpath or "NFC" in dirpath):
                print(f"  DEBUG: {dirpath}", file=sys.stderr)
                print(f"    pngs={len(pngs)}, has_frame_rate={has_frame_rate}, all_frame_style={all_frame_style}", file=sys.stderr)

            if has_frame_rate or all_frame_style:
                is_anim = has_frame_rate and len(pngs) > 1
                prefix = "A_" if is_anim else "I_"
                name = prefix + sanitize(folder)
                if name in global_icons:
                    if debug and ("MainMenu" in dirpath): print(f"    SKIP (in global): {name}", file=sys.stderr)
                    skipped["global"] += 1
                    continue

                frames = []
                w = h = None
                ok = True
                for i, p in enumerate(pngs):
                    try:
                        fw, fh, payload = png_to_payload(Path(dirpath) / p)
                    except Exception as e:
                        if debug: print(f"    ERROR processing {p}: {e}", file=sys.stderr)
                        ok = False; break
                    if w is None: w, h = fw, fh
                    elif fw != w or fh != h: ok = False; break
                    sym = f"_{name}_{i}"
                    frames.append(sym)
                    f.write(f"const uint8_t {sym}[] = {payload};\n")
                if not ok or not frames:
                    skipped["mixed_size"] += 1
                    continue
                f.write(f"const uint8_t* const _{name}[] = {{{','.join(frames)}}};\n")
                rate = 0
                if has_frame_rate:
                    try: rate = int((Path(dirpath) / "frame_rate").read_text().strip())
                    except: pass
                f.write(f"const Icon {name} = {{.width={w},.height={h},"
                        f".frame_count={len(frames)},.frame_rate={rate},.frames=_{name}}};\n\n")
                icons.append(name)
                if debug and ("MainMenu" in dirpath): print(f"    ADDED: {name}", file=sys.stderr)
            else:
                for p in pngs:
                    name = "I_" + sanitize(p.rsplit(".", 1)[0])
                    if name in global_icons:
                        skipped["global"] += 1
                        continue
                    try:
                        w, h, payload = png_to_payload(Path(dirpath) / p)
                    except Exception:
                        skipped["error"] += 1
                        continue
                    sym = f"_{name}_0"
                    f.write(f"const uint8_t {sym}[] = {payload};\n")
                    f.write(f"const uint8_t* const _{name}[] = {{{sym}}};\n")
                    f.write(f"const Icon {name} = {{.width={w},.height={h},"
                            f".frame_count=1,.frame_rate=0,.frames=_{name}}};\n\n")
                    icons.append(name)

    with h_path.open("w") as f:
        f.write('#pragma once\n#include <gui/icon.h>\n#include <assets_icons.h>\n\n')
        for name in icons:
            f.write(f"extern const Icon {name};\n")

    if debug:
        print(f"\n  Stats: {skipped}", file=sys.stderr)

    return len(icons)

if __name__ == "__main__":
    n = compile_icons(Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3], debug=True)
    print(f"Compiled {n} icons")
