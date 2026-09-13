#!/usr/bin/env python3
"""End-to-end test of color_mode 6/7 (image plot modes) wired into
mbpy_grdplot_script.py.

Mode 6 needs a real georeferenced image (its own embedded georeferencing
drives placement); mode 7 needs a plain, non-georeferenced image placed
via explicit -R bounds. Both are built here from a synthetic grid via
`gmt grdimage ... -A<file>` rather than checked into the repo.
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


def run_script(script_path: Path) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["bash", str(script_path)], cwd=script_path.parent,
        capture_output=True, text=True,
    )


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_image_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    region = "-122.5/-121.5/36.5/37.0"
    grid_path = tmp / "geo_wide.grd"
    subprocess.run(
        ["gmt", "grdmath", f"-R{region}", "-I0.01", "X", "Y", "MUL", "1000", "MUL", "=", str(grid_path)],
        check=True, capture_output=True, text=True,
    )
    grid_info = layout.run_grdinfo(str(grid_path))
    lay_geo = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")

    interval = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(11, color_style=1), lay_geo.contour_int,
    )
    colors = color.interpolate_palette(1, 11)
    cpt_lines = color.build_cpt_continuous(colors, interval.color_start, interval.color_int)
    cpt_path = tmp / "src.cpt"
    script.write_cpt_file(str(cpt_path), cpt_lines)

    # --- mode 6: a real georeferenced GeoTIFF (embedded georeferencing) ---
    print("\n=== case mode6_embedded_georeferencing ===")
    geotiff_path = tmp / "georef.tif"
    subprocess.run(
        ["gmt", "grdimage", str(grid_path), f"-C{cpt_path}", f"-A{geotiff_path}"],
        check=True, capture_output=True, text=True,
    )
    all_ok &= check("GeoTIFF fixture created", geotiff_path.exists())

    image_info = layout.run_grdinfo(str(geotiff_path))
    lay6 = layout.compute_layout(image_info, pagesize="a", scale_loc="b")
    root6 = tmp / "mode6"
    opts6 = script.ScriptOptions(
        grid=str(geotiff_path), root=str(root6), layout=lay6, grid_info=image_info,
        output_format="png", image_mode=6, title="Embedded Georeferencing",
    )
    script_path6 = root6.with_suffix(".sh")
    script.generate_and_write(opts6, str(script_path6))
    text6 = script_path6.read_text()
    all_ok &= check("no -C or -I flags for grdimage (image, not grid+CPT)",
                     " -C" not in text6 and " -I" not in text6)
    all_ok &= check("no -D/-Dr flag (auto-detect embedded georeferencing)",
                     " -D" not in text6)
    all_ok &= check("no color scale bar (image has no scalar z)", "gmt colorbar" not in text6)

    proc6 = run_script(script_path6)
    all_ok &= check("mode 6 script runs without error", proc6.returncode == 0,
                     detail=f"stdout:\n{proc6.stdout}\nstderr:\n{proc6.stderr}")
    out6 = root6.with_suffix(".png")
    all_ok &= check("mode 6 output PNG produced", out6.exists())
    if out6.exists():
        data = out6.read_bytes()
        all_ok &= check("mode 6 output has a real PNG header", data[:8] == b"\x89PNG\r\n\x1a\n")
        all_ok &= check("mode 6 output is non-trivially sized (>2KB)", len(data) > 2000,
                         detail=f"{len(data)} bytes")

    # --- mode 7: a plain, non-georeferenced image placed via -R ---
    print("\n=== case mode7_specified_bounds ===")
    plain_png_path = tmp / "plain.png"
    subprocess.run(
        ["gmt", "grdimage", str(grid_path), f"-C{cpt_path}", f"-A{plain_png_path}"],
        check=True, capture_output=True, text=True,
    )
    worldfile = plain_png_path.with_suffix(".pgw")
    if worldfile.exists():
        worldfile.unlink()  # strip any georeferencing GMT wrote alongside it
    all_ok &= check("plain PNG fixture created with no world file",
                     plain_png_path.exists() and not worldfile.exists())

    root7 = tmp / "mode7"
    opts7 = script.ScriptOptions(
        # reuse mode 6's own layout/grid_info (not lay_geo/grid_info from
        # the source grid) so both cases render the exact same map
        # extent/size -- any leftover mismatch is then a real content
        # bug (like the grdedit band-collapse this test caught), not
        # just harmless pixel-registration noise between two
        # independently-computed Layouts.
        grid=str(plain_png_path), root=str(root7), layout=lay6, grid_info=image_info,
        output_format="png", image_mode=7, title="Specified Bounds",
    )
    script_path7 = root7.with_suffix(".sh")
    script.generate_and_write(opts7, str(script_path7))
    text7 = script_path7.read_text()
    # mbm_grdplot passes -Dr here, but that hits a real GMT 6.7.0 bug
    # under a non-Cartesian projection (confirmed directly against a
    # real GMT install before writing this workaround: identical
    # "gmt_img_project: Input image does not have sufficient (2)
    # padding" error mode 6's own -D fix was for) -- so this generator
    # instead pre-attaches -R as if it were real georeferencing via
    # `gdal_translate -a_ullr` (NOT `gmt grdedit`, which was tried first
    # and rejected: confirmed directly that it silently collapses a
    # 3-band RGB image to one grayscale band, which grdimage then
    # renders through its own default rainbow CPT instead of the
    # image's real colors), then treats the result like mode 6 (no
    # -D/-Dr at all needed once the image "has" georeferencing).
    all_ok &= check("no -Dr (worked around instead, see comment)", "-Dr" not in text7)
    all_ok &= check("gdal_translate present (the -Dr workaround)", "gdal_translate" in text7)
    all_ok &= check("no color scale bar", "gmt colorbar" not in text7)

    proc7 = run_script(script_path7)
    all_ok &= check("mode 7 script runs without error", proc7.returncode == 0,
                     detail=f"stdout:\n{proc7.stdout}\nstderr:\n{proc7.stderr}")
    out7 = root7.with_suffix(".png")
    all_ok &= check("mode 7 output PNG produced", out7.exists())
    if out7.exists():
        data = out7.read_bytes()
        all_ok &= check("mode 7 output has a real PNG header", data[:8] == b"\x89PNG\r\n\x1a\n")
        all_ok &= check("mode 7 output is non-trivially sized (>2KB)", len(data) > 2000,
                         detail=f"{len(data)} bytes")
    leftover_georef = list(tmp.glob("mode7_georef.tif"))
    all_ok &= check("temporary re-georeferenced image cleaned up", not leftover_georef,
                     detail=str(leftover_georef))

    # mode 6 and mode 7 render the identical underlying colorized data
    # (same grid, same CPT, same region/projection) through two
    # different code paths -- their output should match closely. This
    # is what actually would have caught the grdedit band-collapse bug:
    # that render was structurally fine (ran, produced a properly
    # sized/placed PNG) but silently wrong in content.
    print("\n=== case mode6_vs_mode7_pixel_match ===")
    if out6.exists() and out7.exists():
        try:
            from PIL import Image
            import numpy as np
            a = np.array(Image.open(out6).convert("RGB"), dtype=int)
            b = np.array(Image.open(out7).convert("RGB"), dtype=int)
            same_shape = a.shape == b.shape
            all_ok &= check("mode 6 and mode 7 outputs are the same size", same_shape,
                             detail=f"{a.shape} vs {b.shape}")
            if same_shape:
                mean_diff = np.abs(a - b).mean()
                all_ok &= check("mode 6 and mode 7 outputs match closely (mean diff < 5)",
                                 mean_diff < 5, detail=f"mean diff = {mean_diff}")
        except ImportError:
            print("  [SKIP] Pillow/numpy not available; skipping pixel comparison")

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
