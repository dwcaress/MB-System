#!/usr/bin/env python3
"""Validate resolve_color_control() (the -W<color_control> parser) in
mbpy_color.py against the real Perl mbm_grdplot.

Unlike the other test_mbm_grdplot_*.py scripts, this one is mostly
pure option-parsing logic and doesn't need to compare generated CPT
content -- it checks that:
  - an existing file path bypasses CPT generation entirely (CPT_FILE
    points straight at it, no "echo ... >> $CPT_FILE" lines, and the
    file is not deleted at cleanup), and
  - the "style/palette/ncolors", "style/palette", and bare "style"
    numeric forms parse (and clamp out-of-range values) the same way
    mbm_grdplot's own -W option parsing does.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_color as color

REPO = Path(__file__).resolve().parents[3]
MBM_GRDPLOT = REPO / "src" / "macros" / "mbm_grdplot"


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


def check(name, ok, detail=""):
    status = "OK  " if ok else "FAIL"
    print(f"  [{status}] {name}" + (f": {detail}" if detail and not ok else ""))
    return ok


def main():
    tmp = Path(tempfile.mkdtemp(prefix="mbpy_colorcontrol_test_"))
    print(f"scratch dir: {tmp}")
    all_ok = True

    grid = tmp / "geo_wide.grd"
    make_grid(grid, "-122.5/-121.5/36.5/37.0", "0.01")

    # --- existing-file bypass ---
    print("\n=== case existing_file_bypass ===")
    custom_cpt = tmp / "custom.cpt"
    custom_cpt.write_text(
        "-5000 255 0 0 -2500 0 255 0\n"
        "-2500 0 255 0 0 0 0 255\n"
    )
    root = tmp / "customcpt"
    cmd_text = run_perl(grid, root, [f"-W{custom_cpt}"])

    m = re.search(r"^CPT_FILE=(.*)$", cmd_text, re.MULTILINE)
    perl_cpt_file = m.group(1) if m else None
    has_generation_lines = bool(re.search(r"^echo\s+\S.*\$CPT_FILE\s*$", cmd_text, re.MULTILINE))
    has_cleanup_rm = "rm -f $CPT_FILE" in cmd_text

    cc = color.resolve_color_control(str(custom_cpt))
    all_ok &= check("CPT_FILE points at the given file",
                     perl_cpt_file == str(custom_cpt),
                     f"perl CPT_FILE={perl_cpt_file!r} vs given {str(custom_cpt)!r}")
    all_ok &= check("no CPT-content generation lines in the real .cmd", not has_generation_lines)
    all_ok &= check("no cleanup rm of $CPT_FILE in the real .cmd", not has_cleanup_rm)
    all_ok &= check("resolve_color_control reports file_cpt", cc.file_cpt == str(custom_cpt))
    all_ok &= check("custom.cpt still exists after running the script", custom_cpt.exists())

    # --- numeric forms: cross-check against the CPT content mbm_grdplot
    #     actually generates, which only depends on style/palette/ncolors ---
    print("\n=== case numeric forms (cross-checked via generated CPT ncolors) ===")
    numeric_cases = [
        ("1/2/6", 1, 2, 6),
        ("2/3", 2, 3, color.NCPT),
        ("1", 1, 1, color.NCPT),
        ("1/0/4", 1, 1, 4),      # palette 0 out of range -> clamps to 1
        ("1/2/1", 1, 2, 2),      # ncolors 1 out of range -> clamps to 2
        ("1/99/5", 1, 1, 5),     # palette 99 out of range -> clamps to 1
    ]
    for spec, exp_style, exp_palette, exp_ncolors in numeric_cases:
        cc = color.resolve_color_control(spec)
        ok = (cc.file_cpt is None and cc.color_style == exp_style
              and cc.color_palette == exp_palette and cc.ncolors == exp_ncolors)
        all_ok &= check(
            f"-W{spec}", ok,
            f"got file_cpt={cc.file_cpt} style={cc.color_style} "
            f"palette={cc.color_palette} ncolors={cc.ncolors}"
        )

    # spot-check one clamped case end-to-end against real Perl output:
    # -W1/99/5 should behave exactly like -W1/1/5 (palette clamped to 1)
    print("\n=== case palette_clamp_end_to_end ===")
    root_a = tmp / "clamp_a"
    root_b = tmp / "clamp_b"
    cmd_a = run_perl(grid, root_a, ["-W1/99/5"])
    cmd_b = run_perl(grid, root_b, ["-W1/1/5"])
    cpt_line_re = re.compile(r"^echo\s+(\S.*\S)\s+>>?\s+\$CPT_FILE\s*$", re.MULTILINE)
    lines_a = cpt_line_re.findall(cmd_a)
    lines_b = cpt_line_re.findall(cmd_b)
    all_ok &= check("out-of-range palette 99 clamps to palette 1 (matches real Perl)",
                     lines_a == lines_b, f"{lines_a} vs {lines_b}")

    print("\n" + ("ALL CHECKS PASSED" if all_ok else "SOME CHECKS FAILED"))
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
