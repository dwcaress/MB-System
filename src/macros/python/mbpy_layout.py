#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_layout.py
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
#   Python port of the page-size / projection / scale / tick-interval
#   heuristics from src/macros/mbm_grdplot (the Perl subs GetPageSize,
#   GetProjection, and GetBaseTick, plus the auto-layout algorithm in
#   the main body that decides page size, orientation, plot scale,
#   plot origin, color scale placement, contour interval, and degree
#   annotation format).
#
#   This module only computes the *layout*: it does not generate a
#   GMT (classic or modern mode) script and does not build color
#   palettes. It is meant to be validated numerically against the
#   Perl macro's output before it is wired into a script- or
#   PyGMT-based backend.
#
# Status:
#   Ported and cross-checked against `mbm_grdplot` (Perl) output for
#   several synthetic grids -- see test_mbpy_layout.py.
#
import math
import re
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


# ---------------------------------------------------------------------------
# Page size database (verbatim from mbm_grdplot: @page_size_names,
# %page_width_in, %page_height_in). Dimensions are portrait width/height
# in inches.
# ---------------------------------------------------------------------------

PAGE_SIZE_NAMES = [
    "a", "b", "c", "d", "e", "f", "e1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "b10",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "m1", "m2", "m3", "m4", "m5", "m6",
]

PAGE_WIDTH_IN = {
    "a": 8.50, "b": 11.00, "c": 17.00, "d": 22.00,
    "e": 34.00, "f": 28.00, "e1": 44.00, "a0": 33.11,
    "a1": 23.39, "a2": 16.54, "a3": 11.69, "a4": 8.27,
    "a5": 5.83, "a6": 4.13, "a7": 2.91, "a8": 2.05,
    "a9": 1.46, "a10": 1.02, "b0": 39.37, "b1": 27.83,
    "b2": 19.68, "b3": 13.90, "b4": 9.84, "b5": 6.93,
    "b6": 4.92, "b7": 3.46, "b8": 2.44, "b9": 1.73,
    "b10": 1.22, "c0": 36.00, "c1": 25.60, "c2": 18.00,
    "c3": 12.80, "c4": 9.00, "c5": 6.40, "c6": 4.50,
    "c7": 3.20, "m1": 54.00, "m2": 54.00, "m3": 54.00,
    "m4": 60.00, "m5": 60.00, "m6": 60.00,
}

PAGE_HEIGHT_IN = {
    "a": 11.00, "b": 17.00, "c": 22.00, "d": 34.00,
    "e": 44.00, "f": 40.00, "e1": 68.00, "a0": 46.81,
    "a1": 33.11, "a2": 23.39, "a3": 16.54, "a4": 11.69,
    "a5": 8.27, "a6": 5.83, "a7": 4.13, "a8": 2.91,
    "a9": 2.05, "a10": 1.46, "b0": 56.67, "b1": 39.37,
    "b2": 27.83, "b3": 19.68, "b4": 13.90, "b5": 9.84,
    "b6": 6.93, "b7": 4.92, "b8": 3.46, "b9": 2.44,
    "b10": 1.73, "c0": 51.20, "c1": 36.00, "c2": 25.60,
    "c3": 18.00, "c4": 12.80, "c5": 9.00, "c6": 6.40,
    "c7": 4.50, "m1": 72.00, "m2": 84.00, "m3": 96.00,
    "m4": 72.00, "m5": 84.00, "m6": 96.00,
}


def get_decimal_degrees(value: str) -> float:
    """Port of GetDecimalDegrees: accepts dd, dd:mm, or dd:mm:ss strings."""
    s = value.strip()
    m = re.match(r"^(\S+):(\S+):(\S+)$", s)
    if m:
        degrees, minutes, seconds = m.groups()
        d = float(degrees)
        mm = float(minutes)
        ss = float(seconds)
        if degrees.startswith("-"):
            return d - mm / 60.0 - ss / 3600.0
        return d + mm / 60.0 + ss / 3600.0
    m = re.match(r"^(\S+):(\S+)$", s)
    if m:
        degrees, minutes = m.groups()
        d = float(degrees)
        mm = float(minutes)
        if degrees.startswith("-"):
            return d - mm / 60.0
        return d + mm / 60.0
    return float(s)


# ---------------------------------------------------------------------------
# GetPageSize
# ---------------------------------------------------------------------------

@dataclass
class PageMargins:
    space_top: float
    space_bottom: float
    space_left: float
    space_right: float
    width_max_landscape: float
    height_max_landscape: float
    width_max_portrait: float
    height_max_portrait: float


def get_page_size(pagesize: str, scale_loc: str = "b") -> PageMargins:
    """Port of sub GetPageSize.

    scale_loc: 'l' (left), 'r' (right), 't' (top), or anything else
    (bottom, the mbm_grdplot default).
    """
    page_h = PAGE_HEIGHT_IN[pagesize]
    page_w = PAGE_WIDTH_IN[pagesize]
    a_h = PAGE_HEIGHT_IN["a"]

    if scale_loc == "l":
        space_top = min(1.50 * page_h / a_h, 4.50)
        space_bottom = min(0.75 * page_h / a_h, 2.25)
        space_left = min(2.50 * page_h / a_h, 7.50)
        space_right = min(1.00 * page_h / a_h, 3.00)
    elif scale_loc == "r":
        space_top = min(1.50 * page_h / a_h, 4.50)
        space_bottom = min(0.75 * page_h / a_h, 2.25)
        space_left = min(1.00 * page_h / a_h, 3.00)
        space_right = min(2.50 * page_h / a_h, 7.50)
    elif scale_loc == "t":
        space_top = min(2.75 * page_h / a_h, 8.25)
        space_bottom = min(0.75 * page_h / a_h, 2.25)
        space_left = min(1.00 * page_h / a_h, 3.00)
        space_right = min(1.00 * page_h / a_h, 3.00)
    else:
        space_top = min(1.50 * page_h / a_h, 4.50)
        space_bottom = min(2.00 * page_h / a_h, 6.00)
        space_left = min(1.00 * page_h / a_h, 3.00)
        space_right = min(1.00 * page_h / a_h, 3.00)

    width_max_landscape = page_h - space_left - space_right
    height_max_landscape = page_w - space_bottom - space_top
    width_max_portrait = page_w - space_left - space_right
    height_max_portrait = page_h - space_bottom - space_top

    return PageMargins(
        space_top, space_bottom, space_left, space_right,
        width_max_landscape, height_max_landscape,
        width_max_portrait, height_max_portrait,
    )


# ---------------------------------------------------------------------------
# GetProjection: parse a GMT -J projection spec of the form used by
# mbm_grdplot's $map_scale (e.g. "m1.0", "M6i", "x0.01/0.01", "Q-122/6i").
# Returns (projection_letter, projection_pars, use_scale, use_width,
#          separator, trial_value, use_ratio, linear, plot_scale_or_width)
# ---------------------------------------------------------------------------

@dataclass
class ProjectionSpec:
    projection: str
    projection_pars: str
    use_scale: bool
    use_width: bool
    use_ratio: bool
    linear: bool
    separator: str
    trial_value: str
    plot_scale: Optional[float] = None
    plot_width: Optional[float] = None


def get_projection(map_scale: str) -> ProjectionSpec:
    """Port of sub GetProjection.

    map_scale is the value of -J as mbm_grdplot expects it, e.g.
    "m1.0", "M6i", "x0.02", "Xd", "q-122/1.0".
    """
    m = re.match(r"^(\w)(.*)$", map_scale)
    if not m:
        raise ValueError(f"unparseable map_scale: {map_scale!r}")
    projection, rest = m.group(1), m.group(2)

    use_scale = False
    use_width = False
    use_ratio = False
    linear = False
    separator = "/"
    trial_value = "1.0"
    plot_scale = None
    plot_width = None

    def tail_number(pattern: str) -> Optional[float]:
        mm = re.match(pattern, map_scale)
        return float(mm.group(1)) if mm else None

    if projection == "c":
        plot_scale = tail_number(r"^c\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "C":
        plot_width = tail_number(r"^C\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "m":
        plot_scale = tail_number(r"^m([\d.eE+-]+)$")
        use_scale = True
        separator = ""
    elif projection == "M":
        plot_width = tail_number(r"^M([\d.eE+-]+)$")
        use_width = True
        separator = ""
    elif projection == "o":
        if map_scale.startswith("oa"):
            plot_scale = tail_number(r"^oa\S+/\S+/\S+/([\d.eE+-]+)$")
        elif map_scale.startswith("ob"):
            plot_scale = tail_number(r"^ob\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        elif map_scale.startswith("oc"):
            plot_scale = tail_number(r"^oc\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "O":
        if map_scale.startswith("Oa"):
            plot_width = tail_number(r"^Oa\S+/\S+/\S+/([\d.eE+-]+)$")
        elif map_scale.startswith("Ob"):
            plot_width = tail_number(r"^Ob\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        elif map_scale.startswith("Oc"):
            plot_width = tail_number(r"^Oc\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "q":
        plot_scale = tail_number(r"^q\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "Q":
        plot_width = tail_number(r"^Q\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "t":
        plot_scale = tail_number(r"^t\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "T":
        plot_width = tail_number(r"^T\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "u":
        plot_scale = tail_number(r"^u\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "U":
        plot_width = tail_number(r"^U\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "y":
        plot_scale = tail_number(r"^y\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "Y":
        plot_width = tail_number(r"^Y\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "a":
        plot_scale = tail_number(r"^a\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
        trial_value = "1:1"
        use_ratio = True
    elif projection == "A":
        plot_width = tail_number(r"^A\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "e":
        plot_scale = tail_number(r"^e\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
        trial_value = "1:1"
        use_ratio = True
    elif projection == "E":
        plot_width = tail_number(r"^E\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "g":
        plot_scale = tail_number(r"^g\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
        trial_value = "1:1"
        use_ratio = True
    elif projection == "G":
        plot_width = tail_number(r"^G\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "s":
        plot_scale = tail_number(r"^s\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
        trial_value = "1:1"
        use_ratio = True
    elif projection == "S":
        plot_width = tail_number(r"^S\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "b":
        plot_scale = tail_number(r"^b\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "B":
        plot_width = tail_number(r"^B\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "l":
        plot_scale = tail_number(r"^l\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "L":
        plot_width = tail_number(r"^L\S+/\S+/\S+/\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "h":
        plot_scale = tail_number(r"^h\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "H":
        plot_width = tail_number(r"^H\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "i":
        plot_scale = tail_number(r"^i\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "I":
        plot_width = tail_number(r"^I\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "k":
        plot_scale = tail_number(r"^k\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "K":
        plot_width = tail_number(r"^K\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "n":
        plot_scale = tail_number(r"^n\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "N":
        plot_width = tail_number(r"^N\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "r":
        plot_scale = tail_number(r"^r\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "R":
        plot_width = tail_number(r"^R\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "w":
        plot_scale = tail_number(r"^w\S+/([\d.eE+-]+)$")
        use_scale = True
    elif projection == "W":
        plot_width = tail_number(r"^W\S+/([\d.eE+-]+)$")
        use_width = True
    elif projection == "p":
        plot_scale = tail_number(r"^p([\d.eE+-]+)$")
        use_scale = True
        separator = ""
    elif projection == "P":
        plot_width = tail_number(r"^P([\d.eE+-]+)$")
        use_width = True
        separator = ""
    elif projection == "x":
        if map_scale == "xd":
            linear = True
            map_scale = map_scale[:-1]
        else:
            linear = True
            plot_scale = tail_number(r"^x([\d.eE+-]+)$")
        use_scale = True
        separator = ""
    elif projection == "X":
        if map_scale == "Xd":
            linear = True
            map_scale = map_scale[:-1]
        else:
            linear = True
            plot_width = tail_number(r"^X([\d.eE+-]+)$")
        use_width = True
        separator = ""
    else:
        raise ValueError(f"unrecognized projection letter: {projection!r}")

    projection_pars = re.match(rf"^{re.escape(projection)}(\S*)$", map_scale).group(1)

    return ProjectionSpec(
        projection=projection,
        projection_pars=projection_pars,
        use_scale=use_scale,
        use_width=use_width,
        use_ratio=use_ratio,
        linear=linear,
        separator=separator,
        trial_value=trial_value,
        plot_scale=plot_scale,
        plot_width=plot_width,
    )


# ---------------------------------------------------------------------------
# GetBaseTick
# ---------------------------------------------------------------------------

@dataclass
class BaseTick:
    tick_x: str
    tick_y: str
    gridline_y: Optional[str] = None
    xlabel: Optional[str] = None
    ylabel: Optional[str] = None


_GEOGRAPHIC_TICK_TABLE = [
    (0.00006944444445, "0.25s"),
    (0.00013888888889, "0.5s"),
    (0.0002777777, "1s"),
    (0.0005555555, "2s"),
    (0.0013888889, "5s"),
    (0.0027777778, "10s"),
    (0.0041666667, "15s"),
    (0.0083333333, "30s"),
    (0.0166667, "1m"),
    (0.0333333, "2m"),
    (0.0833333, "5m"),
    (0.1666667, "10m"),
    (0.25, "15m"),
    (0.5, "30m"),
    (1.0, "1"),
    (2.0, "2"),
    (5.0, "5"),
    (10.0, "10"),
    (15.0, "15"),
    (30.0, "30"),
    (360.0, "60"),
]


def get_base_tick(
    gridprojected: int,
    xmin: float, xmax: float, ymin: float, ymax: float,
    xunits: str = "", yunits: str = "",
) -> BaseTick:
    """Port of sub GetBaseTick.

    gridprojected: 0 geographic, 1 UTM/projected-linear, 2 seismic
    (time vs. trace), 3 generic linear.
    """
    if gridprojected == 2:
        base_tick_x = 200.0 if (xmax - xmin) > 200 else (xmax - xmin)
        base_tick_y = 10.0 ** math.floor(math.log(abs(ymax - ymin) / 5) / math.log(10))
    else:
        base_tick_x = (xmax - xmin) / 3
        base_tick_y = (ymax - ymin) / 3

    if gridprojected == 2:
        base_gridline_y = base_tick_y / 2
        tick_y = f"a{base_tick_y}g{base_gridline_y}"
        return BaseTick(
            tick_x=str(base_tick_x), tick_y=tick_y,
            gridline_y=str(base_gridline_y),
            xlabel='"Trace Number"', ylabel='"Time (sec)"',
        )

    if gridprojected == 3:
        return BaseTick(
            tick_x=str(base_tick_x), tick_y=str(base_tick_y),
            xlabel=f'"{xunits}"', ylabel=f'"{yunits}"',
        )

    if gridprojected == 1:
        base_tick = min(base_tick_x, base_tick_y)
        if base_tick >= 10.0:
            base_tick = int(base_tick)
        elif base_tick >= 1.0:
            base_tick = 0.1 * int(10 * base_tick)
        elif base_tick >= 0.1:
            base_tick = 0.01 * int(100 * base_tick)
        elif base_tick >= 0.01:
            base_tick = 0.001 * int(1000 * base_tick)
        elif base_tick >= 0.001:
            base_tick = 0.0001 * int(10000 * base_tick)
        return BaseTick(tick_x=str(base_tick), tick_y=str(base_tick))

    # gridprojected == 0: geographic
    base_tick = min(base_tick_x, base_tick_y)
    tick_str = "60"
    for threshold, label in _GEOGRAPHIC_TICK_TABLE:
        if base_tick < threshold:
            tick_str = label
            break
    return BaseTick(tick_x=tick_str, tick_y=tick_str)


# ---------------------------------------------------------------------------
# Grid info (subset of mbm_grdplot's `gmt grdinfo` text-output parsing)
# ---------------------------------------------------------------------------

@dataclass
class GridInfo:
    xmin: float
    xmax: float
    ymin: float
    ymax: float
    zmin: float
    zmax: float
    xunits: str = ""
    yunits: str = ""
    gridprojected: int = 0
    utm_zone: Optional[str] = None


def run_grdinfo(grid_path: str) -> GridInfo:
    # errors="replace": a grid's embedded NetCDF metadata (e.g. a
    # "Remark" line carrying whatever locale/encoding wrote it) can
    # contain bytes that are not valid UTF-8 -- confirmed on a real grid
    # in the wild, where grdinfo's own "Remark:" line embeds a non-UTF-8
    # byte from its creation-time hostname/timestamp string. None of the
    # numeric fields this function actually parses live on that line, so
    # replacing undecodable bytes there (rather than raising) lets
    # parsing proceed unaffected.
    out = subprocess.run(
        ["gmt", "grdinfo", grid_path],
        capture_output=True, text=True, check=True,
        encoding="utf-8", errors="replace",
    ).stdout

    xmin = xmax = ymin = ymax = zmin = zmax = None
    xunits = yunits = ""
    gridprojected = 0
    utm_zone = None

    for line in out.splitlines():
        m = re.search(r"Projection: UTM Zone (\S+)", line)
        if m:
            utm_zone = m.group(1)
            gridprojected = 1
        if "Projection: SeismicProfile" in line:
            gridprojected = 2
        if "Projection: GenericLinear" in line:
            gridprojected = 3
        if "Projection: Geographic" in line:
            gridprojected = 0

        m = re.search(
            r"x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+\S+\s+(?:units|name):\s+(.+?)\s+n(?:x|_columns):\s+\S+",
            line,
        )
        if m:
            xmin, xmax, xunits = float(m.group(1)), float(m.group(2)), m.group(3)
        else:
            m = re.search(r"x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:", line)
            if m:
                xmin, xmax = float(m.group(1)), float(m.group(2))

        m = re.search(
            r"y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+\S+\s+(?:units|name):\s+(.+?)\s+n(?:y|_rows):\s+\S+",
            line,
        )
        if m:
            ymin, ymax, yunits = float(m.group(1)), float(m.group(2)), m.group(3)
        else:
            m = re.search(r"y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:", line)
            if m:
                ymin, ymax = float(m.group(1)), float(m.group(2))

        m = re.search(r"[zv]_?min:\s+(\S+)\s+[zv]_?max:\s+(\S+)\s+(?:units|name):", line)
        if m:
            zmin, zmax = float(m.group(1)), float(m.group(2))

    if xmin is None or ymin is None:
        raise RuntimeError(f"could not parse grdinfo output for {grid_path}")
    if zmin is None:
        zmin, zmax = 0.0, 1.0
    if zmin >= zmax:
        zmax = zmin + 1.0

    return GridInfo(xmin, xmax, ymin, ymax, zmin, zmax, xunits, yunits, gridprojected, utm_zone)


def _base_pars(projection: str, map_scale: Optional[str]) -> str:
    """Mirrors `($projection_pars) = $map_scale =~ /^$projection(\\S+)/;`
    as used when mbm_grdplot recomputes projection_pars just before the
    final sprintf. Returns "" when map_scale is falsy, exactly as the
    Perl regex match against an undefined/empty string does."""
    if not map_scale:
        return ""
    m = re.match(rf"^{re.escape(projection)}(\S+)$", map_scale)
    return m.group(1) if m else ""


def _perl_numify(s: str) -> float:
    """Mimic Perl's numeric coercion of a string: parse the longest
    leading valid-number prefix, defaulting to 0.0 if none."""
    m = re.match(r"^\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", s)
    return float(m.group(1)) if m else 0.0


def _sprintf_g5(base: str, separator: str, value) -> str:
    """Port of `sprintf("$base$separator%1.5g", $value)`."""
    v = _perl_numify(value) if isinstance(value, str) else value
    return f"{base}{separator}{v:.5g}"


def _mapproject_bbox(xmin, xmax, ymin, ymax, projection, projection_pars, region) -> tuple:
    """Run `gmt mapproject` on the four corner points of the region and
    return (dxx, dyy) -- the plotted width/height in inches at the trial
    scale/width used in projection_pars. Mirrors the corner-point logic
    in mbm_grdplot's main body (around the `elsif` opposite the linear
    fast path)."""
    points = [(xmin, ymin), (xmax, ymin), (xmax, ymax), (xmin, ymax)]
    with tempfile.NamedTemporaryFile("w", suffix=".dat", delete=False) as f:
        for x, y in points:
            f.write(f"{x} {y}\n")
        tmp_path = f.name
    try:
        proc = subprocess.run(
            ["gmt", "mapproject", tmp_path,
             f"-J{projection}{projection_pars}", f"-R{region}"],
            capture_output=True, text=True, check=True,
        )
    finally:
        Path(tmp_path).unlink(missing_ok=True)

    xs, ys = [], []
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            xs.append(float(parts[0]))
            ys.append(float(parts[1]))
    if not xs:
        raise RuntimeError(f"gmt mapproject produced no output (stderr: {proc.stderr})")
    return max(xs) - min(xs), max(ys) - min(ys)


# ---------------------------------------------------------------------------
# Full layout computation (port of the auto page/scale/orientation-fit
# algorithm in mbm_grdplot's main body, roughly lines 1252-1675).
# ---------------------------------------------------------------------------

@dataclass
class Layout:
    pagesize: str
    scale_loc: str
    projection: str
    projection_pars: str
    region: str
    landscape: bool
    width: float          # chosen page width, inches (already oriented)
    height: float          # chosen page height, inches
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
    base_tick: BaseTick
    gridprojected: int = 0   # resolved value (see compute_layout()'s docstring on
                             # the out-of-bounds fallback) -- callers building a
                             # GridInfo-consuming ScriptOptions from the same grid
                             # should use this instead of the raw GridInfo.gridprojected


def compute_layout(
    grid: GridInfo,
    pagesize: str = "a",
    scale_loc: str = "b",
    map_scale: Optional[str] = None,     # user-supplied -J value, if any
    orientation: int = 0,                # 0 auto, 1 portrait, 2 landscape
    region: Optional[str] = None,        # user -R override; else grid bounds
) -> Layout:
    xmin, xmax, ymin, ymax = grid.xmin, grid.xmax, grid.ymin, grid.ymax
    dzz = grid.zmax - grid.zmin

    # Port of mbm_grdplot lines ~1167-1191: a user-supplied -R overrides
    # not just the final plot region string but xmin/xmax/ymin/ymax
    # themselves, which drive every subsequent computation below (the
    # apparent plot size via mapproject, the projected-vs-geographic
    # fallback check, contour interval, tick spacing, ...). Confirmed as
    # a real gap in this port: without this, an explicit --region was
    # honored for the final -R string but silently ignored for the
    # actual page-fit sizing, which kept using the grid's own full
    # extent instead -- producing a wildly wrong plot scale/aspect
    # ratio whenever --region requested a sub-area of the grid.
    if region:
        xmin, xmax, ymin, ymax = (float(v) for v in region.split("/")[:4])

    margins = get_page_size(pagesize, scale_loc)

    use_scale = use_width = use_ratio = linear = False
    # NOTE: mbm_grdplot's $separator is only ever assigned inside
    # GetProjection; when map_scale is not supplied that sub never runs,
    # so $separator stays Perl-undef, which stringifies to "" everywhere
    # it is later interpolated. Mirror that here rather than defaulting
    # to "/".
    separator = ""
    trial_value = "1.0"
    plot_scale = plot_width = None
    projection = projection_pars = None

    if map_scale:
        spec = get_projection(map_scale)
        projection = spec.projection
        projection_pars = spec.projection_pars
        use_scale, use_width = spec.use_scale, spec.use_width
        use_ratio, linear = spec.use_ratio, spec.linear
        separator, trial_value = spec.separator, spec.trial_value
        plot_scale, plot_width = spec.plot_scale, spec.plot_width

    # Port of mbm_grdplot lines ~1200-1206: even when the grid's own
    # metadata doesn't tag it as a recognized UTM projection (see
    # run_grdinfo() above -- it only checks for the literal string "UTM
    # Zone"), treat it as projected (not geographic) if its plot bounds
    # (xmin/xmax/ymin/ymax, already reflecting a -R override above if
    # given) fall outside plausible lon/lat ranges. This catches grids
    # in other projected coordinate systems MB-System supports -- e.g.
    # its own "LTM" (local tangent Mercator) grids -- which would
    # otherwise default to a Mercator projection and fail outright (gmt
    # mapproject errors out, or worse silently mis-projects) when handed
    # plot bounds in meters instead of degrees.
    gridprojected = grid.gridprojected
    if gridprojected == 0 and (xmin < -360.0 or xmax > 360.0 or ymin < -90.0 or ymax > 90.0):
        gridprojected = 1

    if (use_scale and plot_scale) or (use_width and plot_width):
        pass  # projection/projection_pars as parsed above
    elif use_scale or use_width:
        projection_pars = f"{projection_pars}{separator}{trial_value}"
    elif gridprojected > 0:
        projection, projection_pars = "x", "1.0"
        use_scale, linear = True, True
    else:
        projection, projection_pars = "m", "1.0"
        use_scale = True

    bounds_plot = region or f"{xmin:.11g}/{xmax:.11g}/{ymin:.11g}/{ymax:.11g}"

    if projection == "x" and map_scale and re.match(r"^\S+/\S+", map_scale[1:]):
        xscale, yscale = (float(v) for v in re.match(r"^x(\S+)/(\S+)", map_scale).groups())
        dxx = abs(xmax * xscale - xmin * xscale)
        dyy = abs(ymax * yscale - ymin * yscale)
    else:
        dxx, dyy = _mapproject_bbox(xmin, xmax, ymin, ymax, projection, projection_pars, bounds_plot)
        dxx, dyy = abs(dxx), abs(dyy)

    landscape = portrait = False
    width = height = 0.0
    width_max = height_max = 0.0

    if (use_scale and plot_scale) or (use_width and plot_width):
        plot_width_, plot_height_ = dxx, dyy
        if orientation == 1:
            portrait = True
            width, height = PAGE_WIDTH_IN[pagesize], PAGE_HEIGHT_IN[pagesize]
            width_max, height_max = margins.width_max_portrait, margins.height_max_portrait
        elif orientation == 2:
            landscape = True
            width, height = PAGE_HEIGHT_IN[pagesize], PAGE_WIDTH_IN[pagesize]
            width_max, height_max = margins.width_max_landscape, margins.height_max_landscape
        elif dxx > dyy:
            landscape = True
            width, height = PAGE_HEIGHT_IN[pagesize], PAGE_WIDTH_IN[pagesize]
            width_max, height_max = margins.width_max_landscape, margins.height_max_landscape
        else:
            portrait = True
            width, height = PAGE_WIDTH_IN[pagesize], PAGE_HEIGHT_IN[pagesize]
            width_max, height_max = margins.width_max_portrait, margins.height_max_portrait

        if plot_width_ > width_max or plot_height_ > height_max:
            good_page = None
            for elem in PAGE_SIZE_NAMES:
                m2 = get_page_size(elem, scale_loc)
                wm = m2.width_max_portrait if portrait else m2.width_max_landscape
                hm = m2.height_max_portrait if portrait else m2.height_max_landscape
                if plot_width_ <= wm and plot_height_ <= hm:
                    good_page = elem
                    break
            if good_page is None:
                good_page = pagesize  # mbm_grdplot warns and keeps trying anyway
            pagesize = good_page
            margins = get_page_size(pagesize, scale_loc)
            width = PAGE_WIDTH_IN[pagesize] if portrait else PAGE_HEIGHT_IN[pagesize]
            height = PAGE_HEIGHT_IN[pagesize] if portrait else PAGE_WIDTH_IN[pagesize]

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
            width, height = PAGE_WIDTH_IN[pagesize], PAGE_HEIGHT_IN[pagesize]
        elif orientation == 2:
            landscape = True
            plot_scale = plot_scale_landscape
            width, height = PAGE_HEIGHT_IN[pagesize], PAGE_WIDTH_IN[pagesize]
        elif plot_scale_landscape > plot_scale_portrait:
            landscape = True
            plot_scale = plot_scale_landscape
            width, height = PAGE_HEIGHT_IN[pagesize], PAGE_WIDTH_IN[pagesize]
        else:
            portrait = True
            plot_scale = plot_scale_portrait
            width, height = PAGE_WIDTH_IN[pagesize], PAGE_HEIGHT_IN[pagesize]

        plot_width = dxx * plot_scale
        plot_height = dyy * plot_scale

        # mbm_grdplot re-derives projection_pars from the *original*
        # map_scale string here (not from the trial-value-appended form
        # used for the mapproject probe above); with no user map_scale
        # at all, this is "".
        base = _base_pars(projection, map_scale)
        if use_ratio:
            # Ratio-based azimuthal projections (a/e/g/s) format their
            # scale as "1:NNNN". mbm_grdplot used to run this string
            # through sprintf("%1.5g", ...) here, which numifies "1:NNNN"
            # down to just "1" (Perl's numeric coercion stops at the
            # first non-numeric character) -- fixed upstream by
            # appending the ratio string directly instead of formatting
            # it as %g.
            top = int(1 / plot_scale)
            projection_pars = f"{base}{separator}1:{top}"
        else:
            projection_pars = _sprintf_g5(base, separator, plot_scale)
        if linear and gridprojected == 0:
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
            width, height = PAGE_HEIGHT_IN[pagesize], PAGE_WIDTH_IN[pagesize]
        else:
            width, height = PAGE_WIDTH_IN[pagesize], PAGE_HEIGHT_IN[pagesize]

        base = _base_pars(projection, map_scale)
        projection_pars = _sprintf_g5(base, separator, plot_width)
        if linear and gridprojected == 0:
            projection_pars = f"{projection_pars}d"

    xoffset = (width - abs(plot_width) - margins.space_left - margins.space_right) / 2 + margins.space_left
    yoffset = (height - abs(plot_height) - margins.space_bottom - margins.space_top) / 2 + margins.space_bottom

    # degree annotation format
    degree_format = "ddd:mm"
    if gridprojected == 0:
        xsize = (xmax - xmin) / 3
        ysize = (ymax - ymin) / 3
        size = min(xsize, ysize)
        if size > 4.0:
            degree_format = "ddd"
        elif size > (1.0 / 60.0):
            degree_format = "ddd:mm"
        else:
            degree_format = "ddd:mm:ss"

    # color scale placement
    sl = scale_loc.lower()
    page_h = PAGE_HEIGHT_IN[pagesize]
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

    # contour interval heuristic
    contour_int = 0.0
    if dzz > 0:
        base = int((math.log(dzz) / math.log(10.0)) + 0.5)
        contour_int = (10 ** base) / 10.0
        if dzz / contour_int < 10:
            contour_int = contour_int / 4
        elif dzz / contour_int < 20:
            contour_int = contour_int / 2

    base_tick = get_base_tick(gridprojected, xmin, xmax, ymin, ymax, grid.xunits, grid.yunits)

    return Layout(
        pagesize=pagesize,
        scale_loc=scale_loc,
        projection=projection,
        projection_pars=projection_pars,
        region=bounds_plot,
        landscape=landscape,
        width=width,
        height=height,
        plot_width=plot_width,
        plot_height=plot_height,
        xoffset=xoffset,
        yoffset=yoffset,
        degree_format=degree_format,
        colorscale_length=colorscale_length,
        colorscale_thick=colorscale_thick,
        colorscale_offx=colorscale_offx,
        colorscale_offy=colorscale_offy,
        colorscale_vh=colorscale_vh,
        contour_int=contour_int,
        gridprojected=gridprojected,
        base_tick=base_tick,
    )
