#!/usr/bin/env python3
"""End-to-end test of the shaded-relief / slope-magnitude color modes
(2-5) wired into mbpy_grdplot_script.py.

Same approach as test_mbpy_grdplot_script.py: build up real Layout/CPT
objects, generate a script for each color_mode, run it through a real
GMT installation, and check it produces a valid, non-trivial output
file. This does not (and cannot easily) compare pixel-for-pixel
against mbm_grdplot's own classic-mode output, but the grdgradient/
grdmath pipeline for each mode is a line-for-line port of mbm_grdplot's
own shell commands (see mbpy_grdplot_script.py's helper docstrings), so
a passing run here confirms that pipeline is both syntactically valid
GMT and produces real image content.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout
import mbpy_color as color
import mbpy_grdplot_script as script


def check(name, ok, detail=""):
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}" + (f": {detail}" if detail and not ok else ""))
    return ok


def make_grid(path: Path, region: str, inc: str = "0.01"):
    subprocess.run(
        ["gmt", "grdmath", f"-R{region}", f"-I{inc}", "X", "Y", "MUL", "1000", "MUL", "=", str(path)],
        check=True, capture_output=True, text=True,
    )


def run_script(script_path: Path) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["bash", str(script_path)], cwd=script_path.parent,
        capture_output=True, text=True,
    )


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_shade_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid_path = tmp / "geo_wide.grd"
    make_grid(grid_path, "-122.5/-121.5/36.5/37.0", "0.01")

    grid_info = layout.run_grdinfo(str(grid_path))
    lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")

    ncolors = 11
    interval = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(ncolors, color_style=1), lay.contour_int,
    )
    colors = color.interpolate_palette(1, ncolors)
    normal_cpt = color.build_cpt_continuous(colors, interval.color_start, interval.color_int)

    # mode 4 needs its own slope-magnitude CPT (0..magnitude ramp)
    magnitude4 = 1.0
    slope_boundaries = color.build_slope_boundaries(magnitude4, ncolors, color_style=1)
    slope_cpt = color.build_cpt_continuous_from_boundaries(colors, slope_boundaries, color_flip=False)

    # a synthetic external intensity grid for mode 3
    intensity_path = tmp / "intensity.grd"
    subprocess.run(
        ["gmt", "grdgradient", str(grid_path), "-A45", "-Ne0.6", f"-G{intensity_path}"],
        check=True, capture_output=True, text=True,
    )

    cases = [
        ("mode2_shaded_relief", script.ShadeOptions(color_mode=2), normal_cpt),
        ("mode3_intensity_raw", script.ShadeOptions(
            color_mode=3, file_intensity=str(intensity_path), stretch_shade=False), normal_cpt),
        ("mode3_intensity_stretched", script.ShadeOptions(
            color_mode=3, file_intensity=str(intensity_path), stretch_shade=True), normal_cpt),
        ("mode4_slope_fill", script.ShadeOptions(color_mode=4, magnitude=magnitude4), slope_cpt),
        ("mode5_slope_shaded", script.ShadeOptions(color_mode=5, magnitude=1.0), normal_cpt),
        ("mode2_flipped", script.ShadeOptions(color_mode=2, shade_flip=True), normal_cpt),
    ]

    for label, shade, cpt_lines in cases:
        print(f"\n=== case {label} ===")
        root = tmp / label
        opts = script.ScriptOptions(
            grid=str(grid_path), root=str(root), layout=lay, grid_info=grid_info,
            output_format="png", shade=shade, cpt_lines=cpt_lines,
        )
        script_path = root.with_suffix(".sh")
        script.generate_and_write(opts, str(script_path))

        script_text = script_path.read_text()
        if shade.color_mode == 3:
            all_ok &= check("no gmt grdgradient (mode 3 uses an external intensity file)",
                             "gmt grdgradient" not in script_text)
            all_ok &= check("gmt grdhisteq present iff stretch_shade",
                             ("gmt grdhisteq" in script_text) == shade.stretch_shade)
        else:
            all_ok &= check("gmt grdgradient present", "gmt grdgradient" in script_text)

        proc = run_script(script_path)
        all_ok &= check("script runs without error", proc.returncode == 0,
                         detail=f"stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}")
        outfile = root.with_suffix(".png")
        all_ok &= check("output PNG produced", outfile.exists())
        if outfile.exists():
            data = outfile.read_bytes()
            all_ok &= check("output PNG has a real PNG header", data[:8] == b"\x89PNG\r\n\x1a\n")
            all_ok &= check("output PNG is non-trivially sized (>2KB)", len(data) > 2000,
                             detail=f"{len(data)} bytes")

        # mbm_grdplot deletes the color_mode 4/5 slope grid at the end
        # but -- a real, faithfully-preserved wart, not "fixed" here --
        # never cleans up the color_mode 2/3 intensity (".int") grid,
        # so only check for the former.
        leftover_drv = list(tmp.glob(f"{label}_drvx.grd")) + list(tmp.glob(f"{label}_drvy.grd"))
        all_ok &= check("gradient-component intermediates cleaned up", not leftover_drv,
                         detail=str(leftover_drv))
        leftover_slope = list(tmp.glob(f"{label}_slope.grd"))
        all_ok &= check("slope grid cleaned up if this mode made one", not leftover_slope,
                         detail=str(leftover_slope))

    # verify resolve_shade_defaults() matches mbm_grdplot's own defaulting
    print("\n=== case shade_defaults ===")
    d2 = script.resolve_shade_defaults(script.ShadeOptions(color_mode=2))
    all_ok &= check("mode 2 defaults", (d2.azimuth, d2.magnitude, d2.elevation) == (0.0, 1.0, 30.0),
                     detail=str((d2.azimuth, d2.magnitude, d2.elevation)))
    d3 = script.resolve_shade_defaults(script.ShadeOptions(color_mode=3))
    all_ok &= check("mode 3 default magnitude -0.4", d3.magnitude == -0.4, detail=str(d3.magnitude))
    d4 = script.resolve_shade_defaults(script.ShadeOptions(color_mode=4))
    all_ok &= check("mode 4 default magnitude 1.0", d4.magnitude == 1.0, detail=str(d4.magnitude))
    d2f = script.resolve_shade_defaults(script.ShadeOptions(color_mode=2, shade_flip=True))
    all_ok &= check("shade_flip negates magnitude", d2f.magnitude == -1.0, detail=str(d2f.magnitude))

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
