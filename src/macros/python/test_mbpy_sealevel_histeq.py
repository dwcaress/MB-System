#!/usr/bin/env python3
"""Check the histogram-equalized sealevel CPT (palettes 8/9 with
--stretch-color) in mbpy_color.py. mbm_grdplot has no equivalent, so
instead of a Perl cross-check this verifies the properties the CPT must
have on synthetic grids: increasing, contiguous z boundaries covering
the data, a boundary exactly at 0, Haxby colors on the bathymetry side
and land colors on the topography side (swapped with color_flip), bins
shared in proportion to node counts, and the same result whether or
not the grid is decimated first.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_color as color

LAND_COAST = (200, 150, 40)    # palette 8 stop 10
HAXBY_COAST = (255, 186, 133)  # Haxby stop 1


def make_grid(path: Path, offset: float):
    subprocess.run(
        ["gmt", "grdmath", "-R0/1/0/1", "-I0.01", "X", "Y", "ADD", "1000", "MUL",
         str(offset), "ADD", "=", str(path)],
        check=True, capture_output=True, text=True,
    )


def build(grid: Path, zmin: float, zmax: float, nbins: int, flip: bool, max_nodes=None):
    kwargs = {} if max_nodes is None else {"max_nodes": max_nodes}
    rows_below, rows_above = color.run_grdhisteq_sealevel(str(grid), nbins, zmin, zmax, **kwargs)
    below, above = color.build_sealevel_histogram_boundaries(
        rows_below, rows_above, zmin, zmax, zmax - zmin
    )
    return below, above, color.build_cpt_sealevel_from_boundaries(8, below, above, flip)


def check(label, cond):
    print(f"  {'ok  ' if cond else 'FAIL'} {label}")
    return cond


def main() -> int:
    ok = True
    print("allocate_sealevel_bins")
    ok &= check("proportional split", color.allocate_sealevel_bins(10, 780, 220) == (8, 2))
    ok &= check("at least one bin per side", color.allocate_sealevel_bins(10, 999, 1) == (9, 1))
    ok &= check("all below", color.allocate_sealevel_bins(10, 5, 0) == (10, 0))
    ok &= check("all above", color.allocate_sealevel_bins(10, 0, 5) == (0, 10))

    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        for name, offset, zmin, zmax in (("mixed", -1200.0, -1200.0, 800.0),
                                         ("below", -2100.0, -2100.0, -100.0),
                                         ("above", 5.0, 5.0, 2005.0)):
            grid = tmp / f"{name}.nc"
            make_grid(grid, offset)
            for flip in (False, True):
                print(f"{name} grid, flip={flip}")
                below, above, lines = build(grid, zmin, zmax, 10, flip)
                ok &= check("10 segments", len(lines) == 10)
                ok &= check("covers data", lines[0].d1 < zmin and lines[-1].d2 > zmax)
                ok &= check("contiguous and increasing",
                            all(a.d2 == b.d1 for a, b in zip(lines, lines[1:]))
                            and all(l.d1 < l.d2 for l in lines))
                ok &= check("boundary exactly at 0",
                            any(l.d2 == 0.0 for l in lines) or any(l.d1 == 0.0 for l in lines))
                sea, land = (HAXBY_COAST, LAND_COAST) if not flip else (LAND_COAST, HAXBY_COAST)
                if below:
                    ok &= check("below-zero coast color",
                                lines[len(below) - 2].as_written()[5:] == sea)
                if above:
                    ok &= check("above-zero coast color",
                                lines[len(below) - 1 if below else 0].as_written()[1:4] == land)
                if name == "mixed":
                    ok &= check("bins split by node count", (len(below) - 1, len(above) - 1) == (7, 3))

        print("decimated vs. full grid")
        full = build(tmp / "mixed.nc", -1200.0, 800.0, 10, False)[2]
        dec = build(tmp / "mixed.nc", -1200.0, 800.0, 10, False, max_nodes=2600)[2]
        ok &= check("same bin count", len(full) == len(dec))
        ok &= check("boundaries within 2% of range",
                    all(abs(a.d2 - b.d2) < 40.0 for a, b in zip(full, dec)))

    print("\n" + ("ALL CHECKS PASSED" if ok else "SOME CHECKS FAILED"))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
