#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_histplot_script.py
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
#   Python port of mbm_histplot's page-layout and script-generation
#   logic, targeting GMT **modern mode** (`gmt begin`/`gmt end`,
#   `gmt histogram` -- the modern-mode name for classic `pshistogram`,
#   confirmed to be the same module by identical --help banners)
#   instead of mbm_histplot's own classic-mode <root>.cmd generator.
#
#   mbm_histplot is a much simpler macro than mbm_grdplot/mbm_grd3dplot:
#   it plots a one-column data *file* (not a grid) as a frequency
#   histogram. It shares the same page-size/width/height/font *tables*
#   and GetProjection (confirmed identical to mbm_grdplot's copy by
#   direct diff, module scaffolding aside) -- reused here from
#   mbpy_layout.py. Its GetPageSize, however, is NOT the same sub
#   despite using the same tables (see get_page_size_hist() below), and
#   neither is its default-scale auto-fit algorithm (see
#   compute_layout_hist()'s own docstring) -- both were wrongly assumed
#   reusable from mbpy_layout/mbpy_grdplot_script in an earlier version
#   of this port, which silently produced a squashed, barely-legible
#   plot until caught against real mbm_trnplot output. What is unique
#   to mbm_histplot:
#
#     - it pre-scans the input file itself (mbm_histplot lines ~270-359)
#       to determine the plot's x bounds (data min/max, or the -R
#       override) and, unless -R gave an explicit y range too, bins the
#       data itself with the *same* cell width GMT's own `histogram`
#       will use, purely to compute a y-axis (frequency %) upper bound
#       with a 10% margin. The actual bars in the final plot are drawn
#       by `gmt histogram` re-reading and re-binning the raw file
#       itself (mbm_histplot never passes its own bin counts to GMT) --
#       this port does the same two-pass thing, not because GMT could
#       not bin the data cheaper, but because the pre-scan is also
#       exactly how the plot's y-range is chosen at all.
#     - its own GetBaseTickLinear (mbm_histplot lines ~1574-1606): a
#       "nice number" tick heuristic distinct from mbpy_layout's
#       (rounds to (10**n)/5 with a /4 or /2 refinement, vs.
#       mbpy_layout's (10**n)/10 with the same /4-or-/2 shape) used
#       whenever the projection is plain Cartesian ("x"/"X", not the
#       special "xd"/"Xd" -- see get_base_tick_geo() below); its
#       GetBaseTick (mbm_histplot lines ~1608-1695) is the same
#       geographic degree/minute/second table mbpy_layout.get_base_tick
#       already has, just with tick = min(dx, dy)/5 instead of /3 --
#       used only for the "xd"/"Xd" or a real geographic -J.
#     - a fixed, always-on mean/+-1 sigma/+-2 sigma normal-curve overlay
#       (`-N0+pblack -N1+pred -N2+pgreen`, mbm_histplot line ~910) with
#       no option to disable it -- reproduced as unconditional here too.
#
#   NOTE on real, confirmed-dead pieces of mbm_histplot itself: several
#   variables read out in the script-header/verbose-summary code ($cptfile,
#   $file_cpt, $data_scale, @xyfiles, $save_temp_files, $unix_stamp/
#   $unix_stamp_on) are never assigned anywhere in the source -- no -M
#   sub-option or top-level flag sets any of them (mbm_histplot's -M only
#   dispatches -MIE/-MIT, unlike mbm_grdplot/mbm_grd3dplot's much larger
#   -M grammar). These read as leftover copy-paste from the mbm_grdplot
#   template mbm_histplot was originally cloned from. This port does not
#   expose --data-scale, --xy-overlay, --unix-stamp, or any CPT-file
#   option, since none of that is real, reachable functionality in the
#   macro being ported.
#
# Status:
#   Covers the full documented option set (cell width, fill, pen,
#   title/x-label/y-label, tick-info override, region, projection/page
#   sizing). Verified by running the generated script through a real
#   GMT 6 installation; not exhaustively line-by-line diffed against
#   Perl output the way mbpy_grdplot_script.py was.
#
import math
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import mbpy_layout as layout
from mbpy_grdplot_script import baseline_gmt_defaults

_DTR = 3.1415926 / 180.0


def scan_data_bounds(file_path: str) -> Tuple[float, float, int]:
    """Port of mbm_histplot's first data-file pass (lines ~270-288):
    find the data's own min/max and count. Values are read one per
    line, exactly as the Perl `while ($data=<F>)` loop does (leading/
    trailing whitespace and blank lines are skipped)."""
    xmin = xmax = None
    count = 0
    with open(file_path) as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            v = float(s.split()[0])
            if xmin is None or v < xmin:
                xmin = v
            if xmax is None or v > xmax:
                xmax = v
            count += 1
    if count <= 0:
        raise RuntimeError(f"Input file {file_path} appears to contain no data.")
    return xmin, xmax, count


def compute_histogram_yrange(file_path: str, xmin: float, cellwidth: float, ncell: int, count: int) -> float:
    """Port of the second data-file pass (lines ~332-359): bin the data
    at `cellwidth` starting at `xmin`, convert counts to percent of
    total, and return 1.1x the largest bin's percentage (or 50.0 if
    every bin is empty) -- mirrors mbm_histplot's own y-axis margin."""
    cellcount = [0] * ncell
    with open(file_path) as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            v = float(s.split()[0])
            i = math.floor((v - xmin) / cellwidth)
            if 0 <= i < ncell:
                cellcount[i] += 1
    ymax = 0.0
    for c in cellcount:
        pct = 100.0 * c / count
        if pct > ymax:
            ymax = pct
    return 50.0 if ymax <= 0.0 else 1.1 * ymax


def get_base_tick_linear(xmin: float, xmax: float, ymin: float, ymax: float) -> Tuple[float, float]:
    """Port of mbm_histplot's own GetBaseTickLinear (lines ~1574-1606):
    a "nice number" tick heuristic distinct from mbpy_layout's own
    get_base_tick() -- (10**n)/5 rather than (10**n)/10, with the same
    /4-or-/2 refinement shape."""
    def one(d: float) -> float:
        if d <= 0:
            return 0.0
        base = int((math.log(d) / math.log(10.0)) + 0.5)
        tick = (10 ** base) / 5.0
        if d / tick < 5:
            tick = tick / 4.0
        elif d / tick < 10:
            tick = tick / 2.0
        return tick

    return one(xmax - xmin), one(ymax - ymin)


def get_page_size_hist(pagesize: str) -> layout.PageMargins:
    """Port of mbm_histplot's own GetPageSize (a completely different
    sub from mbpy_layout.get_page_size(), despite sharing the same page-
    size/width/height tables) -- confirmed by direct comparison: unlike
    mbm_grdplot's version, it has no scale_loc-dependent branching at
    all (a single fixed margin proportion regardless of a color scale's
    placement -- appropriate since a histogram has no color scale bar to
    make room for) and, more consequentially, no clamping of the margins
    to an absolute maximum -- mbm_grdplot's version clamps e.g.
    space_top to min(1.5*ratio, 4.5), while this one lets every margin
    scale unbounded with page size. Confirmed directly: reusing
    mbpy_layout.get_page_size() here silently produced a ~13% smaller
    height_max_landscape than the real macro for page size "a" (5.0 vs.
    the real 5.75), which was large enough to visibly distort the
    default-scale computation in compute_layout_hist()."""
    page_h = layout.PAGE_HEIGHT_IN[pagesize]
    page_w = layout.PAGE_WIDTH_IN[pagesize]
    a_h = layout.PAGE_HEIGHT_IN["a"]
    ratio = page_h / a_h

    space_top = 1.25 * ratio
    space_bottom = 1.50 * ratio
    space_left = 1.00 * ratio
    space_right = 1.00 * ratio

    return layout.PageMargins(
        space_top, space_bottom, space_left, space_right,
        width_max_landscape=page_h - space_left - space_right,
        height_max_landscape=page_w - space_bottom - space_top,
        width_max_portrait=page_w - space_left - space_right,
        height_max_portrait=page_h - space_bottom - space_top,
    )


def get_base_tick_geo(xmin: float, xmax: float, ymin: float, ymax: float) -> str:
    """Port of mbm_histplot's own GetBaseTick (lines ~1608-1695): the
    same geographic degree/minute/second lookup table mbpy_layout.
    get_base_tick() uses, but with tick = min(dx, dy)/5 rather than /3.
    Used only for a real geographic -J or the special "xd"/"Xd" flag
    (mbpy_grd3dplot/mbpy_layout call this "linear"; mbm_histplot itself
    confusingly calls it "geographic" -- same meaning, see this
    module's own top docstring)."""
    base_tick = min((xmax - xmin) / 5.0, (ymax - ymin) / 5.0)
    tick_str = "60"
    for threshold, label in layout._GEOGRAPHIC_TICK_TABLE:
        if base_tick < threshold:
            tick_str = label
            break
    return tick_str


@dataclass
class LayoutHist:
    pagesize: str
    projection: str
    projection_pars: str
    region: str
    xoffset: float
    yoffset: float
    axes_interval: str   # the "-Bx...+l... -By...+l..." or "-B<tick>" piece


def compute_layout_hist(
    xmin: float, xmax: float, ymin: float, ymax: float,
    pagesize: str = "a",
    map_scale: Optional[str] = None,
    orientation: int = 0,
    xlabel: str = " ",
    ylabel: str = "Frequency %",
    tick_info: Optional[str] = None,
) -> LayoutHist:
    """Port of mbm_histplot's page/scale-fit algorithm (lines ~369-738):
    the same GetPageSize/GetProjection heuristics mbpy_layout uses
    (default projection "x", plain linear/Cartesian, unlike mbm_grdplot's
    default Mercator), applied to the histogram's own x/y (value/
    frequency-percent) bounding box instead of a grid's geographic
    bounds.
    """
    margins = get_page_size_hist(pagesize)

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

        # This trial-value append (for the mapproject probe just below)
        # only applies to a partially-parsed *user* map_scale (e.g. a
        # bare "-Jx" with no number, where get_projection() leaves
        # projection_pars as just the projection letter's remainder).
        # It must not run for the "no map_scale given" default below,
        # whose own "1/1" is already a complete, valid probe value --
        # appending trial_value to *that* produced a nonsensical
        # "1/11.0" (confirmed the cause of a garbled probe scale).
        if (use_scale and plot_scale) or (use_width and plot_width):
            pass
        elif use_scale or use_width:
            projection_pars = f"{projection_pars}{separator}{trial_value}"
    else:
        projection, projection_pars = "x", "1/1"
        use_scale = True

    bounds_plot = f"{xmin:.8g}/{xmax:.8g}/{ymin:.8g}/{ymax:.8g}"
    dxx, dyy = layout._mapproject_bbox(xmin, xmax, ymin, ymax, projection, projection_pars, bounds_plot)
    dxx, dyy = abs(dxx), abs(dyy)

    landscape = portrait = False
    width = height = 0.0
    width_max = height_max = 0.0

    if (use_scale and plot_scale) or (use_width and plot_width):
        plot_width_, plot_height_ = dxx, dyy
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
                m2 = get_page_size_hist(elem)
                wm = m2.width_max_portrait if portrait else m2.width_max_landscape
                hm = m2.height_max_portrait if portrait else m2.height_max_landscape
                if plot_width_ <= wm and plot_height_ <= hm:
                    good_page = elem
                    break
            if good_page is None:
                good_page = pagesize
            pagesize = good_page
            margins = get_page_size_hist(pagesize)
            width = layout.PAGE_WIDTH_IN[pagesize] if portrait else layout.PAGE_HEIGHT_IN[pagesize]
            height = layout.PAGE_HEIGHT_IN[pagesize] if portrait else layout.PAGE_WIDTH_IN[pagesize]

        plot_width, plot_height = plot_width_, plot_height_

    elif use_scale and projection == "x" and not linear:
        # Port of mbm_histplot's own real default-scale branch (lines
        # ~526-567) -- NOT the same algorithm as mbm_grdplot/mbpy_layout's
        # uniform-scale auto-fit (which picks a single scale preserving
        # the data's own x/y aspect ratio, appropriate for a geographic
        # map). A histogram's x (data value) and y (frequency percent)
        # axes have unrelated units and usually wildly different natural
        # ranges, so mbm_histplot instead computes two *independent*
        # scales that each separately stretch to fill the page, and
        # (unlike mbm_grdplot) always defaults to landscape orientation
        # rather than picking whichever orientation yields a bigger
        # scale. Confirmed directly against real mbm_histplot output: a
        # first attempt at this port reused mbm_grdplot's uniform-scale
        # logic here, which -- for a real 1.4-units-wide by 29-percent-
        # tall histogram -- produced a single scale small enough to fit
        # the *y* range on the page, squashing the plot to a fraction of
        # an inch wide; the real macro's own -Jx<xscale>/<yscale> two-
        # value scale for the same data confirmed this fix.
        if orientation == 1:
            portrait = True
            plot_scale_x = margins.width_max_portrait / dxx
            plot_scale_y = margins.height_max_portrait / dyy
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]
        else:
            landscape = True
            plot_scale_x = margins.width_max_landscape / dxx
            plot_scale_y = margins.height_max_landscape / dyy
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]

        plot_width = dxx * plot_scale_x
        plot_height = dyy * plot_scale_y
        projection_pars = f"{plot_scale_x}/{plot_scale_y}"

    elif use_width and projection == "X" and not linear:
        # Port of mbm_histplot's own real default-width branch (lines
        # ~568-607): same independent-x/y-axis idea as the scale branch
        # above, but for an explicit total plot width (-JX<width>)
        # instead of an auto-computed scale.
        if orientation == 1:
            portrait = True
            plot_width_x = margins.width_max_portrait
            plot_width_y = margins.height_max_portrait
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]
        else:
            landscape = True
            plot_width_x = margins.width_max_landscape
            plot_width_y = margins.height_max_landscape
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]

        plot_width, plot_height = plot_width_x, plot_width_y
        projection_pars = f"{plot_width_x}/{plot_width_y}"

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

        plot_width = dxx * plot_scale
        plot_height = dyy * plot_scale

        base = layout._base_pars(projection, map_scale)
        if use_ratio:
            top = int(1 / plot_scale)
            projection_pars = f"{base}{separator}1:{top}"
        else:
            projection_pars = layout._sprintf_g5(base, separator, plot_scale)
        if linear:
            projection_pars = f"{projection_pars}d"

    elif use_width:
        plot_width_landscape = min(margins.height_max_landscape * dxx / dyy, margins.width_max_landscape)
        plot_width_portrait = min(margins.height_max_portrait * dxx / dyy, margins.width_max_portrait)

        if orientation == 1:
            portrait = True
            plot_width = plot_width_portrait
        elif orientation == 2:
            landscape = True
            plot_width = plot_width_landscape
        elif plot_width_landscape > plot_width_portrait:
            landscape = True
            plot_width = plot_width_landscape
        else:
            portrait = True
            plot_width = plot_width_portrait

        plot_height = plot_width * dyy / dxx
        if landscape:
            width, height = layout.PAGE_HEIGHT_IN[pagesize], layout.PAGE_WIDTH_IN[pagesize]
        else:
            width, height = layout.PAGE_WIDTH_IN[pagesize], layout.PAGE_HEIGHT_IN[pagesize]

        base = layout._base_pars(projection, map_scale)
        projection_pars = layout._sprintf_g5(base, separator, plot_width)
        if linear:
            projection_pars = f"{projection_pars}d"

    xoffset = (width - abs(plot_width) - margins.space_left - margins.space_right) / 2 + margins.space_left
    yoffset = (height - abs(plot_height) - margins.space_bottom - margins.space_top) / 2 + margins.space_bottom

    if tick_info:
        axes_interval = f"-B{tick_info}"
    elif projection[:1] in ("x", "X") and not linear:
        tick_x, tick_y = get_base_tick_linear(xmin, xmax, ymin, ymax)
        axes_interval = f'-Bx{tick_x:.6g}+l"{xlabel}" -By{tick_y:.6g}+l"{ylabel}"'
    else:
        tick = get_base_tick_geo(xmin, xmax, ymin, ymax)
        axes_interval = f"-B{tick}"

    return LayoutHist(
        pagesize=pagesize, projection=projection, projection_pars=projection_pars,
        region=bounds_plot, xoffset=xoffset, yoffset=yoffset, axes_interval=axes_interval,
    )


@dataclass
class ScriptOptionsHist:
    file_data: str
    root: str
    layout: LayoutHist
    xmin: float                      # bin range for -T (mbm_histplot's own $xmin/$xmax,
    xmax: float                      # which may differ from the plot's x -R after cell-gridding)
    cellwidth: float                 # -MC's cell width -> -T<xmin>/<xmax>/<cellwidth>
    output_format: str = "pdf"
    title: Optional[str] = None
    dpi: Optional[int] = None
    fill: Optional[str] = "gray"     # -G; "N" (or None) disables fill
    pen: Optional[str] = "1p"        # -L (histogram's own outline-pen flag)
    gmt_defs: List[str] = field(default_factory=list)


def generate_script_hist(opts: ScriptOptionsHist) -> str:
    """Build the full GMT modern-mode shellscript text for `opts`.
    Does not touch the filesystem at all (unlike the grdplot/grd3dplot
    generators, there is no CPT file to write for a histogram plot)."""
    lay = opts.layout
    j = f"-J{lay.projection}{lay.projection_pars}"
    r = f"-R{lay.region}"

    lines = [
        "#!/usr/bin/env bash",
        "#",
        f"# GMT modern-mode histogram plotting script for {opts.file_data}",
        "# Generated by mbpy_histplot_script.py (Python port of mbm_histplot)",
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
    for gmt_def in baseline_gmt_defaults(lay.pagesize, False, "ddd:mm"):
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

    title = opts.title if opts.title is not None else f"Frequency Histogram of {opts.file_data}"
    axes = f"-BWESN" + (f'+t"{title}"' if title else "") + f" {lay.axes_interval}"

    hparts = [f"gmt histogram {opts.file_data} {j} {r}", axes, f"-T{opts.xmin:.8g}/{opts.xmax:.8g}/{opts.cellwidth:.8g}", "-Z1"]
    if opts.pen:
        # NOTE: classic pshistogram's outline-pen flag was -L; current
        # GMT's `histogram` module (confirmed via its own --help) has no
        # -L at all and uses -W instead, like nearly every other GMT
        # plotting module's pen option. mbm_histplot's Perl source still
        # emits classic -L$pen, which is simply not a valid flag for a
        # current `gmt histogram`/`pshistogram` -- this is not a classic-
        # vs-modern-*mode* difference (unlike gmtset/set) but a genuine
        # GMT module syntax change over time, ported to current syntax.
        hparts.append(f"-W{opts.pen}")
    if opts.fill and opts.fill != "N":
        hparts.append(f"-G{opts.fill}")
    hparts.append("-N0+pblack -N1+pred -N2+pgreen")
    lines.append(" ".join(hparts))
    lines.append("")

    lines.append("gmt end")
    lines.append("")
    return "\n".join(lines)


def generate_and_write_hist(opts: ScriptOptionsHist, script_path: str) -> None:
    Path(script_path).write_text(generate_script_hist(opts))
    Path(script_path).chmod(0o755)
