#!/usr/bin/env python3
"""Validate the sealevel dual-colormap CPT logic in
mbpy_color.py against the real Perl mbm_grdplot.

Same pattern as the other test_mbm_grdplot_*.py scripts, but needs a
grid whose z values actually straddle zero (the sealevel palettes'
whole point is a land/sea color split at z == 0), unlike the other
scripts' synthetic grids which are all-negative.
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


def make_grid(path: Path, region: str, expr: list, inc: str = "0.01"):
    subprocess.run(
        ["gmt", "grdmath", f"-R{region}", f"-I{inc}", *expr, "=", str(path)],
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
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_sealevel_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    # z = (lon + 121.9) * 20 straddles zero across the grid's lon range
    grid = tmp / "sealevel.grd"
    make_grid(grid, "-122.5/-121.5/36.5/37.0", ["X", "121.9", "ADD", "20", "MUL"])
    grid_info = layout.run_grdinfo(str(grid))
    lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")
    zmin, zmax = grid_info.zmin, grid_info.zmax
    dzz = zmax - zmin

    cases = [
        # (label, extra perl args, palette, ncolors, style, flip)
        ("continuous_default", ["-W1/8"], 8, 11, 1, False),
        ("continuous_flip",    ["-W1/8", "-D1"], 8, 11, 1, True),
        ("continuous_n16",     ["-W1/8/16"], 8, 16, 1, False),
        ("discrete_default",   ["-W2/8"], 8, 11, 2, False),
        ("discrete_flip",      ["-W2/8", "-D1"], 8, 11, 2, True),
        ("discrete_palette9",  ["-W2/9"], 9, 11, 2, False),
    ]

    for label, extra, palette, ncolors, style, flip in cases:
        print(f"\n=== case {label} ===")
        root = tmp / f"sealevel_{label}"
        cmd_text = run_perl(grid, root, extra)
        perl_lines = extract_cpt_lines(cmd_text)

        ncolors_use = color.get_ncolors_use(ncolors, color_style=style)
        interval = color.compute_color_interval(
            dzz, zmin, zmax, ncolors_use, lay.contour_int, no_nice_color_int=False
        )
        izero = color.compute_sealevel_izero(
            ncolors_use, interval.color_int, interval.color_start, interval.color_end, flip
        )
        cptub, cptue = color.interpolate_palette_sealevel(palette, ncolors_use, izero)

        if style == 1:
            py_lines_obj = color._write_cpt_continuous_arithmetic(
                cptub, cptue, interval.color_start, interval.color_int, flip, 0, None, None
            )
        else:
            py_lines_obj = color._write_cpt_discrete_arithmetic(
                cptub, cptue, ncolors, interval.color_start, interval.color_int, flip, 0, None, None
            )
        py_lines = [l.as_written() for l in py_lines_obj]

        all_ok &= compare(label, perl_lines, py_lines)

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
