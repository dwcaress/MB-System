#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_plot_script.py
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
#   Data-bounds, color-palette and script-generator logic for mbpy_plot,
#   the Python port of the Perl macro mbm_plot. mbm_plot plots swath
#   data directly (not a grid) with MB-System's GMT supplement modules:
#   `gmt mbswath` for color fill / shaded relief / amplitude / sidescan
#   images (color_mode 1-5) and `gmt mbcontour` for bathymetry contours
#   and annotated navigation. This module writes the same plot as a GMT
#   **modern mode** shellscript (`gmt begin`/`gmt end`, `gmt coast`,
#   `gmt plot`, `gmt colorbar`, `gmt text`, `gmt basemap`), the same way
#   mbpy_grdplot_script.py does for mbm_grdplot. Both modules work
#   inside a modern-mode session (checked with GMT 6.7.0).
#
#   Page layout and map projection fitting are shared with the other
#   mbpy_* macros (mbpy_layout.compute_layout(), fed the expanded swath
#   bounds); mbm_plot's GetBaseTick/GetPageSize/GetProjection are the
#   geographic subset of mbm_grdplot's. What differs from mbm_grdplot,
#   and so lives here:
#   - data bounds come from the swath files' .inf files or mbinfo, and
#     are padded and given a reasonable aspect ratio (mbm_plot lines
#     ~917-1290);
#   - the color interval is computed from the range of the plotted
#     quantity (depth, amplitude or sidescan), and the default palette
#     order puts low values (shallow depths) at the start of the base
#     palette, the reverse of mbm_grdplot, because depth is positive
#     down (mbm_plot lines ~1673-2200);
#   - the histogram color stretch comes from mbhistogram, run on the
#     swath data;
#   - contour and shading controls follow mbswath/mbcontour's own -A
#     and -G syntax.
#
#   Fixes relative to mbm_plot as it was before 3 November 2026 (the
#   same fixes were then made in mbm_plot itself):
#   - default amplitude shading for color_mode 3 (-G3 without -A):
#     mbm_plot tested a bareword (`!stretch_shade`, always false), so it
#     always used magnitude 1 / zero level 191 even without -S../1,
#     which shades the whole swath black for dB amplitudes. Here the
#     amplitude-range-derived default is used unless the shade is
#     stretched, as mbm_plot intended;
#   - -Amagnitude without "/azimuth" left magnitude unset in mbm_plot;
#     here the other field keeps its default;
#   - -D../1 (shade flip) for color_mode 2 was undone when no -A was
#     given; here it always turns the illumination azimuth by 180 deg;
#   - a six-field -C contour control kept its label height;
#   - "center" in -MGL is replaced by the map center lon/lat;
#   - the -MGQ resolution is passed to mbswath as -E<dpi> (not -Q);
#   - the reversed discrete palette no longer drops its first segment;
#   - the stretched-shade CPT always has the same number of entries as
#     the grayscale palette, whatever -W ncolors is.
#
import math
import re
import shlex
import subprocess
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import mbpy_color as color
from mbpy_color import CptLine
from mbpy_layout import Layout
from mbpy_misc import CoastOptions, NavigationOptions, XYOverlayEntry
from mbpy_grdplot_script import baseline_gmt_defaults, write_cpt_file

NCPT = color.NCPT

COLOR_MODE_NAMES = {
    1: "Bathymetry Color Fill",
    2: "Bathymetry Color Shaded Relief",
    3: "Bathymetry Color Shaded by Amplitude",
    4: "Amplitude Fill",
    5: "Sidescan Fill",
}

# mbm_plot's default color scale labels per color_mode
DEFAULT_SCALE_LABELS = {
    1: "Depth (meters)", 2: "Depth (meters)", 3: "Depth (meters)",
    4: "Beam Amplitude Values", 5: "Sidescan Pixel Values",
}


# ---------------------------------------------------------------------------
# Data bounds
# ---------------------------------------------------------------------------

@dataclass
class SwathBounds:
    xmin: float
    xmax: float
    ymin: float
    ymax: float
    zmin: Optional[float] = None    # depth
    zmax: Optional[float] = None
    amin: Optional[float] = None    # beam amplitude
    amax: Optional[float] = None
    smin: Optional[float] = None    # sidescan
    smax: Optional[float] = None
    nrec: int = 0


_INFO_PATTERNS = {
    "nrec": re.compile(r"^Number of Records:\s+(\S+)"),
    "lon": re.compile(r"^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)"),
    "lat": re.compile(r"^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)"),
    "depth": re.compile(r"^Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)"),
    "amp": re.compile(r"^Minimum Amplitude:\s+(\S+)\s+Maximum Amplitude:\s+(\S+)"),
    "ss": re.compile(r"^Minimum Sidescan:\s+(\S+)\s+Maximum Sidescan:\s+(\S+)"),
}


def parse_mbinfo(text: str) -> Optional[SwathBounds]:
    """Parse mbinfo (or .inf file) text; None if it has no lon/lat bounds."""
    vals = {}
    for line in text.splitlines():
        for key, pat in _INFO_PATTERNS.items():
            m = pat.match(line)
            if m:
                vals[key] = tuple(float(g) for g in m.groups())
    if "lon" not in vals or "lat" not in vals:
        return None
    b = SwathBounds(vals["lon"][0], vals["lon"][1], vals["lat"][0], vals["lat"][1],
                    nrec=int(vals.get("nrec", (0,))[0]))
    if "depth" in vals:
        b.zmin, b.zmax = vals["depth"]
    if "amp" in vals:
        b.amin, b.amax = vals["amp"]
    if "ss" in vals:
        b.smin, b.smax = vals["ss"]
    return b


def list_swath_files(file_data: str, fmt: int, region: Optional[str] = None) -> List[Tuple[str, int]]:
    """The (file, format) pairs to plot: the file itself, or the
    datalist's entries as reported by mbdatalist (mbm_plot ~925-947)."""
    if fmt >= 0:
        return [(file_data, fmt)]
    cmd = ["mbdatalist", "-F-1", "-I", file_data]
    if region:
        cmd.append(f"-R{region}")
    out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
    files = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            files.append((parts[0], int(parts[1])))
    return files


def apply_lonflip(xmin: float, xmax: float, lonflip: int) -> Tuple[float, float]:
    """Port of mbm_plot's per-file longitude reset (lines ~1071-1112)."""
    if lonflip == 0 and ((xmin < -180.0 < xmax) or (xmin < 180.0 < xmax)):
        return -180.0, 180.0
    if lonflip == 0 and xmin < -180.0:
        return xmin + 360.0, xmax + 360.0
    if lonflip == 0 and xmax > 180.0:
        return xmin - 360.0, xmax - 360.0
    if lonflip == -1 and xmin < 0.0 < xmax:
        return -360.0, 0.0
    if lonflip == -1 and xmax > 0.0:
        return xmin - 360.0, xmax - 360.0
    if lonflip == 1 and xmin < 0.0 < xmax:
        return 0.0, 360.0
    if lonflip == 1 and xmin < 0.0:
        return xmin + 360.0, xmax + 360.0
    return xmin, xmax


def _combine(a: Optional[float], b: Optional[float], fn) -> Optional[float]:
    if a is None:
        return b
    if b is None:
        return a
    return fn(a, b)


def get_swath_bounds(
    files: List[Tuple[str, int]],
    region_bounds: Optional[Tuple[float, float, float, float]] = None,
    region: Optional[str] = None,
    btime: Optional[str] = None,
    etime: Optional[str] = None,
    lonflip: int = 0,
    color_mode: int = 0,
    verbose: bool = False,
) -> SwathBounds:
    """Port of mbm_plot's data-bounds loop (lines ~948-1135): use each
    file's .inf file when it is readable, no time window is applied and
    (with a region) the file does not straddle the region while a color
    image is wanted; otherwise run `mbinfo -G` on the file. Files with
    no records are skipped."""
    total: Optional[SwathBounds] = None
    for path, fmt in files:
        info = None
        inf = Path(f"{path}.inf")
        if inf.is_file() and not btime and not etime:
            b = parse_mbinfo(inf.read_text(errors="replace"))
            if b is not None:
                use = True
                if region_bounds and b.nrec > 0 and color_mode:
                    xmin, xmax, ymin, ymax = region_bounds
                    inside = (xmin <= b.xmin <= xmax and xmin <= b.xmax <= xmax
                              and ymin <= b.ymin <= ymax and ymin <= b.ymax <= ymax)
                    outside = b.xmax < xmin or b.xmin > xmax or b.ymax < ymin or b.ymin > ymax
                    use = inside or outside
                if use:
                    info = b
                    if verbose:
                        print(f"Reading mbinfo output from file {inf}...")
        if info is None:
            cmd = ["mbinfo", f"-F{fmt}", "-I", path, "-G"]
            if btime:
                cmd.append(f"-B{btime}")
            if etime:
                cmd.append(f"-E{etime}")
            if region:
                cmd.append(f"-R{region}")
            if verbose:
                print("Running " + " ".join(shlex.quote(c) for c in cmd))
            out = subprocess.run(cmd, capture_output=True, text=True, errors="replace").stdout
            info = parse_mbinfo(out)
        if info is None or info.nrec <= 0:
            continue
        info.xmin, info.xmax = apply_lonflip(info.xmin, info.xmax, lonflip)
        if total is None:
            total = info
        else:
            total = SwathBounds(
                min(total.xmin, info.xmin), max(total.xmax, info.xmax),
                min(total.ymin, info.ymin), max(total.ymax, info.ymax),
                _combine(total.zmin, info.zmin, min), _combine(total.zmax, info.zmax, max),
                _combine(total.amin, info.amin, min), _combine(total.amax, info.amax, max),
                _combine(total.smin, info.smin, min), _combine(total.smax, info.smax, max),
                total.nrec + info.nrec,
            )
    if total is None:
        raise RuntimeError("does not appear to be any data in the input")
    return total


def expand_bounds(xmin: float, xmax: float, ymin: float, ymax: float) -> Tuple[float, float, float, float]:
    """Port of mbm_plot lines ~1220-1290: pad the data bounds by 5% and
    widen the shorter side (in approximate meters) so the map's aspect
    ratio is at most 2:1; latitudes are kept inside +/-89 degrees."""
    delx = 0.05 * (xmax - xmin)
    dely = 0.05 * (ymax - ymin)
    if (xmax - xmin) < 0.95 * 360.0:
        xmin, xmax = xmin - delx, xmax + delx
    ymin, ymax = max(ymin - dely, -89.0), min(ymax + dely, 89.0)

    c1, c2, c3 = 111412.84, -93.5, 0.118
    c4, c5, c6, c7 = 111132.92, -559.82, 1.175, 0.0023
    radlat = 0.5 * (ymax + ymin) * math.pi / 180.0
    mtodeglat = 1.0 / abs(c4 + c5 * math.cos(2 * radlat) + c6 * math.cos(4 * radlat) + c7 * math.cos(6 * radlat))
    mtodeglon = 1.0 / abs(c1 * math.cos(radlat) + c2 * math.cos(3 * radlat) + c3 * math.cos(5 * radlat))
    dx = (xmax - xmin) / mtodeglon
    dy = (ymax - ymin) / mtodeglat
    if dy / dx > 2.0:
        delx = (0.5 * (0.5 * dy - dx)) * mtodeglon
        xmin, xmax = xmin - delx, xmax + delx
    elif dx / dy > 2.0:
        dely = (0.5 * (0.5 * dx - dy)) * mtodeglat
        ymin, ymax = max(ymin - dely, -89.0), min(ymax + dely, 89.0)
    return xmin, xmax, ymin, ymax


# ---------------------------------------------------------------------------
# Colors
# ---------------------------------------------------------------------------

def nice_interval(dd: float) -> float:
    """The 1/2.5/5 x 10^n "reasonable interval" heuristic shared by
    mbm_plot's contour and color intervals."""
    if dd <= 0:
        return 0.0
    base = int((math.log(dd) / math.log(10.0)) + 0.5)
    interval = (10 ** base) / 10.0
    if dd / interval < 10:
        interval = interval / 4
    elif dd / interval < 20:
        interval = interval / 2
    return interval


def compute_color_interval_plot(
    dmin: float, dmax: float, zmax: float, ncolors_use: int, no_nice: bool = False
) -> Tuple[float, float]:
    """Port of mbm_plot lines ~1696-1720: returns (color_start,
    color_int). Note that, as in mbm_plot, the "does the last color
    reach the top" test compares against the depth maximum `zmax` even
    when the colored quantity is amplitude or sidescan."""
    dd = dmax - dmin
    if not no_nice and dd > 0:
        start_int = nice_interval(dd) / 2
        multiplier = int(dd / (ncolors_use - 1) / start_int) + 1
        color_int = multiplier * start_int
        if color_int * int(dmin / color_int) + color_int * (ncolors_use - 1) < zmax:
            multiplier += 1
            color_int = multiplier * start_int
        return color_int * int(dmin / color_int), color_int
    color_int = 1.02 * dd / (ncolors_use - 1)
    return dmin - 0.01 * dd, color_int


def run_mbhistogram(file_data: str, fmt: int, data_type: int, dmin: float, dmax: float,
                    nintervals: int, gaussian: bool = False) -> List[float]:
    """Run mbhistogram for an equalized color (or Gaussian shade) stretch;
    returns its interval boundaries (nintervals + 1 values, or
    nintervals with -G). data_type: 0 bathymetry, 1 amplitude, 2 sidescan."""
    cmd = ["mbhistogram", f"-F{fmt}", "-I", file_data, f"-A{data_type}",
           f"-D{dmin:.10g}/{dmax:.10g}", f"-M{nintervals}", "-N1000"]
    if gaussian:
        cmd.append("-G")
    out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
    return [float(v) for v in out.split()]


def build_cpt_plot(
    colors: List[Tuple[float, float, float]],
    color_style: int,
    color_flip: bool,
    color_start: float = 0.0,
    color_int: float = 1.0,
    boundaries: Optional[List[float]] = None,
) -> List[CptLine]:
    """Port of mbm_plot's four CPT-writing loops (lines ~2085-2200).
    `colors` holds ncolors samples of the base palette. Continuous style
    (1) makes ncolors - 1 graded segments, discrete style ncolors flat
    ones. Unflipped, low z gets colors[0] (the start of the palette,
    e.g. white/red for Haxby: shallow depths); flipped reverses that.
    z boundaries step by color_int from color_start, or are consumed in
    order from `boundaries` (the mbhistogram stretch) when given."""
    ncolors = len(colors)
    nseg = ncolors - 1 if color_style == 1 else ncolors
    if boundaries is not None:
        if len(boundaries) < nseg + 1:
            raise ValueError(f"need {nseg + 1} stretch boundaries, got {len(boundaries)}")
        z = list(boundaries[:nseg + 1])
    else:
        z = [color_start + i * color_int for i in range(nseg + 1)]
    lines = []
    for k in range(nseg):
        if color_style == 1:
            i = k if not color_flip else ncolors - 2 - k
            c1, c2 = (colors[i], colors[i + 1]) if not color_flip else (colors[i + 1], colors[i])
        else:
            i = k if not color_flip else ncolors - 1 - k
            c1 = c2 = colors[i]
        lines.append(CptLine(z[k], *c1, z[k + 1], *c2))
    return lines


def build_shade_cpt(boundaries: List[float], shade_flip: bool) -> List[CptLine]:
    """Port of mbm_plot's amplitude-shading CPT for -G3 with a stretched
    shade (lines ~2210-2270): grayscale palette 4 over the Gaussian
    mbhistogram intervals, used by `mbswath -N`."""
    gray = color.interpolate_palette(4, NCPT)
    return build_cpt_plot(gray, 1, shade_flip, boundaries=boundaries)


# ---------------------------------------------------------------------------
# Controls
# ---------------------------------------------------------------------------

def resolve_contour_control(
    contour_control: Optional[str], contour_int_auto: float, color_mode: int, page_height: float
) -> str:
    """Port of mbm_plot lines ~1761-1845: build mbcontour's
    -A<cont_int>/<color_int>/<tick_int>/<label_int>/<tick_len>/<label_hgt>.
    The default label/tick intervals are 4 x contour interval, or (when
    a color image is drawn beneath) effectively never (100000)."""
    color_default = lambda ci: 100000 if color_mode else 4 * ci
    if not contour_control:
        ci = contour_int_auto
        cci = color_default(ci)
        tick_len = 0.001 * page_height
        label_hgt = 0.01 * page_height
        if tick_len < 0.01 or label_hgt < 0.1:
            tick_len, label_hgt = 0.01, 0.1
        parts = [ci, cci, cci, cci, tick_len, label_hgt]
    else:
        f = contour_control.split("/")
        if len(f) >= 6:
            parts = f[:6]
        elif len(f) == 5:
            parts = f + [0.1]
        elif len(f) == 4:
            parts = f + [0.01, 0.1]
        elif len(f) == 3:
            parts = f + [f[1], 0.01, 0.1]
        elif len(f) == 2:
            parts = f + [f[1], f[1], 0.01, 0.1]
        else:
            cci = color_default(float(f[0]))
            parts = [f[0], cci, cci, cci, 0.01, 0.1]
    return "/".join(v if isinstance(v, str) else f"{v:.6g}" for v in parts)


@dataclass
class ShadeControl:
    """mbswath -G<magnitude>/<azimuth> (color_mode 2), or
    -G<magnitude>/<zero_level> (color_mode 3)."""
    magnitude: float
    second: float

    def as_arg(self) -> str:
        return f"{self.magnitude:.4g}/{self.second:.4g}"


def resolve_shade_control(
    color_mode: int,
    magnitude: Optional[float] = None,
    second: Optional[float] = None,
    amin: Optional[float] = None,
    amax: Optional[float] = None,
    stretch_shade: bool = False,
    shade_flip: bool = False,
) -> ShadeControl:
    """Port of mbm_plot lines ~1921-1945 (see the module header for the
    fixes): `second` is the azimuth for color_mode 2 and the amplitude
    zero level for color_mode 3."""
    if color_mode == 3:
        if stretch_shade:
            dmag, dsec = 1.0, 191.0
        elif amin is not None and amax is not None and amax > amin:
            dmag = 2.0 / (amax - amin)
            dsec = amin + 0.2 * (amax - amin)
        else:
            dmag, dsec = 1.0, 191.0
    else:
        dmag, dsec = 2.5, 0.0
    sc = ShadeControl(dmag if magnitude is None else magnitude, dsec if second is None else second)
    if color_mode == 2 and shade_flip:
        sc.second = sc.second - 180.0 if sc.second > 180.0 else sc.second + 180.0
    return sc


# ---------------------------------------------------------------------------
# Script generation
# ---------------------------------------------------------------------------

@dataclass
class SwathOptions:
    """MB-System data-selection options passed to mbswath/mbcontour
    (mbm_plot -MM...)."""
    footprint: Optional[str] = None      # -MMA -> mbswath -A (default 1)
    btime: Optional[str] = None          # -MMB -> -b
    etime: Optional[str] = None          # -MME -> -E
    swath_scale: Optional[str] = None    # -MMD -> mbswath -D
    contour_nplot: Optional[str] = None  # -MMN -> mbcontour -N
    speedmin: Optional[str] = None       # -MMS -> -S
    timegap: Optional[str] = None        # -MMT -> -T
    contour_algorithm: Optional[str] = None   # -MMZ -> mbcontour -Z
    lonflip: Optional[int] = None        # -MML -> -L


@dataclass
class ScriptOptionsPlot:
    file_data: str
    fmt: int
    root: str
    layout: Layout
    output_format: str = "pdf"
    title: Optional[str] = None
    tick_info: Optional[str] = None
    colorbar_label: Optional[str] = None
    length_scale: Optional[str] = None
    unix_stamp: Optional[str] = None     # "" for a plain -U
    dpi: Optional[int] = None

    color_mode: int = 0                  # 0 none, 1-5 mbswath -Z mode
    use_filtered: bool = False           # mbswath -Z<mode>F
    color_palette: int = 1
    color_flip: bool = False
    cpt_lines: Optional[List[CptLine]] = None
    cpt_file: Optional[str] = None
    shade: Optional[ShadeControl] = None
    shade_cpt_lines: Optional[List[CptLine]] = None

    contour_control: Optional[str] = None     # resolved mbcontour -A string, or None
    navigation: Optional[NavigationOptions] = None   # resolved, or None
    swath: SwathOptions = field(default_factory=SwathOptions)

    coast: Optional[CoastOptions] = None
    xy_entries: List[XYOverlayEntry] = field(default_factory=list)
    text_labels: List[str] = field(default_factory=list)
    gmt_defs: List[str] = field(default_factory=list)

    @property
    def cpt_path(self) -> str:
        return self.cpt_file or f"{self.root}.cpt"

    @property
    def shade_cpt_path(self) -> str:
        return f"{self.root}.shade.cpt"

    @property
    def script_path(self) -> str:
        return f"{self.root}.sh"


def _swath_common(opts: ScriptOptionsPlot) -> List[str]:
    s = opts.swath
    parts = []
    if s.btime:
        parts.append(f"-b{s.btime}")
    if s.etime:
        parts.append(f"-E{s.etime}")
    if s.speedmin:
        parts.append(f"-S{s.speedmin}")
    if s.timegap:
        parts.append(f"-T{s.timegap}")
    if s.lonflip is not None:
        parts.append(f"-L{s.lonflip}")
    return parts


def _basemap_axes(opts: ScriptOptionsPlot) -> str:
    """-BWESN (all four sides annotated, as in mbm_plot's classic-mode
    plots; see mbpy_grdplot_script._basemap_axes()) plus the title,
    then the tick intervals."""
    frame = "-BWESN" + (f"+t{shlex.quote(opts.title)}" if opts.title else "")
    if opts.tick_info:
        return f"{frame} -B{opts.tick_info}"
    tick = opts.layout.base_tick
    if tick.tick_x == tick.tick_y:
        return f"{frame} -B{tick.tick_x}"
    return f"{frame} -Bx{tick.tick_x} -By{tick.tick_y}"


def generate_script_plot(opts: ScriptOptionsPlot, command_line: Optional[str] = None) -> str:
    lay = opts.layout
    q = shlex.quote
    j = f"-J{lay.projection}{lay.projection_pars}"
    r = f"-R{lay.region}"

    lines = [
        "#!/usr/bin/env bash",
        "#",
        f"# GMT modern-mode script to plot swath data from {opts.file_data}",
        "# Generated by mbpy_plot (Python port of mbm_plot)",
    ]
    if command_line:
        lines.append(f"#   {command_line}")
    lines += ["#", "set -e", ""]

    # a single swath file is plotted through a one-line datalist, as in
    # mbm_plot; a datalist is used directly
    if opts.fmt >= 0:
        file_list = f"{opts.root}_datalist.mb-1"
        lines.append("# Make datalist file")
        lines.append(f"echo {q(opts.file_data)} {opts.fmt} > {q(file_list)}")
        lines.append("")
    else:
        file_list = opts.file_data
    file_list_q = q(file_list)

    lines.append(f"gmt begin {q(opts.root)} {opts.output_format}")
    lines.append("")
    if opts.dpi:
        lines.append(f"gmt figure {q(opts.root)} {opts.output_format} E{opts.dpi}")
        lines.append("")

    lines.append("# Set temporary GMT defaults")
    pairs = []
    # mbm_plot does not swap the background/foreground colors for -D
    for gmt_def in baseline_gmt_defaults(lay.pagesize, False, lay.degree_format):
        param, _, value = gmt_def.partition(" ")
        pairs.append(f"{param} {value}")
    lines.append(f"gmt set {' '.join(pairs)}")
    lines.append("")
    if opts.gmt_defs:
        lines.append("# User-defined GMT parameter overrides")
        user_pairs = []
        for gmt_def in opts.gmt_defs:
            param, _, value = gmt_def.partition("/")
            user_pairs.append(f"{param} {value}")
        lines.append(f"gmt set {' '.join(user_pairs)}")
        lines.append("")

    if opts.color_mode:
        zmode = f"{opts.color_mode}F" if opts.use_filtered else f"{opts.color_mode}"
        parts = [f"gmt mbswath -F-1 -I{file_list_q} {j} {r} -C{q(opts.cpt_path)} -Z{zmode}",
                 f"-A{opts.swath.footprint or '1'}"]
        if opts.color_mode in (2, 3) and opts.shade is not None:
            parts.append(f"-G{opts.shade.as_arg()}")
        if opts.color_mode == 3 and opts.shade_cpt_lines is not None:
            parts.append(f"-N{q(opts.shade_cpt_path)}")
        if opts.dpi:
            parts.append(f"-E{opts.dpi}")
        if opts.swath.swath_scale:
            parts.append(f"-D{opts.swath.swath_scale}")
        parts += _swath_common(opts)
        lines.append("# Plot the swath data")
        lines.append(" ".join(parts))
        lines.append("")

    nav = opts.navigation
    if opts.contour_control or nav is not None:
        parts = [f"gmt mbcontour -F-1 -I{file_list_q} {j} {r}"]
        if opts.contour_control:
            if opts.swath.contour_algorithm:
                parts.append(f"-Z{opts.swath.contour_algorithm}")
            elif opts.fmt in (41, 101, 102):
                parts.append("-Z1")
            parts.append(f"-A{opts.contour_control}")
        if nav is not None:
            parts.append(f"-D{nav.navigation_control}")
            if nav.name_mode:
                parts.append(f"-G{nav.nav_name_hgt}/1" if nav.name_perp else f"-G{nav.nav_name_hgt}")
            if nav.pingnumber_mode:
                parts.append(f"-M{nav.pingnumber_control}")
        parts += _swath_common(opts)
        if opts.swath.contour_nplot:
            parts.append(f"-N{opts.swath.contour_nplot}")
        lines.append("# Plot contours and/or navigation")
        lines.append(" ".join(parts))
        lines.append("")

    if opts.coast is not None:
        co = opts.coast
        parts = [f"gmt coast {j} {r}"]
        if co.coast_lakefill:
            parts.append(f"-C{co.coast_lakefill}")
        if co.coast_resolution:
            parts.append(f"-D{co.coast_resolution}")
        if co.coast_dryfill:
            parts.append(f"-G{co.coast_dryfill}")
        if co.coast_river:
            parts.append(f"-I{co.coast_river}")
        for boundary in co.coast_boundaries:
            parts.append(f"-N{boundary}")
        if co.coast_wetfill:
            parts.append(f"-S{co.coast_wetfill}")
        if co.coast_pen:
            parts.append(f"-W{co.coast_pen}")
        lines.append("# Plot coastline")
        lines.append(" ".join(parts))
        lines.append("")

    for entry in opts.xy_entries:
        parts = [f"gmt plot {q(entry.file)} {j} {r}"]
        if entry.fill != "N":
            parts.append(f"-G{entry.fill}")
        if entry.symbol != "N":
            parts.append(f"-S{entry.symbol}")
        if entry.pen != "N":
            parts.append(f"-W{entry.pen}")
        lines.append(" ".join(parts))
    if opts.xy_entries:
        lines.append("")

    if opts.color_mode and opts.color_palette < 5:
        # same placement as mbpy_grdplot (see its colorbar comment)
        label = opts.colorbar_label if opts.colorbar_label is not None \
            else DEFAULT_SCALE_LABELS[opts.color_mode]
        if lay.colorscale_vh == "+v":
            d_xy = f"{lay.colorscale_offx:.5g}/0"
        else:
            d_xy = f"0/{lay.colorscale_offy:.5g}"
        lines.append("# Plot color scale")
        lines.append(
            f"gmt colorbar -C{q(opts.cpt_path)} -Dx{d_xy}{lay.colorscale_vh}"
            f"+w{lay.colorscale_length:.5g}/{lay.colorscale_thick:.5g} -B+l{q(label)}"
        )
        lines.append("")

    if opts.text_labels:
        # see mbpy_grdplot_script.py for the modern-mode -F+f+a+j record layout
        lines.append(f"gmt text {j} {r} -F+f+a+j <<EOF")
        for raw in opts.text_labels:
            tx, ty, tsize, tangle, font, just, txt = raw.split("/", 6)
            lines.append(f"{tx} {ty} {tsize},{font} {tangle} {just} {txt}")
        lines.append("EOF")
        lines.append("")

    parts = [f"gmt basemap {j} {r} {_basemap_axes(opts)}"]
    if opts.length_scale:
        parts.append(f"-L{opts.length_scale}")
    if opts.unix_stamp is not None:
        parts.append(f"-U{q(opts.unix_stamp)}" if opts.unix_stamp else "-U")
    lines.append(" ".join(parts))
    lines.append("")
    lines.append("gmt end")
    lines.append("")

    cleanup = []
    if opts.fmt >= 0:
        cleanup.append(file_list_q)
    if cleanup:
        lines.append("# Delete surplus files")
        lines.append("rm -f " + " ".join(cleanup))
        lines.append("")
    return "\n".join(lines)


def generate_and_write_plot(opts: ScriptOptionsPlot, command_line: Optional[str] = None) -> str:
    """Write the CPT files (when generated) and the shellscript; make
    the script executable and return its path."""
    if opts.cpt_lines is not None and opts.cpt_file is None:
        write_cpt_file(opts.cpt_path, opts.cpt_lines)
    if opts.shade_cpt_lines is not None:
        write_cpt_file(opts.shade_cpt_path, opts.shade_cpt_lines)
    path = Path(opts.script_path)
    path.write_text(generate_script_plot(opts, command_line))
    path.chmod(0o755)
    return str(path)
