#!/usr/bin/env python3
"""Validate mbpy_misc.py's -M sub-option parsing against real
Perl mbm_grdplot output.

Rather than checking every one of the ~28 sub-flags end-to-end (many
need real swath-navigation or georeferenced-image files that aren't
practical to synthesize here), this covers a representative sample
from each category and confirms the downstream GMT command flag mbm_
grdplot actually writes matches what parse_misc() extracted.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_misc as misc

REPO = Path(__file__).resolve().parents[3]
MBM_GRDPLOT = REPO / "src" / "macros" / "mbm_grdplot"


def make_grid(path: Path, region: str, inc: str = "0.01"):
    subprocess.run(
        ["gmt", "grdmath", f"-R{region}", f"-I{inc}", "X", "Y", "MUL", "1000", "MUL", "=", str(path)],
        check=True, capture_output=True, text=True,
    )


def run_perl(grid: Path, root: Path, extra_args=()):
    args = ["perl", str(MBM_GRDPLOT), f"-I{grid}", f"-O{root}", *extra_args]
    proc = subprocess.run(args, cwd=grid.parent, capture_output=True, text=True)
    cmdfile = root.with_suffix(".cmd")
    if not cmdfile.exists():
        raise RuntimeError(f"mbm_grdplot did not produce {cmdfile}\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}")
    return cmdfile.read_text()


def check(name, ok, detail=""):
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}" + (f": {detail}" if detail and not ok else ""))
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_misc_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid = tmp / "geo_wide.grd"
    make_grid(grid, "-122.5/-121.5/36.5/37.0", "0.01")

    # --- general: -MGD (gmt default override) ---
    print("\n=== case general_gmt_default ===")
    root = tmp / "misc_gmt_default"
    cmd_text = run_perl(grid, root, ["-MGDMAP_FRAME_TYPE/plain"])
    opts = misc.parse_misc("GDMAP_FRAME_TYPE/plain")
    all_ok &= check("parsed gmt_defs", opts.general.gmt_defs == ["MAP_FRAME_TYPE/plain"])
    all_ok &= check("gmt gmtset line present in real .cmd",
                     "gmt gmtset MAP_FRAME_TYPE plain" in cmd_text)

    # --- general: -MGO (map origin) ---
    print("\n=== case general_map_origin ===")
    root = tmp / "misc_origin"
    cmd_text = run_perl(grid, root, ["-MGO2.5/3.5"])
    opts = misc.parse_misc("GO2.5/3.5")
    all_ok &= check("parsed xorigin/yorigin", (opts.general.xorigin, opts.general.yorigin) == (2.5, 3.5))
    m = re.search(r"^X_OFFSET=(\S+)\nY_OFFSET=(\S+)", cmd_text, re.MULTILINE)
    all_ok &= check("X_OFFSET/Y_OFFSET match in real .cmd",
                     m is not None and float(m.group(1)) == 2.5 and float(m.group(2)) == 3.5,
                     detail=str(m.groups()) if m else "no match")

    # --- general: -MGT (text label) ---
    print("\n=== case general_text_label ===")
    root = tmp / "misc_text"
    spec = "GT1.0/2.0/12/0/1/LB/Hello World"
    cmd_text = run_perl(grid, root, [f"-M{spec}"])
    opts = misc.parse_misc(spec)
    all_ok &= check("parsed one text label", opts.general.text_labels == ["1.0/2.0/12/0/1/LB/Hello World"])
    all_ok &= check("pstext data line present in real .cmd",
                     "1.0 2.0 12 0 1 LB Hello World" in cmd_text)

    # --- contour: -MCA, -MCW (needs -C to enable contour mode) ---
    print("\n=== case contour_annotation_pen ===")
    root = tmp / "misc_contour"
    cmd_text = run_perl(grid, root, ["-C", "-MCA10000", "-MCWthick,red"])
    opts = misc.parse_misc("CA10000:CWthick,red")
    all_ok &= check("parsed contour_anot_int/contour_pen",
                     (opts.contour.contour_anot_int, opts.contour.contour_pen) == ("10000", "thick,red"))
    all_ok &= check("-A10000 present in real .cmd", "-A10000" in cmd_text)
    all_ok &= check("-Wthick,red present in real .cmd", "-Wthick,red" in cmd_text)

    # --- coast: -MTD, -MTG, -MTW ---
    print("\n=== case coast_resolution_dryfill_pen ===")
    root = tmp / "misc_coast"
    cmd_text = run_perl(grid, root, ["-MTDf", "-MTGtan", "-MTW2p,black"])
    opts = misc.parse_misc("TDf:TGtan:TW2p,black")
    all_ok &= check(
        "parsed coast_resolution/coast_dryfill/coast_pen",
        (opts.coast.coast_resolution, opts.coast.coast_dryfill, opts.coast.coast_pen)
        == ("f", "tan", "2p,black"),
    )
    all_ok &= check("-Df present in real .cmd", "-Df" in cmd_text)
    all_ok &= check("-Gtan present in real .cmd", "-Gtan" in cmd_text)
    all_ok &= check("-W2p,black present in real .cmd", "-W2p,black" in cmd_text)

    # --- xy overlay: -MXG/-MXS/-MXW set, then -MXI to commit an entry ---
    print("\n=== case xy_overlay ===")
    xyfile = tmp / "points.xy"
    xyfile.write_text("-122.0 36.7\n-121.8 36.8\n")
    root = tmp / "misc_xy"
    cmd_text = run_perl(grid, root, [f"-MXGred:XSc0.1:XW1p,blue:XI{xyfile}"])
    opts = misc.parse_misc(f"XGred:XSc0.1:XW1p,blue:XI{xyfile}")
    entry = opts.xy.entries[0] if opts.xy.entries else None
    all_ok &= check(
        "parsed one xy entry with fill/symbol/pen set",
        entry is not None and entry.file == str(xyfile)
        and entry.fill == "red" and entry.symbol == "c0.1" and entry.pen == "1p,blue",
        detail=str(entry),
    )
    all_ok &= check("-Gred present in real .cmd", "-Gred" in cmd_text)
    all_ok &= check("-Sc0.1 present in real .cmd", "-Sc0.1" in cmd_text)
    all_ok &= check("-W1p,blue present in real .cmd", "-W1p,blue" in cmd_text)

    # --- xy overlay: default fill/symbol/pen ("N") are omitted from psxy ---
    print("\n=== case xy_overlay_defaults ===")
    root = tmp / "misc_xy_default"
    cmd_text = run_perl(grid, root, [f"-MXI{xyfile}"])
    opts = misc.parse_misc(f"XI{xyfile}")
    entry = opts.xy.entries[0] if opts.xy.entries else None
    all_ok &= check("parsed one xy entry with defaulted N fields",
                     entry is not None and (entry.fill, entry.symbol, entry.pen) == ("N", "N", "N"),
                     detail=str(entry))
    all_ok &= check("no -G/-S/-W flags for psxy in real .cmd (all defaulted to N)",
                     not re.search(r"gmt psxy.*?\n(?:.*\\\n)*?.*-[GSW]\S", cmd_text))

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
