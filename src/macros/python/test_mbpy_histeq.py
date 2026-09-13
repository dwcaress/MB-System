#!/usr/bin/env python3
"""Validate the histogram-equalization / slope-magnitude CPT logic in
mbpy_color.py against the real Perl mbm_grdplot.

Same pattern as test_mbpy_color.py: generate a grid, run the
real Perl macro with -S (real grdhisteq stretch) or -G4 (slope
magnitude fill), extract the CPT echo lines it writes, and compare
against the Python port line-for-line.
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
        print(f"      perl:   {perl_lines}")
        print(f"      python: {py_lines}")
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_histeq_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grids = [
        ("geo_wide", "-122.5/-121.5/36.5/37.0", "0.01"),
        ("geo_tall", "-122.2/-122.0/36.0/37.5", "0.01"),
    ]

    for grid_name, region, inc in grids:
        grid = tmp / f"{grid_name}.grd"
        make_grid(grid, region, inc)
        grid_info = layout.run_grdinfo(str(grid))
        zmin, zmax = grid_info.zmin, grid_info.zmax
        dzz = zmax - zmin

        # --- real histogram-equalized stretch (-S1) ---
        for ncolors, palette, flip, label in [
            (11, 1, False, "stretch_default"),
            (11, 1, True, "stretch_flip"),
            (6, 2, False, "stretch_palette2_n6"),
            (20, 4, False, "stretch_palette4_n20"),
        ]:
            case = f"{grid_name}/{label}"
            print(f"\n=== case {case} ===")
            extra = [f"-W1/{palette}/{ncolors}", "-S1"]
            if flip:
                extra.append("-D1")
            root = tmp / f"histeq_{grid_name}_{label}"
            cmd_text = run_perl(grid, root, extra)
            perl_lines = extract_cpt_lines(cmd_text)

            rows = color.run_grdhisteq(str(grid), ncolors)
            hb = color.build_histogram_boundaries(rows, ncolors, zmin, zmax, dzz, color_style=1)
            colors = color.interpolate_palette(palette, hb.ncolors)
            py_lines_obj = color.build_cpt_continuous_from_boundaries(
                colors, hb.hist, color_flip=flip
            )
            py_lines = [l.as_written() for l in py_lines_obj]

            all_ok &= compare(case, perl_lines, py_lines)

        # --- slope magnitude fill (-G4, magnitude via -Amagnitude) ---
        for magnitude, ncolors, label in [
            (1.0, 11, "slope_default"),
            (2.5, 8, "slope_mag2p5_n8"),
        ]:
            case = f"{grid_name}/{label}"
            print(f"\n=== case {case} ===")
            extra = ["-G4", f"-A{magnitude}", f"-W1/1/{ncolors}"]
            root = tmp / f"histeq_{grid_name}_{label}"
            cmd_text = run_perl(grid, root, extra)
            perl_lines = extract_cpt_lines(cmd_text)

            boundaries = color.build_slope_boundaries(magnitude, ncolors, color_style=1)
            colors = color.interpolate_palette(1, ncolors)
            py_lines_obj = color.build_cpt_continuous_from_boundaries(
                colors, boundaries, color_flip=False
            )
            py_lines = [l.as_written() for l in py_lines_obj]

            all_ok &= compare(case, perl_lines, py_lines)

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
