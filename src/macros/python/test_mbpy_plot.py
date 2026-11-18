#!/usr/bin/env python3
"""Tests of mbpy_plot: unit checks of the mbm_plot-specific logic in
mbpy_plot_script.py (CPT loops, color interval, contour and shade
controls, bounds expansion), then an end-to-end run of the CLI on the
small swath test file in the repo (navigation, contours and a color
fill through `gmt mbswath`/`gmt mbcontour` in a modern-mode session).
"""
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import mbpy_color as color
import mbpy_plot_script as script

CLI = HERE.parent / "mbpy_plot"
REPO = HERE.parents[2]
TESTDATA = REPO / "test" / "utilities" / "testdata" / "mb21" / "TN136HS.309.snipped.mb21"


def check(name, ok, detail=""):
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}" + (f": {detail}" if detail and not ok else ""))
    return ok


def unit_tests():
    ok = True
    colors = color.interpolate_palette(1, 11)

    # continuous, unflipped: 10 graded segments, shallow (low) = palette start
    lines = script.build_cpt_plot(colors, 1, False, 3500.0, 175.0)
    ok &= check("continuous: 10 segments", len(lines) == 10)
    ok &= check("continuous: low z gets palette start",
                lines[0].format_echo().split()[1:4] == ["255", "255", "255"])
    ok &= check("continuous: contiguous z", all(a.d2 == b.d1 for a, b in zip(lines, lines[1:])))
    flipped = script.build_cpt_plot(colors, 1, True, 3500.0, 175.0)
    ok &= check("continuous flipped: low z gets palette end",
                flipped[0].format_echo().split()[1:4] == ["37", "57", "175"])

    # discrete: ncolors flat segments, the flipped one keeps all of them
    disc = script.build_cpt_plot(colors, 2, True, 0.0, 1.0)
    ok &= check("discrete flipped: 11 flat segments",
                len(disc) == 11 and all(l.r1 == l.r2 for l in disc))

    # mbhistogram boundaries are consumed in order
    hist = [float(v) for v in range(12)]
    ok &= check("stretch boundaries", [l.d1 for l in script.build_cpt_plot(colors, 1, False, boundaries=hist)]
                == hist[:10])

    # mbm_plot's color interval (EM122 depths 3627.7-5232.4 m, 11 colors)
    start, step = script.compute_color_interval_plot(3627.7341, 5232.4238, 5232.4238, 11)
    ok &= check("color interval", (start, step) == (3500.0, 175.0), f"{start} {step}")

    ok &= check("contour default", script.resolve_contour_control(None, 50.0, 2, 11.0)
                == "50/100000/100000/100000/0.011/0.11")
    ok &= check("contour 2 fields", script.resolve_contour_control("25/100", 50.0, 0, 11.0)
                == "25/100/100/100/0.01/0.1")
    ok &= check("contour 6 fields keep label height",
                script.resolve_contour_control("10/50/50/100/0.02/0.2", 50.0, 0, 11.0).endswith("/0.2"))

    sc = script.resolve_shade_control(3, amin=-53.6, amax=-15.0)
    ok &= check("mode 3 default shade from amplitude range", sc.as_arg() == "0.05181/-45.88", sc.as_arg())
    ok &= check("mode 2 shade flip", script.resolve_shade_control(2, shade_flip=True).as_arg() == "2.5/180")

    xmin, xmax, ymin, ymax = script.expand_bounds(-11.8, -11.55, 36.71, 36.88)
    ok &= check("bounds padded", xmin < -11.8 and xmax > -11.55 and ymin < 36.71 and ymax > 36.88)
    ok &= check("lonflip 1", script.apply_lonflip(-10.0, -5.0, 1) == (350.0, 355.0))
    return ok


def cli_tests():
    ok = True
    if not TESTDATA.exists():
        print(f"  [SKIP] test data not found: {TESTDATA}")
        return ok
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_plot_test_"))
    print(f"scratch dir: {tmp}")
    datalist = tmp / "datalist.mb-1"
    datalist.write_text(f"{TESTDATA} 21\n")

    cases = {
        "nav": [],
        "contour": ["-C", "5", "-N", "0.01/0.05"],
        "fill": ["-G1", "--coast", "--coast-resolution", "l"],
        "single": ["-I", str(TESTDATA), "-G2"],
    }
    for tag, extra in cases.items():
        cmd = [sys.executable, str(CLI), "-I", str(datalist), "-O", tag, "--format", "png", "-X"] + extra
        proc = subprocess.run(cmd, cwd=tmp, capture_output=True, text=True)
        ok &= check(f"{tag}: script runs", proc.returncode == 0, proc.stderr[-400:])
        png = tmp / f"{tag}.png"
        ok &= check(f"{tag}: plot written", png.exists() and png.stat().st_size > 5000)
        sh = (tmp / f"{tag}.sh").read_text() if (tmp / f"{tag}.sh").exists() else ""
        ok &= check(f"{tag}: modern mode", "gmt begin" in sh and "-K" not in sh and "psbasemap" not in sh)
    ok &= check("no gmt.conf left behind", not (tmp / "gmt.conf").exists())
    ok &= check("temporary datalist removed", not (tmp / "single_datalist.mb-1").exists())
    return ok


def main():
    ok = unit_tests()
    ok &= cli_tests()
    print("ALL OK" if ok else "SOME CHECKS FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
