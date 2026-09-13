#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_xyplot_script.py
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
#   Python port of mbm_xyplot: page/scale-fit layout (shared with
#   mbm_histplot -- see below) plus the GMT **modern mode** script
#   generator for plotting one or more xy data files.
#
#   mbm_xyplot's own page/scale-fit algorithm (its subs GetPageSize,
#   GetProjection, GetBaseTick, GetBaseTickLinear, and the main-body
#   scaling logic around "get user constraints on map scale" through
#   "place the origin") was confirmed, by direct diff against both
#   mbm_grdplot and mbm_histplot, to be byte-for-byte identical (modulo
#   whitespace) to mbm_histplot's own copies of the same subs -- NOT
#   mbm_grdplot's (which uses a different, scale_loc-aware, clamped
#   GetPageSize and a gridprojected-aware GetBaseTick; see
#   mbpy_histplot_script.py's own top docstring for how that was
#   established). get_page_size_hist()/get_base_tick_linear()/
#   get_base_tick_geo() are therefore imported and reused here rather
#   than re-derived, the same way mbpy_histplot_script.py already
#   imports baseline_gmt_defaults() from mbpy_grdplot_script.py.
#
#   A real, confirmed bug was found in mbm_xyplot's own -MGT (text
#   label) misc-option parsing while comparing it against mbm_grdplot's
#   otherwise-identical -MGT handling (mbm_xyplot lines ~245-260): the
#   outer match tests for "GT" but the *inner* extraction regex tests
#   for "GP" (probably a copy/paste slip), so $txt is always undefined
#   and every -MGT text label mbm_xyplot has ever been given is silently
#   discarded ("Invalid text label ignored"), regardless of validity.
#   Moot for this port: like mbm_grdplot's own Python replacement, this
#   tool exposes text labels as a plain --text long option (see
#   mbpy_xyplot) instead of reproducing the "-M<sub>:<sub>:..." mini-
#   language at all, so the bug has nothing to be ported *into*.
#
#   The other real, deliberate change from the classic macro: mbm_xyplot
#   preprocesses every input file (delimiter-splitting, per-file column-
#   selection/math via its "-IC..." mini-language) by writing a
#   *persistent* temporary file "<file><index>.<pid>" into the current
#   working directory for every run -- left behind unless the user also
#   passes -Z, and even then only actually deleted if the resulting
#   command script is executed with -Z's cleanup lines intact. This was
#   the whole reason this replacement was requested. Here, the
#   equivalent column-selection/math is instead performed:
#     - once in Python, in scan_xy_bounds(), purely in memory, to find
#       the auto-fit plot bounds (no file written at all); and
#     - at plot time, by piping each input file through a small `awk`
#       program straight into `gmt plot`'s stdin (see _plot_pipeline())
#       -- a live, ephemeral Unix pipe, not a file on disk.
#   No file this tool writes outlives the plotting script itself except
#   the final plot.
#
import math
import re
import shlex
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import mbpy_layout as layout
from mbpy_grdplot_script import baseline_gmt_defaults
from mbpy_histplot_script import get_page_size_hist, get_base_tick_linear, get_base_tick_geo
from mbpy_misc import CoastOptions


# ---------------------------------------------------------------------------
# Column-expression mini-language (mbm_xyplot's own "-IC<xexpr>_<yexpr>"):
# "c[N]" selects field N (1-indexed), "#" is the input line number. mbm_xyplot
# itself just textually substitutes these into a string handed to Perl's
# eval(); the same textual substitution is used here, targeting first a
# small, safely-sandboxed Python eval() (for scan_xy_bounds()'s in-memory
# bounds computation) and separately an `awk` expression (for the actual
# plot-time pipeline) -- see _expr_to_py()/_expr_to_awk().
# ---------------------------------------------------------------------------

_COL_RE = re.compile(r"c\[(\d+)\]")


def _expr_to_py(expr: str) -> str:
    e = _COL_RE.sub(lambda m: f"_col({m.group(1)})", expr)
    return e.replace("#", "_linecnt")


def _expr_to_awk(expr: str) -> str:
    e = _COL_RE.sub(lambda m: f"${m.group(1)}", expr)
    e = e.replace("#", "NR")
    return e.replace("**", "^")


def _awk_string_literal(s: str) -> str:
    escaped = s.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def _awk_regex_escape(ch: str) -> str:
    return "".join(("\\" + c) if c in ".^$*+?()[]{}|\\" else c for c in ch)


@dataclass
class XYFileEntry:
    """One -I input file plus its (possibly per-file-overridden) styling
    and column-selection, i.e. one iteration of mbm_xyplot's @xyfiles/
    @xysymbols/@xyfills/@xypens/@delimiters/@xmath/@ymath parallel
    arrays (mbm_xyplot lines ~413-476)."""
    file: str
    xexpr: str = "c[1]"
    yexpr: str = "c[2]"
    delimiter: Optional[str] = None   # None => default whitespace splitting
    symbol: str = "N"
    fill: str = "N"
    pen: str = "N"


def scan_xy_bounds(entries: List[XYFileEntry]) -> Tuple[float, float, float, float]:
    """Port of mbm_xyplot's own per-line data extraction and bounds-finding
    (lines ~499-596): read every entry's file, split each line on its
    delimiter, evaluate its x/y column-expression, and track the running
    min/max over *all* files combined (mbm_xyplot's @xvalues/@yvalues are
    global across every -I file, not per-file). Lines containing ">"
    (psxy/plot multi-segment markers) are skipped, matching mbm_xyplot's
    own segment-header passthrough. Unlike mbm_xyplot, nothing is written
    to disk -- this is purely an in-memory scan.

    NOTE: mbm_xyplot's own per-line numeric-result validation regex
    ("-?\\d*\\.?\\d*|-?\\.\\d+") is satisfied by the empty string (every
    branch of the alternation is fully optional), so it never actually
    rejects anything; the only input that is really ever skipped in
    practice is one whose column-expression throws in eval() (chiefly a
    literal divide-by-zero -- Perl is otherwise very lenient, e.g. an
    out-of-range c[N] just evaluates as 0, not an error) or whose result
    stringifies to contain "nan". That real behavior -- skip with a
    warning on eval failure or a non-finite result -- is what is
    reproduced below, not the vestigial regex.
    """
    xmin = ymin = math.inf
    xmax = ymax = -math.inf

    for entry in entries:
        py_x = _expr_to_py(entry.xexpr)
        py_y = _expr_to_py(entry.yexpr)
        linecnt = 1
        with open(entry.file) as f:
            for raw_line in f:
                line = raw_line.rstrip("\n")
                if ">" in line:
                    linecnt += 1
                    continue

                if entry.delimiter:
                    if line.startswith(entry.delimiter):
                        line = line[len(entry.delimiter):]
                    fields = line.split(entry.delimiter)
                else:
                    fields = line.split()

                def _col(n: int, _fields=fields) -> float:
                    return float(_fields[n - 1])

                try:
                    env = {"_col": _col, "_linecnt": linecnt, "__builtins__": {}}
                    xval = float(eval(py_x, env))
                    yval = float(eval(py_y, env))
                    if not (math.isfinite(xval) and math.isfinite(yval)):
                        raise ValueError("non-finite result")
                except Exception:
                    print(
                        f"WARNING!!! NON-NUMERIC RESULT DETECTED! "
                        f"Skipping line {linecnt} of {entry.file}...",
                        file=sys.stderr,
                    )
                    linecnt += 1
                    continue

                xmin, xmax = min(xmin, xval), max(xmax, xval)
                ymin, ymax = min(ymin, yval), max(ymax, yval)
                linecnt += 1

    if not math.isfinite(xmin):
        raise ValueError("no valid xy data found in any --input file")
    return xmin, xmax, ymin, ymax


def _resolve_bounds(
    xmin: float, xmax: float, ymin: float, ymax: float, region: Optional[str]
) -> Tuple[float, float, float, float, str]:
    """Port of mbm_xyplot's -R handling (lines ~587-635): a plain
    "w/e/s/n" region overrides xmin/xmax/ymin/ymax (each field individually
    run through GetDecimalDegrees, so dd:mm:ss values are accepted) but is
    otherwise passed through to GMT's own -R verbatim; a "...r" suffixed
    region gives two corner points instead of a w/e/s/n range (also passed
    through verbatim -- GMT's -R itself understands the trailing "r"); a
    bare "r" reformats the already-known data bounds into that corner-point
    form. With no -R at all, a fresh "w/e/s/n" string is built from the
    data bounds.

    NOTE: mbm_xyplot's own Perl silently falls through to the data-bounds
    default if -R matches none of these three forms (a malformed -R value
    is simply ignored with no warning at all, because none of its 'if'/
    'elsif' branches assign $bounds_plot, which is only caught by a later
    independent "if (!$bounds_plot)" default-fill check). Raising here
    instead is a deliberate improvement -- failing loudly on a malformed
    --region beats silently discarding the user's actual request.

    A second, separate deliberate addition not present in mbm_xyplot at
    all: a degenerate region (xmin == xmax, or ymin == ymax -- e.g. a real
    "heave" column that is exactly 0.0 for an entire dataset because that
    sensor never logged a nonzero value) is padded by a small margin
    rather than passed through as-is. mbm_xyplot itself has no such
    handling either, but neither classic nor modern GMT can compute a
    projection over a zero-width axis (`gmt mapproject`/`gmt basemap`
    simply error out: "unable to init projection"-style failures),
    so an unpadded degenerate region does not faithfully reproduce a
    quirky-but-working classic-mode plot -- it can only ever crash,
    Perl and Python alike. Padding is the only way to actually get a
    (flat-line) plot out for this real, unremarkable case.
    """
    if not region:
        xmin, xmax = _pad_degenerate(xmin, xmax)
        ymin, ymax = _pad_degenerate(ymin, ymax)
        return xmin, xmax, ymin, ymax, f"{xmin:.8g}/{xmax:.8g}/{ymin:.8g}/{ymax:.8g}"

    m = re.match(r"^(\S+)/(\S+)/(\S+)/(\S+)r$", region)
    if m:
        xmin_raw, ymin_raw, xmax_raw, ymax_raw = m.groups()
        xmin = layout.get_decimal_degrees(xmin_raw)
        xmax = layout.get_decimal_degrees(xmax_raw)
        ymin = layout.get_decimal_degrees(ymin_raw)
        ymax = layout.get_decimal_degrees(ymax_raw)
        if xmin == xmax or ymin == ymax:
            xmin, xmax = _pad_degenerate(xmin, xmax)
            ymin, ymax = _pad_degenerate(ymin, ymax)
            region = f"{xmin:.8g}/{ymin:.8g}/{xmax:.8g}/{ymax:.8g}r"
        return xmin, xmax, ymin, ymax, region

    m = re.match(r"^(\S+)/(\S+)/(\S+)/(\S+)$", region)
    if m:
        xmin_raw, xmax_raw, ymin_raw, ymax_raw = m.groups()
        xmin = layout.get_decimal_degrees(xmin_raw)
        xmax = layout.get_decimal_degrees(xmax_raw)
        ymin = layout.get_decimal_degrees(ymin_raw)
        ymax = layout.get_decimal_degrees(ymax_raw)
        if xmin == xmax or ymin == ymax:
            xmin, xmax = _pad_degenerate(xmin, xmax)
            ymin, ymax = _pad_degenerate(ymin, ymax)
            region = f"{xmin:.8g}/{xmax:.8g}/{ymin:.8g}/{ymax:.8g}"
        return xmin, xmax, ymin, ymax, region

    if region == "r":
        xmin, xmax = _pad_degenerate(xmin, xmax)
        ymin, ymax = _pad_degenerate(ymin, ymax)
        return xmin, xmax, ymin, ymax, f"{xmin:.8g}/{ymin:.8g}/{xmax:.8g}/{ymax:.8g}r"

    raise ValueError(f"unrecognized --region spec: {region!r}")


def _pad_degenerate(vmin: float, vmax: float) -> Tuple[float, float]:
    """If vmin == vmax (a constant data column, e.g. an all-zero heave
    trace), pad symmetrically so the value plots as a visible flat line
    instead of a zero-width axis that GMT cannot project at all. See
    _resolve_bounds()'s docstring."""
    if vmin != vmax:
        return vmin, vmax
    pad = abs(vmin) * 0.05 or 1.0
    return vmin - pad, vmax + pad


@dataclass
class LayoutXY:
    pagesize: str
    projection: str
    projection_pars: str
    region: str            # the resolved -R value (verbatim user string, or data-derived)
    xoffset: float
    yoffset: float
    axes_interval: str      # tick portion only of -B (no WESN/title -- see generate_script_xy)
    degree_format: str


def compute_layout_xy(
    data_xmin: float, data_xmax: float, data_ymin: float, data_ymax: float,
    pagesize: str = "a",
    map_scale: Optional[str] = None,
    orientation: int = 0,
    region: Optional[str] = None,
    tick_info: Optional[str] = None,
    xlabel: str = " ",
    ylabel: str = " ",
) -> LayoutXY:
    """Port of mbm_xyplot's page/scale-fit algorithm (lines ~587-1254).
    `data_xmin/xmax/ymin/ymax` are only used as the auto-fit bounds when
    `region` is not given -- see _resolve_bounds(). This is otherwise the
    same algorithm as mbpy_histplot_script.compute_layout_hist(): see this
    module's own top docstring for why the two are ported jointly.
    """
    xmin, xmax, ymin, ymax, bounds_plot = _resolve_bounds(data_xmin, data_xmax, data_ymin, data_ymax, region)

    # get plot degree annotation format (mbm_xyplot lines ~1042-1068; the
    # "!$gridprojected" guard there is unconditionally true -- mbm_xyplot
    # has no grid at all and never sets $gridprojected -- so it always runs)
    xsize, ysize = xmax - xmin, ymax - ymin
    size = min(xsize, ysize)
    if size > 1.0:
        degree_format = "ddd"
    elif size > (1.0 / 60.0):
        degree_format = "ddd:mm"
    else:
        degree_format = "ddd:mm:ss"

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

        if (use_scale and plot_scale) or (use_width and plot_width):
            pass
        elif use_scale or use_width:
            projection_pars = f"{projection_pars}{separator}{trial_value}"
    else:
        projection, projection_pars = "x", "1/1"
        use_scale = True

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

    return LayoutXY(
        pagesize=pagesize, projection=projection, projection_pars=projection_pars,
        region=bounds_plot, xoffset=xoffset, yoffset=yoffset,
        axes_interval=axes_interval, degree_format=degree_format,
    )


@dataclass
class ScriptOptionsXY:
    entries: List[XYFileEntry]
    root: str
    layout: LayoutXY
    output_format: str = "pdf"
    title: Optional[str] = None
    dpi: Optional[int] = None
    length_scale: Optional[str] = None   # -MGL, emitted as -L<value> on `gmt basemap`
    coast: Optional[CoastOptions] = None
    text_labels: List[str] = field(default_factory=list)   # raw "tx/ty/tsize/tangle/font/just/txt"
    gmt_defs: List[str] = field(default_factory=list)


# A conservative "is this a plain decimal/scientific number" pattern, used
# to validate each computed x/y value in the awk pipeline below -- see
# _plot_pipeline()'s docstring for why this real check (absent from
# mbm_xyplot itself) is necessary here.
_AWK_NUMERIC_RE = r"^[+-]?([0-9]+\.?[0-9]*|\.[0-9]+)([eE][+-]?[0-9]+)?$"


def _plot_pipeline(entry: XYFileEntry, j: str, r: str) -> str:
    """Build the `awk '...' file | gmt plot ...` pipeline for one input
    file: a live pipe (no file written to disk at any point) that applies
    the entry's delimiter-splitting and x/y column-expression, mirroring
    what mbm_xyplot's own preprocessing pass wrote into a persistent
    "<file><n>.<pid>" temp file instead (see this module's top docstring).
    A line containing ">" (a segment header) is passed through unchanged,
    matching mbm_xyplot's own segment-passthrough.

    Unlike mbm_xyplot's own per-line numeric-result validation (a no-op
    regex -- see scan_xy_bounds()'s docstring), the check here is real: it
    was added after a genuine, reproducible bug surfaced in testing. A CSV
    file with an ordinary text header row (e.g. "id,lon,lat,depth") is
    common real-world input; mbm_xyplot's own vestigial check lets that
    header row's non-numeric evaluated "value" through unfiltered into its
    temp file, same as this port's awk pipeline would without this
    check -- but `gmt plot`/`psxy` infers each column's data type from the
    *first* record it reads, and a text value there makes it treat the
    entire file as headerless trailing text, silently discarding every
    following (perfectly valid, numeric) row instead of just the one bad
    line. Confirmed directly: a 20-row CSV with one such header line
    rendered only a handful of stray points before this check was added.
    Skipping non-numeric rows here (matching what scan_xy_bounds() already
    does correctly on the Python side) fixes it.
    """
    awk_x = _expr_to_awk(entry.xexpr)
    awk_y = _expr_to_awk(entry.yexpr)
    begin = 'OFMT="%.10g"'
    strip = ""
    if entry.delimiter:
        begin += f"; FS={_awk_string_literal(entry.delimiter)}"
        strip = f'sub(/^{_awk_regex_escape(entry.delimiter)}/, ""); '
    awk_prog = (
        f'BEGIN{{{begin}}} '
        f'/^>/{{print; next}} '
        f'{{{strip}x=({awk_x}); y=({awk_y}); '
        f'if (x ~ /{_AWK_NUMERIC_RE}/ && y ~ /{_AWK_NUMERIC_RE}/) print x, y; '
        f'else print "WARNING!!! NON-NUMERIC RESULT DETECTED! Skipping line " NR > "/dev/stderr"}}'
    )

    parts = [f"awk {shlex.quote(awk_prog)} {shlex.quote(entry.file)} | gmt plot {j} {r}"]
    if entry.fill != "N":
        parts.append(f"-G{entry.fill}")
    if entry.symbol != "N":
        parts.append(f"-S{entry.symbol}")
    if entry.pen != "N":
        parts.append(f"-W{entry.pen}")
    return " ".join(parts)


def _basemap_axes_xy(opts: ScriptOptionsXY) -> str:
    """Port of the -B assembly at mbm_xyplot lines ~1234-1254 (WESN forced
    on, as for mbpy_grdplot -- see that module's _basemap_axes() docstring
    for why modern mode requires it here where classic mode did not)."""
    frame = "-BWESN" + (f'+t"{opts.title}"' if opts.title else "")
    return f"{frame} {opts.layout.axes_interval}"


def generate_script_xy(opts: ScriptOptionsXY) -> str:
    """Build the full GMT modern-mode shellscript text for `opts`. Touches
    no files at all -- not even a temporary one; see this module's own
    top docstring."""
    lay = opts.layout
    j = f"-J{lay.projection}{lay.projection_pars}"
    r = f"-R{lay.region}"

    lines = [
        "#!/usr/bin/env bash",
        "#",
        "# GMT modern-mode xy plotting script",
        "# Generated by mbpy_xyplot_script.py (Python port of mbm_xyplot)",
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
    for gmt_def in baseline_gmt_defaults(lay.pagesize, False, lay.degree_format):
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

    # Order matches mbm_xyplot's own layer order exactly: xy data plots,
    # then coastline, then text labels, then the basemap frame last (mbm_
    # xyplot lines ~1256-1387) -- unlike mbpy_grdplot, which draws its
    # basemap frame *before* xy overlays/text (see mbpy_grdplot_script.py's
    # generate_script()); each macro's own stacking order is preserved
    # rather than unified, since it affects which layer's ink ends up on
    # top in the final plot.
    if opts.entries:
        lines.append("# Make xy data plot(s)")
        for entry in opts.entries:
            lines.append(_plot_pipeline(entry, j, r))
        lines.append("")

    if opts.coast is not None:
        co = opts.coast
        lines.append("# Make coastline data plot")
        cparts = [f"gmt coast {j} {r}"]
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

    if opts.text_labels:
        lines.append("# Make text labels")
        lines.append(f"gmt text {j} {r} -F+f+a+j <<EOF")
        for raw in opts.text_labels:
            tx, ty, tsize, tangle, font, just, txt = raw.split("/", 6)
            lines.append(f"{tx} {ty} {tsize},{font} {tangle} {just} {txt}")
        lines.append("EOF")
        lines.append("")

    lines.append("# Make basemap")
    basemap_parts = [f"gmt basemap {j} {r} {_basemap_axes_xy(opts)}"]
    if opts.length_scale:
        basemap_parts.append(f"-L{opts.length_scale}")
    lines.append(" ".join(basemap_parts))
    lines.append("")

    lines.append("gmt end")
    lines.append("")
    return "\n".join(lines)


def generate_and_write_xy(opts: ScriptOptionsXY, script_path: str) -> None:
    Path(script_path).write_text(generate_script_xy(opts))
