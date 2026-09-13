#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_misc.py
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
#   Python port of mbm_grdplot's -M<misc> sub-option grammar (mbm_grdplot
#   lines ~384-759) -- coastline (pscoast), xy overlay (psxy), contour
#   (grdcontour) tuning, image rendering, and swath-navigation/ping-tick
#   annotation controls, plus a handful of general options (GMT default
#   overrides, color-scale placement, a length/map scale bar, a custom
#   map origin, data rescaling, text labels, and a Unix time stamp).
#
#   -M itself packs ~28 unrelated sub-flags behind a single letter and
#   a ":"-separated mini-language (e.g. "-MGFl:MTDf:MXIfile.xy"); this
#   module models the *information* each sub-flag carries as plain
#   dataclasses (MiscOptions and its per-category members below),
#   independent of how it is spelled on a command line. parse_misc()
#   reads the legacy "-M" string form (for validating against real
#   mbm_grdplot output, and as a migration aid for old scripts); the
#   long-option Python CLI this is meant to serve should populate the
#   same dataclasses directly from descriptive flags (--coast-resolution,
#   --xy-file, --ping-number-ticks, ...) rather than re-exposing "-M".
#
# Status:
#   All sub-options read and modeled below. Cross-checked against real
#   mbm_grdplot .cmd output for a representative sample covering every
#   category (general, contour, image, navigation, coast, xy) -- see
#   test_mbpy_misc.py -- rather than exhaustively for all ~28
#   sub-flags; the remaining ones are straightforward analogues of the
#   validated ones (same regex-extract-and-store shape) but have not
#   individually been run against Perl.
#
import re
import subprocess
from dataclasses import dataclass, field, replace
from typing import List, Optional


@dataclass
class GeneralOptions:
    gmt_defs: List[str] = field(default_factory=list)          # -MGD, repeatable
    scale_loc: Optional[str] = None                             # -MGF
    length_scale: Optional[str] = None                          # -MGL
    length_scale_frame: bool = False                            # -MGLF
    xorigin: Optional[float] = None                             # -MGO (first field)
    yorigin: Optional[float] = None                             # -MGO (second field)
    originset: bool = False
    dpi: Optional[str] = None                                   # -MGQ
    data_scale: Optional[str] = None                            # -MGS
    text_labels: List[str] = field(default_factory=list)        # -MGT, repeatable
    unix_stamp: Optional[str] = None                            # -MGU<label>
    unix_stamp_on: bool = False                                 # -MGU


@dataclass
class ContourOptions:
    contour_anot_int: Optional[str] = None    # -MCA
    contour_gap: Optional[str] = None         # -MCG
    contour_cut: Optional[str] = None         # -MCQ
    contour_tick_on: bool = False             # -MCT
    contour_tick: Optional[str] = None        # -MCT<val>
    contour_pen: Optional[str] = None         # -MCW


@dataclass
class ImageOptions:
    image_render: bool = False
    image_resolution: Optional[str] = None    # -MIE
    image_type: Optional[str] = None          # -MIT


@dataclass
class NavigationOptions:
    name_mode: bool = False               # -MNA...
    name_perp: bool = False
    nav_name_hgt: Optional[float] = None
    swathformat: Optional[str] = None                 # -MNF
    swathnavdatalist: Optional[str] = None             # -MNI
    navigation_mode: bool = False                      # -MNN
    navigation_control: Optional[str] = None           # -MNN<val>
    pingnumber_mode: bool = False                      # -MNP
    pingnumber_control: Optional[str] = None
    pingnumber_tick: Optional[float] = None
    pingnumber_annot: Optional[float] = None
    pingnumber_tick_len: Optional[float] = None
    nav_pen: Optional[str] = None                      # -MNW


def resolve_navigation_control(nav: NavigationOptions, has_swathnavdatalist: bool) -> NavigationOptions:
    """Port of the navigation_control/pingnumber_control normalization at
    mbm_grdplot lines ~1953-2037: rebuilds `navigation_control` into the
    final "tick/annot/dateannot/ticksize" form the `-D` flag of `gmt
    mbcontour` expects (see mbpy_grdplot_script.py), and `pingnumber_control`
    into "tick/annot/ticklen" for `-M`. Returns an updated copy of `nav`.

    NOTE: mbm_grdplot's own first check here (already->=4-field input)
    uses a regex that is also satisfied by 5- and 6-field input (any
    string with 3 or more "/" characters matches it), which makes the
    "elsif" branches further down that specifically look for 5- or
    6-field forms (to pull a trailing name-height/perpendicular-flag
    pair out of `-MNN`) unreachable dead code upstream -- any input
    with that many fields is caught by the first check and passed
    through unparsed instead. Reproduced by only implementing the
    reachable (1-3 field) forms below rather than porting dead code.
    """
    navigation_mode = nav.navigation_mode
    navigation_control = nav.navigation_control
    name_mode = nav.name_mode
    name_perp = nav.name_perp
    nav_name_hgt = nav.nav_name_hgt

    if has_swathnavdatalist and not navigation_control and not navigation_mode:
        navigation_mode = True
        navigation_control = "100000/100000/100000/0.15"

    if navigation_control and navigation_control.count("/") >= 3:
        navigation_mode = True
    elif navigation_control and ("FP" in navigation_control or "fp" in navigation_control):
        navigation_mode = True
        name_mode = True
        name_perp = True
        navigation_control = "0.25/1/4/0.15"
        nav_name_hgt = 0.15
    elif navigation_control and ("F" in navigation_control or "f" in navigation_control):
        navigation_mode = True
        name_mode = True
        name_perp = False
        navigation_control = "0.25/1/4/0.15"
        nav_name_hgt = 0.15
    elif navigation_control:
        parts = navigation_control.split("/")
        if len(parts) == 3:
            nav_time_tick, nav_time_annot, nav_date_annot = parts
            nav_tick_size = "0.15"
        elif len(parts) == 2:
            nav_time_tick, nav_time_annot = parts
            nav_date_annot = "100000"
            nav_tick_size = "0.15"
        else:
            nav_time_tick = navigation_control
            nav_time_annot = "100000"
            nav_date_annot = "100000"
            nav_tick_size = "0.15"
        navigation_mode = True
        navigation_control = f"{nav_time_tick}/{nav_time_annot}/{nav_date_annot}/{nav_tick_size}"
    elif navigation_mode:
        navigation_control = "0.25/1/4/0.15"

    pingnumber_control = nav.pingnumber_control
    if nav.pingnumber_mode:
        pingnumber_control = f"{nav.pingnumber_tick}/{nav.pingnumber_annot}/{nav.pingnumber_tick_len}"

    return replace(
        nav,
        navigation_mode=navigation_mode,
        navigation_control=navigation_control,
        name_mode=name_mode,
        name_perp=name_perp,
        nav_name_hgt=nav_name_hgt,
        pingnumber_control=pingnumber_control,
    )


def resolve_swath_format(swathnavdatalist: str, swathformat: Optional[str] = None) -> str:
    """Port of the swathformat auto-detection at mbm_grdplot lines
    ~2903-2911: if not explicitly given, ask `mbformat -I <datalist> -L`;
    if it reports 0 (unrecognized), fall back to -1 (MB-System's own
    datalist format code).
    """
    if swathformat:
        return swathformat
    proc = subprocess.run(
        ["mbformat", "-I", swathnavdatalist, "-L"],
        capture_output=True, text=True, check=True,
    )
    detected = proc.stdout.split()[0]
    if detected == "0":
        detected = "-1"
    return detected


@dataclass
class CoastOptions:
    coast_control: bool = False
    coast_lakefill: Optional[str] = None      # -MTC
    coast_resolution: Optional[str] = None    # -MTD
    coast_dryfill: Optional[str] = None       # -MTG
    coast_river: Optional[str] = None         # -MTI
    coast_boundaries: List[str] = field(default_factory=list)  # -MTN, repeatable
    coast_wetfill: Optional[str] = None       # -MTS
    coast_pen: Optional[str] = None           # -MTW


@dataclass
class XYOverlayEntry:
    file: str
    symbol: str
    fill: str
    segment: str
    segchar: str
    pen: str


@dataclass
class XYOptions:
    entries: List[XYOverlayEntry] = field(default_factory=list)
    # "pending" values as last set by -MXG/-MXS/-MXW/-MXM; once
    # defaulted at a -MXI (no explicit value yet given), the default
    # itself becomes the new pending value, exactly as mbm_grdplot's
    # persistent globals behave -- see apply_misc_command()'s -MXI
    # handling below.
    _pending_fill: Optional[str] = None
    _pending_symbol: Optional[str] = None
    _pending_segment: Optional[str] = None
    _pending_segchar: Optional[str] = None
    _pending_pen: Optional[str] = None


@dataclass
class MiscOptions:
    general: GeneralOptions = field(default_factory=GeneralOptions)
    contour: ContourOptions = field(default_factory=ContourOptions)
    image: ImageOptions = field(default_factory=ImageOptions)
    navigation: NavigationOptions = field(default_factory=NavigationOptions)
    coast: CoastOptions = field(default_factory=CoastOptions)
    xy: XYOptions = field(default_factory=XYOptions)


def apply_misc_command(cmd: str, opts: MiscOptions) -> None:
    """Apply one ":"-separated -M sub-command to `opts` in place.
    Port of the per-$cmd dispatch inside mbm_grdplot's main -M loop
    (lines ~389-758); branch order and regexes mirror the Perl source.
    """
    g, c, im, nav, coast, xy = (
        opts.general, opts.contour, opts.image, opts.navigation, opts.coast, opts.xy
    )

    # --- general options ---
    m = re.match(r"^[Gg][Dd](\S+)", cmd)
    if m:
        g.gmt_defs.append(m.group(1))

    m = re.match(r"^[Gg][Ff](\S+)", cmd)
    if m:
        g.scale_loc = m.group(1)

    if re.match(r"^[Gg][Ll][Ff]", cmd):
        g.length_scale_frame = True
    else:
        m = re.match(r"^[Gg][Ll](\S+)", cmd)
        if m:
            g.length_scale = m.group(1)

    m = re.match(r"^[Gg][Oo](\S+)/(\S+)", cmd)
    if m:
        g.xorigin, g.yorigin = float(m.group(1)), float(m.group(2))
        g.originset = True

    m = re.match(r"^[GG][Qq](.+)", cmd)
    if m:
        g.dpi = m.group(1)

    m = re.match(r"^[Gg][Ss](\S+)", cmd)
    if m:
        g.data_scale = m.group(1)

    m = re.match(r"^[Gg][Tt](\S+)/(\S+)/(\S+)/(\S+)/(\S+)/(\S+)/(.+)", cmd)
    if m and m.group(7):
        raw = re.match(r"^[Gg][Tt](.*)", cmd).group(1)
        g.text_labels.append(raw)
    elif re.match(r"^[Gg][Tt].", cmd):
        print(f"\nInvalid text label ignored: {cmd}")

    m = re.match(r"^[Gg][Uu](\S+)", cmd)
    if m:
        g.unix_stamp = m.group(1)
        g.unix_stamp_on = True
    elif re.match(r"^[Gg][Uu]", cmd):
        g.unix_stamp_on = True

    # --- grdcontour options ---
    m = re.match(r"^[Cc][Aa](\S+)", cmd)
    if m:
        c.contour_anot_int = m.group(1)

    m = re.match(r"^[Cc][Gg](\S+)", cmd)
    if m:
        c.contour_gap = m.group(1)

    m = re.match(r"^[Cc][Qq](\S+)", cmd)
    if m:
        c.contour_cut = m.group(1)

    if re.match(r"^[Cc][Tt](\S)", cmd):
        c.contour_tick = re.match(r"^[Cc][Tt](\S+)", cmd).group(1)
        c.contour_tick_on = True
    elif re.match(r"^[Cc][Tt]", cmd):
        c.contour_tick_on = True

    m = re.match(r"^[Cc][Ww](\S+)", cmd)
    if m:
        c.contour_pen = m.group(1)

    # --- image rendering options ---
    m = re.match(r"^[Ii][Ee](\S+)", cmd)
    if m:
        im.image_resolution = m.group(1)
        im.image_render = True
        if not im.image_type:
            im.image_type = "j"

    m = re.match(r"^[Ii][Tt](\S+)", cmd)
    if m:
        im.image_type = m.group(1)
        im.image_render = True
        if not im.image_resolution:
            im.image_resolution = "300"

    # --- swath navigation options ---
    m = re.match(r"^[Nn][Aa](\S+)/P", cmd)
    if m:
        nav.nav_name_hgt = float(m.group(1))
        nav.name_mode = True
        nav.name_perp = True
    elif re.match(r"^[Nn][Aa]P", cmd):
        nav.name_mode = True
        nav.name_perp = True
        nav.nav_name_hgt = 0.15
    else:
        m = re.match(r"^[Nn][Aa](\S+)", cmd)
        if m:
            nav.nav_name_hgt = float(m.group(1))
            nav.name_mode = True
            nav.name_perp = False
        elif re.match(r"^[Nn][Aa]", cmd):
            nav.name_mode = True
            nav.name_perp = False
            nav.nav_name_hgt = 0.15

    m = re.match(r"^[Nn][Ff](\S+)", cmd)
    if m:
        nav.swathformat = m.group(1)

    m = re.match(r"^[Nn][Ii](\S+)", cmd)
    if m:
        nav.swathnavdatalist = m.group(1)

    m = re.match(r"^[Nn][Nn](\S+)", cmd)
    if m:
        nav.navigation_control = m.group(1)
        nav.navigation_mode = True
    elif re.match(r"^[Nn][Nn]", cmd):
        nav.navigation_mode = True

    m = re.match(r"^[Nn][Pp](.+)", cmd)
    if m:
        nav.pingnumber_control = m.group(1)
        nav.pingnumber_mode = True
        pc = nav.pingnumber_control
        m3 = re.match(r"(\S+)/(\S+)/(\S+)", pc)
        m2 = re.match(r"(\S+)/(\S+)", pc)
        m1 = re.match(r"(\S+)", pc)
        if m3:
            nav.pingnumber_tick, nav.pingnumber_annot, nav.pingnumber_tick_len = (
                float(m3.group(1)), float(m3.group(2)), float(m3.group(3))
            )
        elif m2:
            nav.pingnumber_tick, nav.pingnumber_annot = float(m2.group(1)), float(m2.group(2))
            nav.pingnumber_tick_len = 0.10
        elif m1:
            nav.pingnumber_tick = float(m1.group(1))
            nav.pingnumber_tick_len = 0.10
            nav.pingnumber_annot = 100
    elif re.match(r"^[Nn][Pp]", cmd):
        nav.pingnumber_mode = True
        nav.pingnumber_tick_len = 0.10
        nav.pingnumber_annot = 100
        nav.pingnumber_tick = 50

    m = re.match(r"^[Nn][Ww](\S+)", cmd)
    if m:
        nav.nav_pen = m.group(1)

    # --- pscoast options ---
    m = re.match(r"^[Tt][Cc](.+)", cmd)
    if m:
        coast.coast_lakefill = m.group(1)
        coast.coast_control = True

    m = re.match(r"^[Tt][Dd](.+)", cmd)
    if m:
        coast.coast_resolution = m.group(1)
        coast.coast_control = True

    m = re.match(r"^[Tt][Gg](.+)", cmd)
    if m:
        coast.coast_dryfill = m.group(1)
        coast.coast_control = True

    m = re.match(r"^[Tt][Ii](.+)", cmd)
    if m:
        coast.coast_river = m.group(1)
        coast.coast_control = True

    m = re.match(r"^[Tt][Nn](.+)", cmd)
    if m:
        coast.coast_boundaries.append(m.group(1))
        coast.coast_control = True

    m = re.match(r"^[Tt][Ss](.+)", cmd)
    if m:
        coast.coast_wetfill = m.group(1)
        coast.coast_control = True

    m = re.match(r"^[Tt][Ww](.+)", cmd)
    if m:
        coast.coast_pen = m.group(1)
        coast.coast_control = True

    # --- psxy options ---
    m = re.match(r"^[Xx][Gg](.+)", cmd)
    if m:
        xy._pending_fill = m.group(1)

    m = re.match(r"^[Xx][Ii](.+)", cmd)
    if m:
        xyfile = m.group(1)
        if xy._pending_symbol is None:
            xy._pending_symbol = "N"
        if xy._pending_fill is None:
            xy._pending_fill = "N"
        if xy._pending_segment is None:
            xy._pending_segment = "N"
        if xy._pending_segchar is None:
            xy._pending_segchar = ">"
        if xy._pending_pen is None:
            xy._pending_pen = "N"
        xy.entries.append(XYOverlayEntry(
            file=xyfile,
            symbol=xy._pending_symbol,
            fill=xy._pending_fill,
            segment=xy._pending_segment,
            segchar=xy._pending_segchar,
            pen=xy._pending_pen,
        ))

    if re.match(r"^[Xx][Mm]", cmd):
        m = re.match(r"^[Xx][Mm](\S)", cmd)
        if m:
            xy._pending_segchar = m.group(1)
        else:
            xy._pending_segchar = ">"
        if xy._pending_segment is None:
            xy._pending_segment = "Y"
        elif xy._pending_segment != "N":
            xy._pending_segment = "N"
        else:
            xy._pending_segment = "Y"

    m = re.match(r"^[Xx][Ss](.+)", cmd)
    if m:
        xy._pending_symbol = m.group(1)

    m = re.match(r"^[Xx][Ww](.+)", cmd)
    if m:
        xy._pending_pen = m.group(1)


def parse_misc(misc: Optional[str]) -> MiscOptions:
    """Port of the legacy "-M<misc>" string form: split on ":" and apply
    each piece via apply_misc_command(). Provided for validating the
    dataclasses above against real mbm_grdplot output, and as a
    migration aid for translating old command lines -- the long-option
    Python CLI itself should populate MiscOptions directly rather than
    building and re-parsing this string.
    """
    opts = MiscOptions()
    if not misc:
        return opts
    for cmd in misc.split(":"):
        apply_misc_command(cmd, opts)
    return opts
