#!/usr/bin/env python3
"""Validate the color-scale-bar skip condition (mbm_grdplot lines
~2958-2960, ported as mbpy_grdplot_script._show_colorscale()) end to
end: uniform gray/black/white palettes and seismic-profile grids
should draw a color-filled grid with no colorbar; everything else
should keep it.
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
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_grdplot_colorscale_skip_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid_path = tmp / "geo_wide.grd"
    make_grid(grid_path, "-122.5/-121.5/36.5/37.0", "0.01")
    grid_info = layout.run_grdinfo(str(grid_path))
    lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")

    interval = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(11, color_style=1), lay.contour_int,
    )

    def make_opts(label, palette, gridprojected_override=None):
        if palette in color.SEALEVEL_PALETTES:
            izero = color.compute_sealevel_izero(
                11, interval.color_int, interval.color_start, interval.color_end, False
            )
            cptub, cptue = color.interpolate_palette_sealevel(palette, 11, izero)
            cpt_lines = color._write_cpt_continuous_arithmetic(
                cptub, cptue, interval.color_start, interval.color_int, False, 0, None, None
            )
        else:
            colors = color.interpolate_palette(palette, 11)
            cpt_lines = color.build_cpt_continuous(colors, interval.color_start, interval.color_int)
        gi = grid_info
        if gridprojected_override is not None:
            gi = layout.GridInfo(**{**grid_info.__dict__, "gridprojected": gridprojected_override})
        root = tmp / label
        return script.ScriptOptions(
            grid=str(grid_path), root=str(root), layout=lay, grid_info=gi,
            output_format="png", cpt_lines=cpt_lines, color_palette=palette,
        ), root

    cases = [
        # (label, palette, gridprojected_override, expect_colorbar)
        ("palette1_haxby", 1, None, True),
        ("palette4_grayscale", 4, None, True),
        ("palette5_uniform_gray", 5, None, False),
        ("palette6_uniform_black", 6, None, False),
        ("palette7_uniform_white", 7, None, False),
        ("palette8_sealevel", 8, None, True),
        ("palette10_other", 10, None, True),
        ("seismic_profile", 1, 2, False),
    ]

    for label, palette, gp_override, expect_colorbar in cases:
        print(f"\n=== case {label} ===")
        opts, root = make_opts(label, palette, gp_override)
        script_path = root.with_suffix(".sh")
        script.generate_and_write(opts, str(script_path))
        text = script_path.read_text()

        has_colorbar_line = "gmt colorbar" in text
        all_ok &= check(f"_show_colorscale() == {expect_colorbar}",
                         script._show_colorscale(opts) == expect_colorbar,
                         detail=str(script._show_colorscale(opts)))
        all_ok &= check(f"colorbar {'present' if expect_colorbar else 'absent'} in generated script",
                         has_colorbar_line == expect_colorbar)

        proc = run_script(script_path)
        all_ok &= check("script runs without error", proc.returncode == 0,
                         detail=f"stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}")
        outfile = root.with_suffix(".png")
        all_ok &= check("output PNG produced", outfile.exists())

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
