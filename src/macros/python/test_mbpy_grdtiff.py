#!/usr/bin/env python3
"""End-to-end test of mbpy_grdtiff: runs the CLI on a synthetic
geographic grid for each color_mode (1-5), executes the generated GMT
modern-mode script, and checks that a GeoTIFF plus .tfw world file is
produced and that no gmt.conf is left behind.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE.parent / "mbpy_grdtiff"


def check(name, ok, detail=""):
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}" + (f": {detail}" if detail and not ok else ""))
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_grdtiff_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid = tmp / "topo.grd"
    subprocess.run(
        ["gmt", "grdmath", "-R-122.5/-122.0/36.5/37.0", "-I0.005",
         "X", "122.25", "ADD", "10", "MUL", "SIN", "Y", "36.75", "SUB", "8", "MUL", "COS", "MUL",
         "1000", "MUL", "3000", "SUB", "=", str(grid)],
        check=True, capture_output=True, text=True,
    )
    intensity = tmp / "int.grd"
    subprocess.run(["gmt", "grdgradient", str(grid), "-A45", "-Nt", f"-G{intensity}", "-fg"],
                   check=True, capture_output=True, text=True)

    cases = {
        "g1": ["-G1"],
        "g2": ["-G2", "--azimuth", "45"],
        "g3": ["-K", str(intensity)],
        "g4": ["-G4"],
        "g5": ["-G5", "-W", "2/4/8"],
        "zb": ["-G1", "-Z", "-3500/-2500/1", "--data-scale", "1.0"],
    }
    for tag, extra in cases.items():
        proc = subprocess.run(
            [sys.executable, str(CLI), "-I", str(grid), "-O", tag, "-X"] + extra,
            cwd=tmp, capture_output=True, text=True,
        )
        ok = proc.returncode == 0
        all_ok &= check(f"{tag}: script runs", ok, proc.stderr[-400:])
        tif = tmp / f"{tag}.tif"
        all_ok &= check(f"{tag}: GeoTIFF written", tif.exists() and tif.stat().st_size > 10000)
        all_ok &= check(f"{tag}: world file written", (tmp / f"{tag}.tfw").exists())
        script = (tmp / f"{tag}_tiff.sh").read_text() if (tmp / f"{tag}_tiff.sh").exists() else ""
        all_ok &= check(f"{tag}: modern mode", "gmt begin" in script and "gmt end" in script
                        and "gmtset" not in script)

    all_ok &= check("no gmt.conf left behind", not (tmp / "gmt.conf").exists())
    print("ALL OK" if all_ok else "SOME CHECKS FAILED")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
