#!/usr/bin/env python3
"""End-to-end test of the swath-navigation/ping-tick overlay
(color_mode-independent -- it layers onto any of them) wired into
mbpy_grdplot_script.py via MB-System's own `gmt mbcontour` GMT
supplement module.

Uses a real (small, "snipped") MB-System swath test data file already
in the repo, referenced through a one-line datalist, exactly the way
mbm_grdplot itself expects -I<swathnavdatalist> to be structured.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_layout as layout
import mbpy_color as color
import mbpy_misc as misc
import mbpy_grdplot_script as script

REPO = Path(__file__).resolve().parents[3]
TESTDATA = REPO / "test" / "utilities" / "testdata" / "mb21" / "TN136HS.309.snipped.mb21"


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
    tmp = Path(tempfile.mkdtemp(prefix="mbm_grdplot_navigation_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    if not TESTDATA.exists():
        print(f"  [SKIP] test data not found: {TESTDATA}")
        return 0

    datalist = tmp / "datalist.mb-1"
    datalist.write_text(f"{TESTDATA} 21\n")

    # a synthetic backdrop grid covering the swath file's own bounds
    region = "-124.51/-124.49/40.83/40.85"
    grid_path = tmp / "backdrop.grd"
    subprocess.run(
        ["gmt", "grdmath", f"-R{region}", "-I0.001", "X", "Y", "MUL", "1000", "MUL", "=", str(grid_path)],
        check=True, capture_output=True, text=True,
    )
    grid_info = layout.run_grdinfo(str(grid_path))
    lay = layout.compute_layout(grid_info, pagesize="a", scale_loc="b")

    interval = color.compute_color_interval(
        grid_info.zmax - grid_info.zmin, grid_info.zmin, grid_info.zmax,
        color.get_ncolors_use(11, color_style=1), lay.contour_int,
    )
    colors = color.interpolate_palette(1, 11)
    cpt_lines = color.build_cpt_continuous(colors, interval.color_start, interval.color_int)

    print("\n=== case resolve_swath_format ===")
    fmt = misc.resolve_swath_format(str(datalist))
    all_ok &= check("auto-detected format is -1 (a datalist)", fmt == "-1", detail=repr(fmt))

    print("\n=== case resolve_navigation_control ===")
    nav_raw = misc.NavigationOptions(swathnavdatalist=str(datalist), navigation_control="50/100")
    nav = misc.resolve_navigation_control(nav_raw, has_swathnavdatalist=True)
    all_ok &= check("2-field form normalized to 4 fields",
                     nav.navigation_control == "50/100/100000/0.15", detail=nav.navigation_control)
    nav_fp = misc.resolve_navigation_control(
        misc.NavigationOptions(swathnavdatalist=str(datalist), navigation_control="FP"),
        has_swathnavdatalist=True,
    )
    all_ok &= check("FP keyword sets name_mode+name_perp and fixed control",
                     nav_fp.name_mode and nav_fp.name_perp
                     and nav_fp.navigation_control == "0.25/1/4/0.15", detail=str(nav_fp))
    nav_ping = misc.resolve_navigation_control(
        misc.NavigationOptions(
            swathnavdatalist=str(datalist), pingnumber_mode=True,
            pingnumber_tick=50, pingnumber_annot=100, pingnumber_tick_len=0.1,
        ),
        has_swathnavdatalist=True,
    )
    all_ok &= check("pingnumber_control rebuilt from tick/annot/ticklen",
                     nav_ping.pingnumber_control == "50/100/0.1", detail=nav_ping.pingnumber_control)

    print("\n=== case full_navigation_overlay ===")
    nav_opts = misc.resolve_navigation_control(
        misc.NavigationOptions(
            swathnavdatalist=str(datalist),
            swathformat=fmt,
            navigation_control="0.25/1/4/0.15",
            pingnumber_mode=True, pingnumber_tick=50, pingnumber_annot=100, pingnumber_tick_len=0.1,
            nav_pen="1p,red",
        ),
        has_swathnavdatalist=True,
    )
    root = tmp / "navplot"
    opts = script.ScriptOptions(
        grid=str(grid_path), root=str(root), layout=lay, grid_info=grid_info,
        output_format="png", cpt_lines=cpt_lines, navigation=nav_opts,
        title="Swath Navigation Overlay",
    )
    script_path = root.with_suffix(".sh")
    script.generate_and_write(opts, str(script_path))

    text = script_path.read_text()
    all_ok &= check("gmt mbcontour present", "gmt mbcontour" in text)
    all_ok &= check("-F-1 (datalist format) present", "-F-1" in text)
    all_ok &= check("-D (navigation ticks) present", "-D0.25/1/4/0.15" in text)
    all_ok &= check("-M (ping numbers) present", "-M50/100/0.1" in text)
    all_ok &= check("-W (nav pen) present", "-W1p,red" in text)

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

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
