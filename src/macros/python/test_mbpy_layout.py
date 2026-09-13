#!/usr/bin/env python3
"""Validate mbpy_layout.py against the real Perl mbm_grdplot.

For each synthetic test grid and option combination, this:
  1. runs the Perl `mbm_grdplot` macro to generate a `.cmd` shellscript,
  2. extracts the ground-truth MAP_PROJECTION / MAP_SCALE / X_OFFSET /
     Y_OFFSET / basemap tick values from that script,
  3. runs the equivalent computation through mbpy_layout.py,
  4. compares the two numerically.

Requires `gmt` and `perl` on PATH and mbm_grdplot from this checkout.
Not wired into ctest yet -- this is a validation script for the layout
port, run by hand while porting.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout

REPO = Path(__file__).resolve().parents[3]
MBM_GRDPLOT = REPO / "src" / "macros" / "mbm_grdplot"

TOLERANCE = 5e-3  # inches / scale units


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


def extract(cmd_text: str, key: str) -> str:
    m = re.search(rf"^{key}=(.*)$", cmd_text, re.MULTILINE)
    return m.group(1) if m else None


def extract_tick(cmd_text: str) -> str:
    m = re.search(r"-B(\S+) -B\+t", cmd_text)
    return m.group(1) if m else None


def check(name, expected, actual, tol=TOLERANCE):
    try:
        e, a = float(expected), float(actual)
        ok = abs(e - a) <= tol * max(1.0, abs(e))
    except (TypeError, ValueError):
        ok = (expected == actual)
        e, a = expected, actual
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}: perl={e!r} python={a!r}")
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_layout_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    cases = [
        ("geo_wide", "-122.5/-121.5/36.5/37.0", {}),
        ("geo_tall", "-122.2/-122.0/36.0/37.5", {}),
        ("geo_small", "-122.05/-122.0/36.80/36.83", {"inc": "0.0005"}),
    ]

    for name, region, opts in cases:
        grid = tmp / f"{name}.grd"
        make_grid(grid, region, opts.get("inc", "0.01"))

        for pagesize, scale_loc in [("a", "b"), ("a", "l"), ("a", "r"), ("a", "t"), ("b", "b")]:
            root = tmp / f"{name}_{pagesize}_{scale_loc}"
            extra = [f"-P{pagesize}"]
            if scale_loc != "b":
                extra.append(f"-MGF{scale_loc}")
            print(f"\n=== case {name} pagesize={pagesize} scale_loc={scale_loc} ===")
            cmd_text = run_perl(grid, root, extra)

            grid_info = layout.run_grdinfo(str(grid))
            result = layout.compute_layout(grid_info, pagesize=pagesize, scale_loc=scale_loc)

            perl_proj = extract(cmd_text, "MAP_PROJECTION")
            perl_scale = extract(cmd_text, "MAP_SCALE")
            perl_x = extract(cmd_text, "X_OFFSET")
            perl_y = extract(cmd_text, "Y_OFFSET")
            perl_tick = extract_tick(cmd_text)

            all_ok &= check("projection", perl_proj, result.projection)
            all_ok &= check("scale/pars", perl_scale, result.projection_pars)
            all_ok &= check("x_offset", perl_x, result.xoffset)
            all_ok &= check("y_offset", perl_y, result.yoffset)
            all_ok &= check("base_tick", perl_tick, result.base_tick.tick_x)

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
