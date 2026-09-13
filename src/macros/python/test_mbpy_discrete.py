#!/usr/bin/env python3
"""Validate the discrete (color_style != 1) CPT logic in
mbpy_color.py against the real Perl mbm_grdplot.

Same pattern as the other test_mbm_grdplot_*.py scripts: generate a
grid, run the real Perl macro with -W2 (discrete style) in various
combinations, extract the CPT echo lines it writes, and compare
against the Python port line-for-line -- including the black-segment
quirk from the out-of-range array read (see mbpy_color.py).
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout
import mbpy_color as color

REPO = Path(__file__).resolve().parents[3]
MBM_GRDPLOT = REPO / "src" / "macros" / "mbm_grdplot"

CPT_LINE_RE = re.compile(
    r"^echo\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+>>?\s+\$CPT_FILE\s*$"
)


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


def extract_cpt_lines(cmd_text: str):
    lines = []
    for line in cmd_text.splitlines():
        m = CPT_LINE_RE.match(line.strip())
        if m:
            lines.append(tuple(float(x) for x in m.groups()))
    return lines


def close(a, b, tol=1e-3):
    return abs(a - b) <= tol * max(1.0, abs(a))


def compare(name, perl_lines, py_lines):
    ok = len(perl_lines) == len(py_lines)
    if ok:
        for p, y in zip(perl_lines, py_lines):
            if not all(close(pp, yy) for pp, yy in zip(p, y)):
                ok = False
                break
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}: {len(perl_lines)} perl lines, {len(py_lines)} python lines")
    if not ok:
        for i, (p, y) in enumerate(zip(perl_lines, py_lines)):
            marker = "" if all(close(pp, yy) for pp, yy in zip(p, y)) else "  <-- MISMATCH"
            print(f"      [{i}] perl={p} python={y}{marker}")
        if len(perl_lines) != len(py_lines):
            print(f"      perl:   {perl_lines}")
            print(f"      python: {py_lines}")
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_discrete_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grids = [
        ("geo_wide", "-122.5/-121.5/36.5/37.0", "0.01"),
        ("geo_tall", "-122.2/-122.0/36.0/37.5", "0.01"),
    ]

    arithmetic_cases = [
        # (label, extra perl args, palette, ncolors, flip, no_nice)
        ("discrete_default",      ["-W2"],              1, 11, False, False),
        ("discrete_flip",         ["-W2", "-D1"],       1, 11, True,  False),
        ("discrete_palette2_n6",  ["-W2/2/6"],          2, 6,  False, False),
        ("discrete_palette4_n20", ["-W2/4/20"],         4, 20, False, False),
        ("discrete_no_nice_int",  ["-W2", "-Y"],        1, 11, False, True),
    ]

    for grid_name, region, inc in grids:
        grid = tmp / f"{grid_name}.grd"
        make_grid(grid, region, inc)
        grid_info = layout.run_grdinfo(str(grid))
        lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")
        zmin, zmax = grid_info.zmin, grid_info.zmax
        dzz = zmax - zmin

        for name, extra, palette, ncolors, flip, no_nice in arithmetic_cases:
            case_name = f"{grid_name}/{name}"
            print(f"\n=== case {case_name} ===")
            root = tmp / f"discrete_{grid_name}_{name}"
            cmd_text = run_perl(grid, root, extra)
            perl_lines = extract_cpt_lines(cmd_text)

            ncolors_use = color.get_ncolors_use(ncolors, color_style=2)
            interval = color.compute_color_interval(
                dzz, zmin, zmax, ncolors_use, lay.contour_int, no_nice_color_int=no_nice
            )
            colors = color.interpolate_palette(palette, ncolors_use)
            py_lines_obj = color.build_cpt_discrete(
                colors, ncolors, interval.color_start, interval.color_int, color_flip=flip
            )
            py_lines = [l.as_written() for l in py_lines_obj]

            all_ok &= compare(case_name, perl_lines, py_lines)

        # discrete + real histogram-equalized stretch
        for ncolors, palette, flip, label in [
            (11, 1, False, "discrete_stretch_default"),
            (11, 1, True, "discrete_stretch_flip"),
            (6, 2, False, "discrete_stretch_palette2_n6"),
        ]:
            case_name = f"{grid_name}/{label}"
            print(f"\n=== case {case_name} ===")
            extra = [f"-W2/{palette}/{ncolors}", "-S1"]
            if flip:
                extra.append("-D1")
            root = tmp / f"discrete_{grid_name}_{label}"
            cmd_text = run_perl(grid, root, extra)
            perl_lines = extract_cpt_lines(cmd_text)

            rows = color.run_grdhisteq(str(grid), ncolors)
            hb = color.build_histogram_boundaries(rows, ncolors, zmin, zmax, dzz, color_style=2)
            colors = color.interpolate_palette(palette, hb.ncolors + 1)
            py_lines_obj = color.build_cpt_discrete_from_boundaries(
                colors, hb.ncolors, hb.hist, color_flip=flip
            )
            py_lines = [l.as_written() for l in py_lines_obj]

            all_ok &= compare(case_name, perl_lines, py_lines)

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
