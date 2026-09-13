#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_grd3dplot_script.py
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
#   Python port of mbm_grd3dplot's page-layout and script-generation
#   logic, targeting GMT **modern mode** (`gmt begin`/`gmt end`,
#   `gmt grdview`/`gmt coast`/`gmt colorbar`/`gmt basemap`/`gmt text`)
#   instead of mbm_grd3dplot's own classic-mode <root>.cmd generator.
#
#   mbm_grd3dplot shares most of its page-size/projection/color-palette
#   machinery verbatim with mbm_grdplot (confirmed by direct diff of the
#   two Perl sources): GetPageSize, GetProjection, color palette tables,
#   color-interval and CPT-interpolation math, and the color_mode 2/3/4/5
#   shading pipelines are all identical. Those pieces are imported here
#   from mbpy_layout.py / mbpy_color.py / mbpy_grdplot_script.py rather
#   than re-implemented. What genuinely differs for a 3D perspective
#   plot is ported fresh below:
#
#     - a view azimuth/elevation (mbm_grd3dplot's -E, "azimuth/elevation",
#       default "240/30") that both rotates the apparent plot footprint
#       (mbm_grd3dplot lines ~1004-1241: an extra x_axis_rot/y_axis_rot
#       correction folded into the page-fit scale/width search) and
#       selects which two sides of the frame get axis ticks (lines
#       ~1952-1967: NEZ/SEZ/WSZ/WNZ depending on which 90-degree
#       quadrant the azimuth falls in);
#     - a z-axis scale/exaggeration (mbm_grd3dplot's -F/z-scale-via-J,
#       lines ~1243-1286): computed from a geodetic meters-per-degree
#       conversion (the same C1..C7 constants mbm_grd3dplot uses) so
#       that a requested vertical exaggeration comes out correct in the
#       final plot regardless of whether the grid is geographic;
#     - a z-axis tick interval (mbm_grd3dplot's own GetBaseTick, lines
#       ~3031-3120): notably a *reduced* version of mbpy_layout's -- it
#       always uses the geographic degree/minute/second tick table for
#       x/y regardless of the grid's actual projection type (unlike
#       mbpy_layout.get_base_tick(), which branches on gridprojected),
#       plus a new z tick fixed at (zmax-zmin)/5 with no "nice-number"
#       snapping at all. Reproduced as-is (not "fixed") to match
#       mbm_grd3dplot's real behavior -- this looks like an
#       under-developed corner of the original macro (it never received
#       mbm_grdplot's later UTM/seismic/generic-linear handling), but a
#       port should not silently improve on it without being asked.
#     - grdview itself (color_mode 1-5: -Qi<dpi> image rendering, same
#       CPT/-I<shade or slope file> plumbing as 2D grdimage; color_mode
#       6/7: -Qm mesh rendering, with mode 7 adding a contour pen -W
#       directly on the same grdview call -- there is no separate
#       grdcontour step for 3D plots at all).
#
#   NOTE on a real, confirmed-dead option in mbm_grd3dplot itself: the
#   macro defines a -C<contour_control> option and a $contour_mode/
#   $contour_control pair of variables, but -- unlike mbm_grdplot's -C,
#   which drives an actual grdcontour overlay -- grep across the whole
#   source shows $contour_control is only ever read to print a verbose
#   summary line; it is never passed to any GMT module. $contour_mode
#   only affects one thing: whether color_mode defaults to 1 when
#   otherwise unset. In other words, mbm_grd3dplot's "contour" option
#   does not draw contours. This port does not expose a --contour flag
#   at all rather than port a no-op long option; color_mode 7's
#   contour-on-mesh (-MVW / --mesh-contour-pen below) is the only real
#   contour capability mbm_grd3dplot has.
#
#   A known GMT-installation limitation, not a bug in this port or in
#   mbm_grd3dplot: `gmt grdview -Qm -W<pen>` (color_mode 7 -- mesh with
#   contour lines, i.e. --mesh-contour-pen) reliably segfaults inside
#   GMT_grdview under a 3-D perspective (-p) with -Jz active, on at
#   least GMT 6.7.0, regardless of the pen syntax used. Plain -Qm
#   (color_mode 6, no contour pen) works fine. This looks like an
#   upstream GMT bug triggered by the -Qm/-W/-p/-Jz combination; there
#   is no script-side workaround that preserves the actual requested
#   mesh+contour rendering, so it is simply documented here rather than
#   silently avoided.
#
#   A second real, confirmed bug in mbm_grd3dplot itself (not just a
#   dead option): its -M sub-option parser splits the concatenated
#   -M<sub1>:<sub2>:... string on the literal 7-character separator
#   ":::::::" (mbm_grd3dplot line ~355: `split(/:::::::/, $misc)`)
#   while MBGetopts (its own option-concatenation helper) joins repeated
#   -M invocations with a *single* colon. Since a normal -M string never
#   contains 7 consecutive colons, this split never actually splits
#   anything: every -M sub-option after the first on any command line
#   using more than one gets silently absorbed into the first
#   sub-option's own greedy \S+ value instead of being parsed on its own
#   (confirmed against mbm_grd3dplot's real "GLfx.../GQ300/IE300/ITg"
#   -M chain, which is exactly the shape real usage takes -- see
#   src/macros/mbm_grd3dplot's own process.cmd-style callers). This port
#   sidesteps the whole issue by construction: like mbpy_grdplot, it
#   exposes each concept as its own descriptive long option and never
#   builds or re-parses a colon-joined -M string at all.
#
# Status:
#   Covers color_mode 1-5 (color fill / shaded relief / intensity-file
#   shading / slope-magnitude fill / slope-magnitude shading, identical
#   math to mbpy_grdplot_script.py), 6-7 (mesh plot, with or without
#   contour lines), an optional coastline overlay, drape file, null
#   plane, and z-level. Not ported: swath navigation overlay and xy
#   overlay -- mbm_grd3dplot itself has no such options (confirmed: no
#   -MN or -MX sub-option dispatch anywhere in its source), unlike
#   mbm_grdplot. Verified by running the generated script through a
#   real GMT 6 installation; not exhaustively line-by-line diffed
#   against Perl output the way mbpy_grdplot_script.py was (see that
#   module's own Status note for the difference in rigor).
#
import math
from dataclasses import dataclass, field, replace
from pathlib import Path
from typing import List, Optional, Tuple

import mbpy_layout as layout
from mbpy_color import CptLine
from mbpy_layout import GridInfo
from mbpy_misc import CoastOptions
from mbpy_grdplot_script import (
    ShadeOptions,
    resolve_shade_defaults,
    _illumination_commands,
    _intensity_commands_mode3,
    _slope_commands,
    baseline_gmt_defaults,
    write_cpt_file,
)

_DTR = 3.1415926 / 180.0

# Geodetic meters-per-degree conversion constants (mbm_grd3dplot lines
# ~1244-1256), used only to convert a requested vertical --exaggeration
# into a map z-scale (or vice versa) for a geographic grid.
_C1, _C2, _C3 = 111412.84, -93.5, 0.118
_C4, _C5, _C6, _C7 = 111132.92, -559.82, 1.175, 0.0023


def get_base_tick_z(zmin: float, zmax: float) -> str:
    """Port of the z-tick half of mbm_grd3dplot's own GetBaseTick
    (line ~3119): a plain (zmax-zmin)/5, with no "nice number" rounding
    at all -- unlike every other tick interval in either macro."""
    return str((zmax - zmin) / 5.0)


@dataclass
class Layout3D:
    pagesize: str
    scale_loc: str
    projection: str
    projection_pars: str
    region: str
    zmin: float
    zmax: float
    view_azimuth: float
    view_elevation: float
    map_zscale: float
    exaggeration: float
    landscape: bool
    plot_width: float
    plot_height: float
    xoffset: float
    yoffset: float
    degree_format: str
    colorscale_length: float
    colorscale_thick: float
    colorscale_offx: float
    colorscale_offy: float
    colorscale_vh: str
    contour_int: float
    base_tick_xy: str
    base_tick_z: str
    gridprojected: int = 0   # resolved value (see compute_layout_3d()'s docstring
                             # on the out-of-bounds fallback) -- callers building a
                             # GridInfo-consuming ScriptOptions3D from the same grid
                             # should use this instead of the raw GridInfo.gridprojected


def compute_layout_3d(
    grid: GridInfo,
    view_control: str = "240/30",
    pagesize: str = "a",
    scale_loc: str = "b",
    map_scale: Optional[str] = None,
    orientation: int = 0,
    region: Optional[str] = None,
    exaggeration: Optional[float] = None,
    map_zscale: Optional[float] = None,
) -> Layout3D:
    """Port of mbm_grd3dplot's page/scale-fit algorithm (lines ~700-1292):
    the same GetPageSize/GetProjection heuristics mbpy_layout.compute_layout
    uses, plus the view-azimuth footprint-rotation correction and the
    z-scale/exaggeration calculation that are specific to a 3D plot.
    """
    xmin, xmax, ymin, ymax = grid.xmin, grid.xmax, grid.ymin, grid.ymax
    zmin, zmax = grid.zmin, grid.zmax

    # Port of mbm_grd3dplot's own -R handling (mirrors mbm_grdplot's
    # equivalent -- see mbpy_layout.compute_layout()'s docstring on this
    # gap): a user-supplied -R overrides xmin/xmax/ymin/ymax themselves,
    # not just the final plot region string, since they drive the
    # apparent-plot-size/page-fit computation below.
    if region:
        xmin, xmax, ymin, ymax = (float(v) for v in region.split("/")[:4])

    view_azimuth, view_elevation = (float(v) for v in view_control.split("/"))

    margins = layout.get_page_size(pagesize, scale_loc)

    use_scale = use_width = use_ratio = linear = False
    separator = ""
    trial_value = "1.0"
    plot_scale = plot_width = None
    projection = projection_pars = None

    if map_scale:
        spec = layout.get_projection(map_scale)
        projection = spec.projection
        projection_pars = spec.projection_pars
        use_scale, use_width = spec.use_scale, spec.use_width
        use_ratio, linear = spec.use_ratio, spec.linear
        separator, trial_value = spec.separator, spec.trial_value
        plot_scale, plot_width = spec.plot_scale, spec.plot_width

    # Port of mbm_grd3dplot line ~867 (same fallback as mbm_grdplot's own
    # copy -- see mbpy_layout.compute_layout()'s docstring on this): treat
    # the grid as projected, not geographic, if its plot bounds
    # (xmin/xmax/ymin/ymax, already reflecting a -R override above if
    # given) fall outside plausible lon/lat ranges, even when
    # run_grdinfo() didn't recognize its projection metadata as a tagged
    # "UTM Zone".
    gridprojected = grid.gridprojected
    if gridprojected == 0 and (xmin < -360.0 or xmax > 360.0 or ymin < -90.0 or ymax > 90.0):
        gridprojected = 1

    if (use_scale and plot_scale) or (use_width and plot_width):
        pass
    elif use_scale or use_width:
        projection_pars = f"{projection_pars}{separator}{trial_value}"
    elif gridprojected > 0:
        projection, projection_pars = "x", "1.0"
        use_scale, linear = True, True
    else:
        projection, projection_pars = "m", "1.0"
        use_scale = True

    bounds_plot = region or f"{xmin:.11g}/{xmax:.11g}/{ymin:.11g}/{ymax:.11g}"
    dxx, dyy = layout._mapproject_bbox(xmin, xmax, ymin, ymax, projection, projection_pars, bounds_plot)
    dxx, dyy = abs(dxx), abs(dyy)

    # view-azimuth footprint rotation, common to all three scale-fit
    # branches below (mbm_grd3dplot's own "(kluge)" comment on this,
    # line ~1225, is the original author's, not this port's)
    x_axis_rot = abs(dxx * math.cos(_DTR * view_azimuth))
    y_axis_rot = abs(dyy * math.sin(_DTR * view_azimuth))

    landscape = portrait = False
    width = height = 0.0
    width_max = height_max = 0.0

    if (use_scale and plot_scale) or (use_width and plot_width):
        plot_width_factor = (x_axis_rot + y_axis_rot) / dxx
        plot_width_ = plot_width_factor * dxx
        plot_height_ = plot_width_factor * dyy

        if orientation == 1:
            portrait = True
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]
            width_max, height_max = margins.width_max_portrait, margins.height_max_portrait
        elif orientation == 2:
            landscape = True
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
            width_max, height_max = margins.width_max_landscape, margins.height_max_landscape
        elif dxx > dyy:
            landscape = True
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
            width_max, height_max = margins.width_max_landscape, margins.height_max_landscape
        else:
            portrait = True
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]
            width_max, height_max = margins.width_max_portrait, margins.height_max_portrait

        if plot_width_ > width_max or plot_height_ > height_max:
            good_page = None
            for elem in layout.PAGE_SIZE_NAMES:
                m2 = layout.get_page_size(elem, scale_loc)
                wm = m2.width_max_portrait if portrait else m2.width_max_landscape
                hm = m2.height_max_portrait if portrait else m2.height_max_landscape
                if plot_width_ <= wm and plot_height_ <= hm:
                    good_page = elem
                    break
            if good_page is None:
                good_page = pagesize
            pagesize = good_page
            margins = layout.get_page_size(pagesize, scale_loc)
            width = layout.PAGE_WIDTH_IN[pagesize] if portrait else layout.PAGE_HEIGHT_IN[pagesize]
            height = layout.PAGE_HEIGHT_IN[pagesize] if portrait else layout.PAGE_WIDTH_IN[pagesize]

        plot_width, plot_height = plot_width_, plot_height_

    elif use_scale:
        plot_scale_landscape = margins.width_max_landscape / dxx
        if plot_scale_landscape * dyy > margins.height_max_landscape:
            plot_scale_landscape = margins.height_max_landscape / dyy
        plot_scale_portrait = margins.width_max_portrait / dxx
        if plot_scale_portrait * dyy > margins.height_max_portrait:
            plot_scale_portrait = margins.height_max_portrait / dyy

        if orientation == 1:
            portrait = True
            plot_scale = plot_scale_portrait
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]
        elif orientation == 2:
            landscape = True
            plot_scale = plot_scale_landscape
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
        elif plot_scale_landscape > plot_scale_portrait:
            landscape = True
            plot_scale = plot_scale_landscape
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
        else:
            portrait = True
            plot_scale = plot_scale_portrait
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]

        plot_width_raw = dxx * plot_scale
        plot_height_raw = dyy * plot_scale

        plot_scale_factor = dxx / (x_axis_rot + y_axis_rot)
        plot_scale = plot_scale_factor * plot_scale
        plot_width = plot_scale_factor * plot_width_raw
        plot_height = plot_scale_factor * plot_height_raw

        if use_ratio:
            top = int(1 / plot_scale)
            projection_pars = f"{layout._base_pars(projection, map_scale)}{separator}1:{top}"
        else:
            projection_pars = layout._sprintf_g5(layout._base_pars(projection, map_scale), separator, plot_scale)
        if linear and gridprojected == 0:
            projection_pars = f"{projection_pars}d"

    elif use_width:
        plot_width_landscape = min(margins.height_max_landscape * dxx / dyy, margins.width_max_landscape)
        plot_width_portrait = min(margins.height_max_portrait * dxx / dyy, margins.width_max_portrait)

        if orientation == 1:
            portrait = True
            plot_width_raw = plot_width_portrait
        elif orientation == 2:
            landscape = True
            plot_width_raw = plot_width_landscape
        elif plot_width_landscape > plot_width_portrait:
            landscape = True
            plot_width_raw = plot_width_landscape
        else:
            portrait = True
            plot_width_raw = plot_width_portrait

        plot_height_raw = plot_width_raw * dyy / dxx
        if landscape:
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
        else:
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]

        # NOTE: mbm_grd3dplot's own use_width branch (line ~1225-1230)
        # computes "$plots_width_factor" here (with an s) but then goes
        # on to *use* "$plot_width_factor" (without the s) -- a variable
        # from the unrelated use_scale branch above that is undefined on
        # this code path (Perl treats it as 0), which would zero out
        # plot_width_xaxis/plot_height_yaxis entirely. This looks like a
        # genuine bug in mbm_grd3dplot itself, not intentional behavior
        # to reproduce; this port uses the correctly-computed factor.
        plot_width_factor = dxx / (x_axis_rot + y_axis_rot)
        plot_width = plot_width_factor * plot_width_raw
        plot_height = plot_width_factor * plot_height_raw

        projection_pars = layout._sprintf_g5(layout._base_pars(projection, map_scale), separator, plot_width)
        if linear and gridprojected == 0:
            projection_pars = f"{projection_pars}d"

    # z-scale / vertical exaggeration (mbm_grd3dplot lines ~1243-1286)
    radlat = 0.5 * (ymax + ymin) * _DTR
    mtodeglat = 1.0 / abs(_C4 + _C5 * math.cos(2 * radlat) + _C6 * math.cos(4 * radlat) + _C7 * math.cos(6 * radlat))
    mtodeglon = 1.0 / abs(_C1 * math.cos(radlat) + _C2 * math.cos(3 * radlat) + _C3 * math.cos(5 * radlat))
    if map_zscale is None and exaggeration is not None:
        if gridprojected:
            map_zscale = exaggeration * plot_width / (xmax - xmin)
        else:
            map_zscale = exaggeration * plot_width * mtodeglon / (xmax - xmin)
    elif map_zscale is None:
        map_zscale = 2.0 / (zmax - zmin)
    if exaggeration is None:
        if gridprojected:
            exaggeration = map_zscale * (xmax - xmin) / plot_width
        else:
            exaggeration = map_zscale * (xmax - xmin) / plot_width / mtodeglon

    xoffset = (width - abs(plot_width) - margins.space_left - margins.space_right) / 2 + margins.space_left
    yoffset = (height - abs(plot_height) - margins.space_bottom - margins.space_top) / 2 + margins.space_bottom

    # degree annotation format -- NOTE: threshold is 1.0 here, not the
    # 4.0 mbpy_layout.compute_layout() uses for 2D plots (mbm_grd3dplot
    # line ~1308 vs. mbm_grdplot line ~1615): confirmed as a genuine
    # difference between the two macros, not a transcription slip.
    degree_format = "ddd:mm"
    if gridprojected == 0:
        xsize = (xmax - xmin) / 3
        ysize = (ymax - ymin) / 3
        size = min(xsize, ysize)
        if size > 1.0:
            degree_format = "ddd"
        elif size > (1.0 / 60.0):
            degree_format = "ddd:mm"
        else:
            degree_format = "ddd:mm:ss"

    sl = scale_loc.lower()
    page_h = layout.PAGE_HEIGHT_IN[pagesize]
    if sl == "l":
        colorscale_length = plot_height
        colorscale_thick = 0.013636364 * page_h
        colorscale_offx = -0.13636 * page_h
        colorscale_offy = 0.5 * plot_height
        colorscale_vh = "+v"
    elif sl == "r":
        colorscale_length = plot_height
        colorscale_thick = 0.013636364 * page_h
        colorscale_offx = plot_width + 0.0909 * page_h
        colorscale_offy = 0.5 * plot_height
        colorscale_vh = "+v"
    elif sl == "t":
        colorscale_length = plot_width
        colorscale_thick = 0.013636364 * page_h
        colorscale_offx = 0.5 * plot_width
        colorscale_offy = plot_height + 0.15 * page_h
        colorscale_vh = "+h"
    else:
        colorscale_length = plot_width
        colorscale_thick = 0.013636364 * page_h
        colorscale_offx = 0.5 * plot_width
        colorscale_offy = -0.045454545 * page_h
        colorscale_vh = "+h"
    colorscale_length = max(colorscale_length, 3.0)

    dzz = zmax - zmin
    contour_int = 0.0
    if dzz > 0:
        base = int((math.log(dzz) / math.log(10.0)) + 0.5)
        contour_int = (10 ** base) / 10.0
        if dzz / contour_int < 10:
            contour_int = contour_int / 4
        elif dzz / contour_int < 20:
            contour_int = contour_int / 2

    base_tick_xy = layout.get_base_tick(0, xmin, xmax, ymin, ymax).tick_x
    base_tick_z = get_base_tick_z(zmin, zmax)

    return Layout3D(
        pagesize=pagesize, scale_loc=scale_loc,
        projection=projection, projection_pars=projection_pars, region=bounds_plot,
        zmin=zmin, zmax=zmax,
        view_azimuth=view_azimuth, view_elevation=view_elevation,
        map_zscale=map_zscale, exaggeration=exaggeration,
        landscape=landscape, plot_width=plot_width, plot_height=plot_height,
        xoffset=xoffset, yoffset=yoffset, degree_format=degree_format,
        colorscale_length=colorscale_length, colorscale_thick=colorscale_thick,
        colorscale_offx=colorscale_offx, colorscale_offy=colorscale_offy, colorscale_vh=colorscale_vh,
        contour_int=contour_int, base_tick_xy=base_tick_xy, base_tick_z=base_tick_z,
        gridprojected=gridprojected,
    )


def _basemap_axes_3d(lay: Layout3D, tick_info: Optional[str], title: Optional[str]) -> str:
    """Port of the -B assembly at mbm_grd3dplot lines ~1941-1971: an x/y/z
    tick spec plus a view-azimuth-dependent choice of which two frame
    sides (of the four NSEW verticals) get ticks/annotations, since a 3D
    view can only usefully show the two walls facing the viewer."""
    if tick_info:
        interval = f"-B{tick_info}"
    else:
        interval = f"-Bxy{lay.base_tick_xy} -Bz{lay.base_tick_z}"

    az = lay.view_azimuth % 360.0
    if 0.0 <= az < 90.0:
        sides = "NEZ"
    elif 90.0 <= az < 180.0:
        sides = "SEZ"
    elif 180.0 <= az < 270.0:
        sides = "WSZ"
    else:
        sides = "WNZ"
    frame = f"-B{sides}" + (f'+t"{title}"' if title else "")

    return f"{frame} {interval}"


@dataclass
class ScriptOptions3D:
    grid: str
    root: str
    layout: Layout3D
    grid_info: GridInfo
    output_format: str = "pdf"
    title: Optional[str] = None
    tick_info: Optional[str] = None
    colorbar_label: Optional[str] = None
    length_scale: Optional[str] = None
    dpi: Optional[int] = None

    shade: Optional[ShadeOptions] = None    # None => plain color_mode 1 fill
    mesh: bool = False                       # color_mode 6/7 (-Qm) instead of -Qi<dpi>
    mesh_contour_pen: Optional[str] = None   # color_mode 7's grdview -W (mbm_grd3dplot's -MVW)
    color_palette: int = 1
    color_flip: bool = False
    data_scale: Optional[float] = None
    cpt_lines: Optional[List[CptLine]] = None
    cpt_file: Optional[str] = None

    drape_file: Optional[str] = None         # grdview -G (mbm_grd3dplot's -N)
    mesh_null: Optional[str] = None          # grdview -N (mbm_grd3dplot's -MVN)
    mesh_zlevel: Optional[str] = None        # grdview/basemap -Z (mbm_grd3dplot's -MVZ)

    coast: Optional[CoastOptions] = None
    text_labels: List[str] = field(default_factory=list)

    gmt_defs: List[str] = field(default_factory=list)


def _show_colorscale_3d(opts: ScriptOptions3D) -> bool:
    """Port of the psscale gate at mbm_grd3dplot line ~2093:
    ($color_mode && $color_mode < 7 && $color_palette < 5). NOTE this
    includes color_mode 6 (plain mesh) even though grdview itself never
    gets a -C on that mode (mbm_grd3dplot line ~1989: "if (color_mode !=
    6) -C..."). Reproduced as-is: the CPT is still built and written
    for mode 6, so showing a scale bar for it, while not obviously
    useful, is exactly what the real macro does."""
    return opts.color_palette < 5


def generate_script_3d(opts: ScriptOptions3D) -> str:
    """Build the full GMT modern-mode shellscript text for `opts`.
    Does not touch the filesystem (except that the caller is expected
    to have already written opts.cpt_lines via write_cpt_file(), or to
    do so before running the returned script) -- see generate_and_write_3d()."""
    lay = opts.layout
    j = f"-J{lay.projection}{lay.projection_pars} -Jz{lay.map_zscale:.5g}"
    p = f"-p{lay.view_azimuth:.5g}/{lay.view_elevation:.5g}"
    r = f"-R{lay.region}/{lay.zmin:.10g}/{lay.zmax:.10g}"
    cptfile = opts.cpt_file or f"{opts.root}.cpt"

    lines = [
        "#!/usr/bin/env bash",
        "#",
        f"# GMT modern-mode 3D plotting script for {opts.grid}",
        "# Generated by mbpy_grd3dplot_script.py (Python port of mbm_grd3dplot)",
        "#",
        "set -e",
        "",
        f"gmt begin {opts.root} {opts.output_format}",
        "",
    ]

    if opts.dpi:
        lines.append(f"gmt figure {opts.root} {opts.output_format} E{opts.dpi}")
        lines.append("")

    lines.append("# Set temporary GMT defaults")
    baseline_pairs = []
    for gmt_def in baseline_gmt_defaults(lay.pagesize, opts.color_flip, lay.degree_format):
        param, _, value = gmt_def.partition(" ")
        baseline_pairs.append(f"{param} {value}")
    lines.append(f"gmt set {' '.join(baseline_pairs)}")
    lines.append("")

    if opts.gmt_defs:
        lines.append("# User-defined GMT parameter overrides")
        user_pairs = []
        for gmt_def in opts.gmt_defs:
            param, _, value = gmt_def.partition("/")
            user_pairs.append(f"{param} {value}")
        lines.append(f"gmt set {' '.join(user_pairs)}")
        lines.append("")

    file_use = opts.grid
    if opts.data_scale is not None:
        file_use = f"{opts.root}_scaled.grd"
        lines.append("# Rescale data")
        lines.append(f"gmt grdmath {opts.grid} {opts.data_scale:.6g} MUL = {file_use}")
        lines.append("")

    temp_file_cleanup = None
    grid_for_view = file_use
    intensity_file = None

    shade = resolve_shade_defaults(opts.shade) if opts.shade else None
    geographic = opts.grid_info.gridprojected == 0

    if shade is None or shade.color_mode == 1 or opts.mesh:
        pass
    elif shade.color_mode == 2:
        cmds, intensity_file = _illumination_commands(
            file_use, opts.root, shade.magnitude, shade.azimuth, shade.elevation, geographic
        )
        lines.append("# Get shading array")
        lines.extend(cmds)
        lines.append("")
    elif shade.color_mode == 3:
        if not shade.file_intensity:
            raise ValueError("color_mode 3 requires shade.file_intensity")
        cmds, intensity_file = _intensity_commands_mode3(
            shade.file_intensity, opts.root, shade.stretch_shade, shade.magnitude
        )
        if cmds:
            lines.append("# Get shading array")
            lines.extend(cmds)
            lines.append("")
    elif shade.color_mode == 4:
        cmds, grid_for_view = _slope_commands(file_use, opts.root, geographic, None)
        temp_file_cleanup = grid_for_view
        lines.append("# Get slope array")
        lines.extend(cmds)
        lines.append("")
    elif shade.color_mode == 5:
        cmds, intensity_file = _slope_commands(file_use, opts.root, geographic, shade.magnitude)
        temp_file_cleanup = intensity_file
        lines.append("# Get slope array")
        lines.extend(cmds)
        lines.append("")

    gv = [f"gmt grdview {grid_for_view} {j} {p} {r}"]
    if not opts.mesh:
        gv.append(f"-C{cptfile}")
    if opts.mesh_null:
        gv.append(f"-N{opts.mesh_null}")
    if opts.mesh_zlevel:
        gv.append(f"-Z{opts.mesh_zlevel}")
    if shade is not None and shade.color_mode in (2, 3):
        gv.append(f"-I{intensity_file}")
    elif shade is not None and shade.color_mode == 5:
        gv.append(f"-I{intensity_file}")
    if opts.drape_file:
        gv.append(f"-G{opts.drape_file}")
    if opts.mesh:
        gv.append("-Qm")
        if opts.mesh_contour_pen:
            gv.append(f"-W{opts.mesh_contour_pen}")
    else:
        gv.append(f"-Qi{opts.dpi}" if opts.dpi else "-Qi")
    lines.append(" ".join(gv))
    lines.append("")

    if opts.coast is not None:
        co = opts.coast
        cparts = [f"gmt coast {j} {p} {r}"]
        if co.coast_resolution:
            cparts.append(f"-D{co.coast_resolution}")
        if co.coast_dryfill:
            cparts.append(f"-G{co.coast_dryfill}")
        if co.coast_wetfill:
            cparts.append(f"-S{co.coast_wetfill}")
        if co.coast_lakefill:
            cparts.append(f"-C{co.coast_lakefill}")
        if co.coast_river:
            cparts.append(f"-I{co.coast_river}")
        for boundary in co.coast_boundaries:
            cparts.append(f"-N{boundary}")
        if co.coast_pen:
            cparts.append(f"-W{co.coast_pen}")
        lines.append(" ".join(cparts))
        lines.append("")

    if _show_colorscale_3d(opts):
        label = opts.colorbar_label if opts.colorbar_label is not None else "z"
        if lay.colorscale_vh == "+v":
            d_xy = f"{lay.colorscale_offx:.5g}/0"
        else:
            d_xy = f"0/{lay.colorscale_offy:.5g}"
        lines.append(
            f"gmt colorbar -C{cptfile} "
            f"-Dx{d_xy}"
            f"{lay.colorscale_vh}+w{lay.colorscale_length:.5g}/{lay.colorscale_thick:.5g} "
            f'-B+l"{label}"'
        )
        lines.append("")

    if opts.text_labels:
        lines.append(f"gmt text {j} {p} {r} -F+f+a+j <<EOF")
        for raw in opts.text_labels:
            tx, ty, tsize, tangle, font, just, txt = raw.split("/", 6)
            lines.append(f"{tx} {ty} {tsize},{font} {tangle} {just} {txt}")
        lines.append("EOF")
        lines.append("")

    basemap_parts = [f"gmt basemap {j} {p} {r} {_basemap_axes_3d(lay, opts.tick_info, opts.title)}"]
    if opts.mesh_zlevel:
        basemap_parts.append(f"-Z{opts.mesh_zlevel}")
    if opts.length_scale:
        basemap_parts.append(f"-L{opts.length_scale}")
    lines.append(" ".join(basemap_parts))
    lines.append("")

    if temp_file_cleanup:
        lines.append("# Delete surplus files")
        lines.append(f"rm -f {temp_file_cleanup}")
        lines.append("")

    lines.append("gmt end")
    lines.append("")
    return "\n".join(lines)


def generate_and_write_3d(opts: ScriptOptions3D, script_path: str) -> None:
    """Write both the CPT file (if opts.cpt_lines is set) and the
    generated shellscript to disk, and make the script executable."""
    if opts.cpt_lines is not None and opts.cpt_file is None:
        write_cpt_file(f"{opts.root}.cpt", opts.cpt_lines)
    Path(script_path).write_text(generate_script_3d(opts))
    Path(script_path).chmod(0o755)
