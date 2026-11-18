#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_grdtiff_script.py
#
#    Copyright (c) 2026 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Purpose:
#   Script generator for mbpy_grdtiff, the Python port of the Perl
#   macro mbm_grdtiff. Builds the color palette (via mbpy_color.py) and
#   writes a GMT **modern mode** shellscript that runs MB-System's own
#   `gmt mbgrdtiff` supplement module to make a georeferenced TIFF
#   (GeoTIFF plus .tfw world file) of a grid, colored by a CPT and
#   optionally shaded (color_mode 1-5, the same modes as mbm_grdtiff).
#
#   mbgrdtiff writes its image file directly rather than adding a layer
#   to a figure, so the script opens a figure-less session (`gmt begin`
#   with no figure name) only so that `gmt set` defaults stay inside the
#   session rather than in a gmt.conf in the current directory (which
#   mbm_grdtiff had to delete before and after). Confirmed with GMT
#   6.7.0: no gmt.conf and no figure file is left behind.
#
#   The shading pipelines (color_mode 2/3/4/5) are the same as
#   mbm_grdplot's and mbm_grdtiff's, so they are reused from
#   mbpy_grdplot_script.py rather than duplicated here.
#
#   Differences from mbm_grdtiff:
#   - A list of grids given to -I (one per line) is not supported; run
#     mbpy_grdtiff once per grid.
#   - mbm_grdtiff's -MGW (world file) is not offered: mbgrdtiff always
#     writes the .tfw world file.
#   - Discrete palettes (-W2/...) and the histogram color stretch (-S)
#     follow the current mbm_grdplot and mbpy_grdplot, so they differ
#     slightly from mbm_grdtiff, which never received mbm_grdplot's
#     fixes there (mbm_grdplot samples ncolors + 1 palette colors for
#     discrete palettes and runs grdhisteq with ncolors bins).
#   - The sealevel palettes 8 and 9 of mbpy_grdplot are available too.
#   - The image is not displayed after it is made (as in the other
#     mbpy_* macros).
#
import shlex
import subprocess
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import mbpy_color as color
from mbpy_color import CptLine
from mbpy_grdplot_script import (
    ShadeOptions,
    _illumination_commands,
    _intensity_commands_mode3,
    _slope_commands,
    resolve_shade_defaults,
    write_cpt_file,
)

COLOR_MODE_NAMES = {
    1: "Color Fill",
    2: "Color Shaded Relief",
    3: "Color Shaded by Intensity File",
    4: "Color Fill of Slope Magnitude",
    5: "Color Shaded by Slope Magnitude",
}


def grid_zrange(grid: str, region: str) -> Tuple[float, float]:
    """z minimum and maximum of `grid` inside `region` ("W/E/S/N"), as
    mbm_grdtiff gets them from mbm_grdinfo -R when -R is given, so the
    color stretch spans the imaged part of the grid only."""
    out = subprocess.run(
        ["gmt", "grdinfo", "-C", grid, f"-R{region}"],
        capture_output=True, text=True, check=True, encoding="utf-8", errors="replace",
    ).stdout.split()
    # -C columns: name w e s n z0 z1 dx dy nx ny ...
    return float(out[5]), float(out[6])


def compute_contour_int(zmin: float, zmax: float) -> float:
    """Port of mbm_grdtiff's "reasonable color intervals" base step
    (the contour interval mbm_grdplot uses too), from which
    mbpy_color.compute_color_interval() derives the color interval."""
    import math
    dzz = zmax - zmin
    contour_int = 0.0
    if dzz > 0:
        base = int((math.log(dzz) / math.log(10.0)) + 0.5)
        contour_int = (10 ** base) / 10.0
        if dzz / contour_int < 10:
            contour_int = contour_int / 4
        elif dzz / contour_int < 20:
            contour_int = contour_int / 2
    return contour_int


def build_cpt(
    grid: str,
    color_mode: int,
    cc: color.ColorControl,
    zmin: float,
    zmax: float,
    zmode: int = 0,
    zmin_t: Optional[float] = None,
    zmax_t: Optional[float] = None,
    color_flip: bool = False,
    no_nice_interval: bool = False,
    stretch_color: bool = False,
    magnitude: Optional[float] = None,
    data_scale: Optional[float] = None,
) -> List[CptLine]:
    """Build the CPT for mbgrdtiff, following mbm_grdtiff's -W/-S/-Y/-Z
    dispatch (mbm_grdtiff lines ~700-1150) with the shared mbpy_color
    helpers. `zmin`/`zmax` are the (possibly -Z overridden, already
    data_scale-rescaled) z bounds used for the color stretch;
    `zmin_t`/`zmax_t` are the data's own extremes, which zmode 1 uses
    to extend the end segments so no data value falls outside the CPT.
    `cc` must not name an existing CPT file (the caller uses that file
    directly instead).
    """
    dzz = zmax - zmin
    ncolors_use = color.get_ncolors_use(cc.ncolors, cc.color_style)
    is_sealevel = cc.color_palette in color.SEALEVEL_PALETTES

    if color_mode == 4:
        mag = magnitude if magnitude is not None else 1.0
        boundaries = color.build_slope_boundaries(mag, ncolors_use, cc.color_style)
        colors = color.interpolate_palette(cc.color_palette, ncolors_use)
        if cc.color_style == 1:
            return color.build_cpt_continuous_from_boundaries(colors, boundaries, color_flip)
        return color.build_cpt_discrete_from_boundaries(colors, cc.ncolors, boundaries, color_flip)

    if stretch_color and is_sealevel:
        nbins = ncolors_use - 1 if cc.color_style == 1 else cc.ncolors
        rows_below, rows_above = color.run_grdhisteq_sealevel(grid, nbins, zmin, zmax)
        below, above = color.build_sealevel_histogram_boundaries(rows_below, rows_above, zmin, zmax, dzz)
        return color.build_cpt_sealevel_from_boundaries(cc.color_palette, below, above, color_flip)

    if stretch_color:
        # grdhisteq runs on the unscaled input grid, so its boundaries
        # are rescaled to the data_scale'd z units the image uses
        rows = color.run_grdhisteq(grid, cc.ncolors)
        if data_scale is not None:
            scaled = ((lo * data_scale, hi * data_scale) for lo, hi in rows)
            rows = sorted((min(lo, hi), max(lo, hi)) for lo, hi in scaled)
        hb = color.build_histogram_boundaries(rows, cc.ncolors, zmin, zmax, dzz, color_style=cc.color_style)
        nsamples = hb.ncolors if cc.color_style == 1 else hb.ncolors + 1
        colors = color.interpolate_palette(cc.color_palette, nsamples)
        if cc.color_style == 1:
            return color.build_cpt_continuous_from_boundaries(colors, hb.hist, color_flip)
        return color.build_cpt_discrete_from_boundaries(colors, hb.ncolors, hb.hist, color_flip)

    contour_int = compute_contour_int(zmin, zmax)
    interval = color.compute_color_interval(dzz, zmin, zmax, ncolors_use, contour_int, no_nice_interval)
    if is_sealevel:
        izero = color.compute_sealevel_izero(
            ncolors_use, interval.color_int, interval.color_start, interval.color_end, color_flip
        )
        cptub, cptue = color.interpolate_palette_sealevel(cc.color_palette, ncolors_use, izero)
        if cc.color_style == 1:
            return color.build_cpt_continuous_sealevel(
                cptub, cptue, interval.color_start, interval.color_int, color_flip, zmode, zmin_t, zmax_t)
        return color.build_cpt_discrete_sealevel(
            cptub, cptue, cc.ncolors, interval.color_start, interval.color_int, color_flip, zmode, zmin_t, zmax_t)
    colors = color.interpolate_palette(cc.color_palette, ncolors_use)
    if cc.color_style == 1:
        return color.build_cpt_continuous(
            colors, interval.color_start, interval.color_int, color_flip, zmode, zmin_t, zmax_t)
    return color.build_cpt_discrete(
        colors, cc.ncolors, interval.color_start, interval.color_int, color_flip, zmode, zmin_t, zmax_t)


@dataclass
class ScriptOptionsTiff:
    grid: str
    root: str
    region: str                          # -R for mbgrdtiff (grid bounds, or the user's -R)
    geographic: bool = True              # grid is in lon/lat: grdgradient gets -fg
    color_mode: int = 1                  # 1-5, as in mbm_grdtiff -G
    shade: Optional[ShadeOptions] = None    # color_mode 2-5
    cpt_lines: Optional[List[CptLine]] = None   # generated CPT, written to <root>.cpt
    cpt_file: Optional[str] = None               # OR: an existing CPT used as-is
    color_flip: bool = False             # swaps COLOR_BACKGROUND/COLOR_FOREGROUND
    data_scale: Optional[float] = None   # rescale the grid before coloring/shading
    nudge: Optional[str] = None          # "dx/dy" meters east/north (mbgrdtiff -N)
    gmt_defs: List[str] = field(default_factory=list)   # user "PARAM/value" overrides

    @property
    def tiff_file(self) -> str:
        return f"{self.root}.tif"

    @property
    def script_path(self) -> str:
        return f"{self.root}_tiff.sh"


def generate_script_tiff(opts: ScriptOptionsTiff, command_line: Optional[str] = None) -> str:
    """Build the GMT modern-mode shellscript text. Does not touch the
    filesystem; see generate_and_write_tiff()."""
    cptfile = opts.cpt_file or f"{opts.root}.cpt"
    q = shlex.quote

    lines = [
        "#!/usr/bin/env bash",
        "#",
        f"# GMT modern-mode script to create a GeoTIFF image of {opts.grid}",
        "# Generated by mbpy_grdtiff (Python port of mbm_grdtiff)",
    ]
    if command_line:
        lines.append(f"#   {command_line}")
    lines += [
        "#",
        "set -e",
        "",
        "# mbgrdtiff writes the image itself; the figure-less session only",
        "# keeps the GMT defaults below out of ./gmt.conf",
        "gmt begin",
        "",
        "# Set temporary GMT defaults",
    ]
    if opts.color_flip:
        lines.append("gmt set COLOR_BACKGROUND white COLOR_FOREGROUND black COLOR_NAN white")
    else:
        lines.append("gmt set COLOR_BACKGROUND black COLOR_FOREGROUND white COLOR_NAN white")
    lines.append("")

    if opts.gmt_defs:
        lines.append("# User-defined GMT parameter overrides")
        pairs = []
        for gmt_def in opts.gmt_defs:
            param, _, value = gmt_def.partition("/")
            pairs.append(f"{param} {value}")
        lines.append(f"gmt set {' '.join(pairs)}")
        lines.append("")

    file_use = q(opts.grid)
    cleanup: List[str] = []
    if opts.data_scale is not None:
        file_use = f"{q(opts.root)}_scaled.grd"
        lines.append("# Rescale data")
        lines.append(f"gmt grdmath {q(opts.grid)} {opts.data_scale:.6g} MUL = {file_use}")
        lines.append("")
        cleanup.append(file_use)

    grid_for_image = file_use
    intensity_file = None
    shade = resolve_shade_defaults(opts.shade) if opts.shade else None
    root = q(opts.root)

    if opts.color_mode == 2:
        cmds, intensity_file = _illumination_commands(
            file_use, root, shade.magnitude, shade.azimuth, shade.elevation, opts.geographic)
        lines.append("# Get shading array")
        lines.extend(cmds)
        lines.append("")
        cleanup.append(intensity_file)
    elif opts.color_mode == 3:
        if not shade or not shade.file_intensity:
            raise ValueError("color_mode 3 requires an intensity file")
        cmds, intensity_file = _intensity_commands_mode3(
            q(shade.file_intensity), root, shade.stretch_shade, shade.magnitude)
        if cmds:
            lines.append("# Get shading array")
            lines.extend(cmds)
            lines.append("")
            cleanup.append(intensity_file)
    elif opts.color_mode == 4:
        cmds, grid_for_image = _slope_commands(file_use, root, opts.geographic, None)
        lines.append("# Get slope array")
        lines.extend(cmds)
        lines.append("")
        cleanup.append(grid_for_image)
    elif opts.color_mode == 5:
        cmds, intensity_file = _slope_commands(file_use, root, opts.geographic, shade.magnitude)
        lines.append("# Get slope array")
        lines.extend(cmds)
        lines.append("")
        cleanup.append(intensity_file)
    elif opts.color_mode != 1:
        raise ValueError(f"unsupported color_mode: {opts.color_mode}")

    parts = [
        f"gmt mbgrdtiff -I{grid_for_image}",
        f"-O{q(opts.tiff_file)}",
        f"-R{opts.region} -Jx1.0",
    ]
    if opts.nudge:
        parts.append(f"-N{opts.nudge}")
    parts.append(f"-C{q(cptfile)}")
    if intensity_file:
        # mbgrdtiff takes the intensity grid as its second -I
        parts.append(f"-I{intensity_file}")
    lines.append("# Make the GeoTIFF image (and its .tfw world file)")
    lines.append(" ".join(parts))
    lines.append("")

    if cleanup:
        lines.append("# Delete surplus files")
        lines.append("rm -f " + " ".join(cleanup))
        lines.append("")

    lines.append("gmt end")
    lines.append("")
    return "\n".join(lines)


def generate_and_write_tiff(opts: ScriptOptionsTiff, command_line: Optional[str] = None) -> str:
    """Write the CPT (when generated) and the shellscript; make the
    script executable and return its path."""
    if opts.cpt_lines is not None and opts.cpt_file is None:
        write_cpt_file(f"{opts.root}.cpt", opts.cpt_lines)
    path = Path(opts.script_path)
    path.write_text(generate_script_tiff(opts, command_line))
    path.chmod(0o755)
    return str(path)
