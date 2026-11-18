#!/usr/bin/env python3
"""Validate mbpy_xyplot's handling of user "x"/"X" -J specs against the
real Perl mbm_xyplot.

mbm_xyplot's (and mbm_histplot's) GetProjection treats only a bare
"xd"/"Xd" as a geographic (degree) axis; any other remainder, such as
"x3.0/-0.05" (separate x and y scales, y axis reversed), "X6/-8" or
"X6i", is a complete user scale/width. mbm_grdplot's GetProjection
instead marks every "x"/"X" spec as "linear", and reusing that for the
xy and histogram plots produced unusable -J values such as
"-Jx3.0/-0.050.2d" and dropped the axis labels.

For each -J spec this:
  1. runs the Perl `mbm_xyplot` macro to generate a `.cmd` shellscript,
  2. extracts MAP_PROJECTION / MAP_SCALE / X_OFFSET / Y_OFFSET and the
     -B axis spec from that script,
  3. runs mbpy_xyplot_script.compute_layout_xy() on the same data,
  4. compares the two.
It also checks mbpy_histplot_script.compute_layout_hist(), which shares
the parser, for the same specs.

Requires `gmt` and `perl` on PATH and mbm_xyplot from this checkout.
Not wired into ctest -- run by hand, like the other test_mbpy_*.py.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout
import mbpy_xyplot_script as xyplot
import mbpy_histplot_script as histplot

REPO = Path(__file__).resolve().parents[3]
MBM_XYPLOT = REPO / "src" / "macros" / "mbm_xyplot"

TOLERANCE = 5e-3  # inches

# sound speed (m/s) vs depth (m), as made by `mbsvplist -S | awk '{print $2,$1}'`
SVP_DATA = "1500 0\n1495 50\n1490 100\n1485 200\n"

SPECS = ["x3.0/-0.05", "x0.3", "X6/-8", "X6i", "X8/6"]


def run_perl(datafile: Path, root: Path, map_scale: str) -> str:
    args = ["perl", str(MBM_XYPLOT), f"-ISc0.05:{datafile.name}", f"-J{map_scale}",
            "-LT:Sound Speed (m/s):Depth (m)", f"-O{root.name}"]
    subprocess.run(args, cwd=datafile.parent, capture_output=True, text=True)
    return (root.parent / f"{root.name}.cmd").read_text()


def extract(cmd_text: str, var: str) -> str:
    m = re.search(rf"^{var}=(\S+)$", cmd_text, re.M)
    return m.group(1) if m else ""


def extract_axes(cmd_text: str) -> str:
    m = re.search(r"(-Bx\S+\+l\"[^\"]*\" -By\S+\+l\"[^\"]*\")", cmd_text)
    return m.group(1) if m else ""


def check(name, expected, got):
    try:
        ok = abs(float(expected) - float(got)) <= TOLERANCE
    except (TypeError, ValueError):
        ok = str(expected) == str(got)
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}: perl={expected} python={got}")
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_xyplot_projection_test_"))
    datafile = tmp / "svp.txt"
    datafile.write_text(SVP_DATA)
    all_ok = True

    # the parser itself
    print("\n=== get_projection_xy ===")
    spec = layout.get_projection_xy("x3.0/-0.05")
    all_ok &= check("x3.0/-0.05 linear", False, spec.linear)
    all_ok &= check("x3.0/-0.05 plot_scale", 3.0, spec.plot_scale)
    all_ok &= check("x3.0/-0.05 projection_pars", "3.0/-0.05", spec.projection_pars)
    spec = layout.get_projection_xy("X6i")
    all_ok &= check("X6i plot_width", 6.0, spec.plot_width)
    spec = layout.get_projection_xy("xd")
    all_ok &= check("xd linear", True, spec.linear)
    spec = layout.get_projection_xy("m1.0")
    all_ok &= check("m1.0 unchanged", layout.get_projection("m1.0"), spec)

    for map_scale in SPECS:
        print(f"\n=== xyplot -J{map_scale} ===")
        root = tmp / f"perl_{map_scale.replace('/', '_')}"
        cmd_text = run_perl(datafile, root, map_scale)
        result = xyplot.compute_layout_xy(
            1485.0, 1500.0, 0.0, 200.0, map_scale=map_scale,
            xlabel="Sound Speed (m/s)", ylabel="Depth (m)",
        )
        all_ok &= check("projection", extract(cmd_text, "MAP_PROJECTION"), result.projection)
        all_ok &= check("scale/pars", extract(cmd_text, "MAP_SCALE"), result.projection_pars)
        all_ok &= check("x_offset", extract(cmd_text, "X_OFFSET"), result.xoffset)
        all_ok &= check("y_offset", extract(cmd_text, "Y_OFFSET"), result.yoffset)
        all_ok &= check("axes", extract_axes(cmd_text), result.axes_interval)

        print(f"=== histplot -J{map_scale} ===")
        hist = histplot.compute_layout_hist(
            1485.0, 1500.0, 0.0, 200.0, map_scale=map_scale,
            xlabel="Sound Speed (m/s)", ylabel="Frequency %",
        )
        all_ok &= check("scale/pars", map_scale[1:], hist.projection_pars)
        all_ok &= check("labeled axes", True, "+l\"Frequency %\"" in hist.axes_interval)

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
