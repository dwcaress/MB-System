#!/usr/bin/env python3
"""End-to-end test of mbpy_grdplot_script.py: wire mbpy_layout /
mbpy_color / mbpy_misc together, generate a GMT
modern-mode script, actually run it through a real GMT installation,
and check it produces a valid output file.

Unlike the other test_mbm_grdplot_*.py scripts, this does not compare
against real Perl mbm_grdplot output line-for-line (a modern-mode
script is structurally different from mbm_grdplot's classic-mode one
by design -- see mbpy_grdplot_script.py's module docstring). Instead it
verifies the generated script is actually runnable GMT and produces
sane output.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout
import mbpy_color as color
import mbpy_misc as misc
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
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_grdplot_script_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid_path = tmp / "geo_wide.grd"
    make_grid(grid_path, "-122.5/-121.5/36.5/37.0", "0.01")

    xy_path = tmp / "points.xy"
    xy_path.write_text("-122.0 36.7\n-121.8 36.8\n-121.9 36.6\n")

    grid_info = layout.run_grdinfo(str(grid_path))
    lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")

    ncolors = 11
    interval = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(ncolors, color_style=1), lay.contour_int,
    )
    colors = color.interpolate_palette(1, ncolors)
    cpt_lines = color.build_cpt_continuous(colors, interval.color_start, interval.color_int)

    # --- case 1: full-featured plot (color fill + contour + coast + xy + text + title) ---
    print("\n=== case full_featured ===")
    root1 = tmp / "full"
    opts1 = script.ScriptOptions(
        grid=str(grid_path),
        root=str(root1),
        layout=lay,
        grid_info=grid_info,
        output_format="pdf",
        title="Test Plot",
        cpt_lines=cpt_lines,
        contour=misc.ContourOptions(contour_pen="0.5p,black"),
        coast=misc.CoastOptions(coast_control=True, coast_resolution="i", coast_dryfill="gray"),
        xy_entries=[misc.XYOverlayEntry(
            file=str(xy_path), symbol="c0.1", fill="red", segment="N", segchar=">", pen="N"
        )],
        text_labels=["-122.2/36.6/12/0/1/LB/Monterey Bay"],
    )
    script_path1 = tmp / "full.sh"
    script.generate_and_write(opts1, str(script_path1))

    all_ok &= check("cpt file written", (tmp / "full.cpt").exists())
    all_ok &= check("script file written and executable",
                     script_path1.exists() and (script_path1.stat().st_mode & 0o111) != 0)

    script_text = script_path1.read_text()
    all_ok &= check("modern-mode begin/end framing present",
                     f"gmt begin {root1} pdf" in script_text and script_text.strip().endswith("gmt end"))
    all_ok &= check("no classic-mode -K/-O flags anywhere", " -K" not in script_text and " -O " not in script_text)
    # a real bug once here: PS_PAGE_ORIENTATION LANDSCAPE (copied
    # verbatim from mbm_grdplot's classic-mode baseline) rotated modern
    # mode's auto-sized output 90 degrees, and PS_MEDIA forced a full
    # physical-page canvas with large blank margins around it -- both
    # confirmed against real mbm_grdplot output before being removed
    # (see baseline_gmt_defaults()'s docstring).
    all_ok &= check("no PS_PAGE_ORIENTATION (rotates modern-mode output)",
                     "PS_PAGE_ORIENTATION" not in script_text)
    all_ok &= check("no PS_MEDIA (forces an oversized canvas in modern mode)",
                     "PS_MEDIA" not in script_text)
    all_ok &= check("grdimage + colorbar present", "gmt grdimage" in script_text and "gmt colorbar" in script_text)
    # a real bug once here: `gmt basemap`'s bare -B<interval> (no axes-side
    # selector) only annotates whichever frame sides grdimage's earlier
    # implicit frame hadn't already claimed, in GMT modern mode -- silently
    # dropping lat/lon tick labels from two of the four sides. -BWESN forces
    # all four sides regardless of what ran before it (see _basemap_axes()'s
    # docstring; confirmed by direct comparison against real mbm_grdplot
    # classic-mode reference output, which always annotates all four sides).
    all_ok &= check("basemap forces all four sides annotated (-BWESN)",
                     "gmt basemap" in script_text and "-BWESN" in script_text)
    # a real bug once here: the colorbar -D used both colorscale_offx AND
    # colorscale_offy together, but mbm_grdplot hardcodes whichever one
    # doesn't apply to the bar's orientation to 0 (see module docstring).
    # scale_loc "b" (this case's default) is horizontal: X must be 0.
    m = re.search(r"gmt colorbar \S+ -Dx(\S+?)/(\S+?)\+h", script_text)
    all_ok &= check("horizontal colorbar has X hardcoded to 0",
                     m is not None and m.group(1) == "0", detail=str(m.groups() if m else None))
    all_ok &= check("grdcontour present", "gmt grdcontour" in script_text)
    # a real bug once here: grdcontour was missing -C entirely, so every
    # contour silently fell back to GMT's own auto levels instead of the
    # requested/heuristic interval -- see mbpy_grdplot_script.py's module
    # docstring. Check the flag is actually present with a real value,
    # not just that the command line exists.
    m = re.search(r"gmt grdcontour \S+ -J\S+ -R\S+ -C(\S+)", script_text)
    all_ok &= check("grdcontour has a real -C interval (defaulted from layout.contour_int)",
                     m is not None and float(m.group(1)) == lay.contour_int,
                     detail=f"match={m.group(0) if m else None}, expected contour_int={lay.contour_int}")
    all_ok &= check("coast present with -Di -Ggray", "-Di" in script_text and "-Ggray" in script_text)
    all_ok &= check("xy plot present with -Sc0.1 -Gred", "-Sc0.1" in script_text and "-Gred" in script_text)
    all_ok &= check("text label heredoc present", "gmt text" in script_text and "Monterey Bay" in script_text)

    proc = run_script(script_path1)
    all_ok &= check("script runs without error", proc.returncode == 0,
                     detail=f"stdout:\n{proc.stdout}\nstderr:\n{proc.stderr}")
    outfile = root1.with_suffix(".pdf")
    all_ok &= check("output PDF produced", outfile.exists())
    if outfile.exists():
        data = outfile.read_bytes()
        all_ok &= check("output PDF has a real PDF header", data[:5] == b"%PDF-")
        all_ok &= check("output PDF is non-trivially sized (>5KB)", len(data) > 5000,
                         detail=f"{len(data)} bytes")

    # --- case 2: minimal plot (color fill only, no overlays) ---
    print("\n=== case minimal ===")
    root2 = tmp / "minimal"
    opts2 = script.ScriptOptions(
        grid=str(grid_path), root=str(root2), layout=lay, grid_info=grid_info,
        output_format="png", cpt_lines=cpt_lines,
    )
    script_path2 = tmp / "minimal.sh"
    script.generate_and_write(opts2, str(script_path2))
    proc2 = run_script(script_path2)
    all_ok &= check("minimal script runs without error", proc2.returncode == 0,
                     detail=f"stdout:\n{proc2.stdout}\nstderr:\n{proc2.stderr}")
    outfile2 = root2.with_suffix(".png")
    all_ok &= check("minimal output PNG produced", outfile2.exists())
    if outfile2.exists():
        all_ok &= check("minimal output PNG has a real PNG header",
                         outfile2.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n")

    # --- case 3: explicit contour_interval overrides the layout.contour_int default ---
    print("\n=== case explicit_contour_interval ===")
    root3 = tmp / "explicit_contour"
    opts3 = script.ScriptOptions(
        grid=str(grid_path), root=str(root3), layout=lay, grid_info=grid_info,
        output_format="png", cpt_lines=cpt_lines,
        contour=misc.ContourOptions(), contour_interval="12345",
    )
    script_path3 = tmp / "explicit_contour.sh"
    script.generate_and_write(opts3, str(script_path3))
    text3 = script_path3.read_text()
    all_ok &= check("explicit contour_interval used verbatim", "-C12345" in text3, detail=text3)
    proc3 = run_script(script_path3)
    all_ok &= check("explicit-interval script runs without error", proc3.returncode == 0,
                     detail=f"stdout:\n{proc3.stdout}\nstderr:\n{proc3.stderr}")

    # --- case 4: vertical (scale_loc "l") colorbar -- Y must be hardcoded to 0 ---
    print("\n=== case vertical_colorbar ===")
    lay_left = layout.compute_layout(grid_info, pagesize="a", scale_loc="l")
    interval_left = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(ncolors, color_style=1), lay_left.contour_int,
    )
    cpt_lines_left = color.build_cpt_continuous(colors, interval_left.color_start, interval_left.color_int)
    root4 = tmp / "vertical_colorbar"
    opts4 = script.ScriptOptions(
        grid=str(grid_path), root=str(root4), layout=lay_left, grid_info=grid_info,
        output_format="png", cpt_lines=cpt_lines_left,
    )
    script_path4 = tmp / "vertical_colorbar.sh"
    script.generate_and_write(opts4, str(script_path4))
    text4 = script_path4.read_text()
    m4 = re.search(r"gmt colorbar \S+ -Dx(\S+?)/(\S+?)\+v", text4)
    all_ok &= check("vertical colorbar has Y hardcoded to 0",
                     m4 is not None and m4.group(2) == "0", detail=str(m4.groups() if m4 else None))
    proc4 = run_script(script_path4)
    all_ok &= check("vertical-colorbar script runs without error", proc4.returncode == 0,
                     detail=f"stdout:\n{proc4.stdout}\nstderr:\n{proc4.stderr}")

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
