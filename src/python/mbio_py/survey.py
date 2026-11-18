#--------------------------------------------------------------------
#    The MB-system:  survey.py  1/11/2026
#
#    Copyright (c) 2025-2026 by
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------
"""Pairing of a Kongsberg survey line: *.kmall or *.all (nav, attitude, bathymetry) + *.kmwcd or *.wcd
(water column).

SIS names each file nnnn_YYYYMMDD_HHMMSS after the time of its first datagram, so the
two files of one line can differ by seconds: they are paired by line number nnnn and
nearest file name time, then ping by ping by ping number and time.
"""
import glob
import os
import re
from dataclasses import dataclass, field
from datetime import datetime, timezone

import numpy as np

from . import constants as K
from .reader import Reader
from .watercolumn import WaterColumnFile

_NAME = re.compile(r"^(\d{4})_(\d{8})_(\d{6})")
_COMPANION = {".kmall": ".kmwcd", ".kmwcd": ".kmall", ".all": ".wcd", ".wcd": ".all"}


def _name_time(path):
    m = _NAME.match(os.path.basename(path))
    if not m:
        return None, None
    t = datetime.strptime(m.group(2) + m.group(3), "%Y%m%d%H%M%S").replace(tzinfo=timezone.utc)
    return m.group(1), t.timestamp()


def find_companion(path, max_offset_s=600.0):
    """Return the *.kmwcd of a *.kmall or the *.wcd of a *.all (or the reverse) in the same directory, or None."""
    line, t = _name_time(path)
    if line is None:
        return None
    other = _COMPANION.get(os.path.splitext(path)[1].lower())
    if other is None:
        return None
    best, best_dt = None, max_offset_s
    for cand in glob.glob(os.path.join(os.path.dirname(os.path.abspath(path)), line + "_*")):
        if not cand.lower().endswith(other):
            continue
        cline, ct = _name_time(cand)
        if cline == line and ct is not None and abs(ct - t) <= best_dt:
            best, best_dt = cand, abs(ct - t)
    return best


@dataclass
class SurveyPings:
    """The survey pings of a *.kmall: per ping arrays plus the soundings of each ping."""
    time_d: np.ndarray
    ping_number: np.ndarray
    navlon: np.ndarray
    navlat: np.ndarray
    speed: np.ndarray          # km/h
    heading: np.ndarray        # deg
    sensordepth: np.ndarray    # m, transducer depth below the waterline
    beamflag: list = field(repr=False)
    bath: list = field(repr=False)             # m below the waterline
    bathacrosstrack: list = field(repr=False)  # m, positive starboard, from the sonar
    bathalongtrack: list = field(repr=False)
    detects: list = field(repr=False)          # MB_DETECT_* per beam
    nav_time_d: np.ndarray = field(repr=False, default=None)  # asynchronous position records
    nav_lon: np.ndarray = field(repr=False, default=None)
    nav_lat: np.ndarray = field(repr=False, default=None)
    svps: list = field(repr=False, default_factory=list)   # (time_d, depth, velocity)

    def __len__(self):
        return len(self.time_d)


def load_survey(path, format=0):
    """Read all survey pings and position records of a *.kmall."""
    cols = {k: [] for k in ("time_d", "ping_number", "navlon", "navlat", "speed", "heading", "sensordepth",
                            "beamflag", "bath", "bathacrosstrack", "bathalongtrack", "detects")}
    nav = {"t": [], "lon": [], "lat": []}
    svps = []
    with Reader(path, format) as r:
        for rec in r:
            if rec.svp_depth is not None:
                svps.append((rec.time_d, rec.svp_depth, rec.svp_velocity))
            elif rec.kind == K.MB_DATA_DATA and rec.ok:
                for k in cols:
                    v = getattr(rec, k)
                    cols[k].append(v if v is not None else np.zeros(len(rec.bath), np.int32))
            elif rec.kind == K.MB_DATA_NAV and rec.ok and (rec.navlon != 0.0 or rec.navlat != 0.0):
                nav["t"].append(rec.time_d)
                nav["lon"].append(rec.navlon)
                nav["lat"].append(rec.navlat)
    scalars = {k: np.asarray(cols[k], dtype=float if k != "ping_number" else np.int64)
               for k in ("time_d", "ping_number", "navlon", "navlat", "speed", "heading", "sensordepth")}
    return SurveyPings(**scalars, beamflag=cols["beamflag"], bath=cols["bath"],
                       bathacrosstrack=cols["bathacrosstrack"], bathalongtrack=cols["bathalongtrack"],
                       detects=cols["detects"], nav_time_d=np.asarray(nav["t"]), nav_lon=np.asarray(nav["lon"]),
                       nav_lat=np.asarray(nav["lat"]), svps=svps)


class KongsbergLine:
    """A *.kmall and its *.kmwcd, or a *.all and its *.wcd, either of which may be missing.

    The attributes kmall and kmwcd name the survey file and the water column file of either
    kind. A *.all without a *.wcd is both, as it may hold the water column itself.

    line = KongsbergLine("0080_20250618_223434.kmwcd")
    wc = line.wc.ping(i)                 # water column (WaterColumnFile)
    j = line.survey_index(i)             # matching *.kmall ping or None
    lon, lat, sog_kn, cog = line.nav_at(wc.time_d)
    """

    def __init__(self, path, progress=None, max_dt_s=1.0):
        suffix = os.path.splitext(path)[1].lower()
        self.kmwcd = path if suffix in (".kmwcd", ".wcd") else find_companion(path)
        self.kmall = path if suffix in (".kmall", ".all") else find_companion(path)
        if suffix == ".all" and self.kmwcd is None:
            self.kmwcd = path
        self.survey = load_survey(self.kmall) if self.kmall else None
        self.wc = WaterColumnFile(self.kmwcd, progress=progress) if self.kmwcd else None
        if self.wc is not None and len(self.wc) == 0:
            self.wc = None
        self._match = {}
        if self.survey is not None and self.wc is not None:
            by_ping = {}
            for j, p in enumerate(self.survey.ping_number):
                by_ping.setdefault(int(p), []).append(j)
            for i, (p, t) in enumerate(zip(self.wc.ping_numbers, self.wc.times)):
                for j in by_ping.get(int(p), []):
                    if abs(self.survey.time_d[j] - t) <= max_dt_s:
                        self._match[i] = j
                        break

    def survey_index(self, i):
        """Index of the *.kmall ping matching water column ping i, or None."""
        return self._match.get(i)

    @property
    def matched(self):
        return len(self._match)

    def nav_at(self, time_d):
        """(lon, lat, sog_knots, cog_deg) interpolated at time_d from the *.kmall; NaNs if unavailable.

        Uses the *.kmall position records (#SPO) when time_d is inside their time range, else
        the *.kmall ping positions, else the position records of the *.kmwcd; COG and SOG from
        the positions around time_d."""
        nan = (np.nan,) * 4
        s = self.survey
        sources = []
        if s is not None and s.nav_time_d is not None and len(s.nav_time_d) > 1:
            sources.append((s.nav_time_d, s.nav_lon, s.nav_lat))
        if s is not None and len(s) > 1:
            known = (s.navlon != 0.0) | (s.navlat != 0.0)  # no position before the first position record
            if np.count_nonzero(known) > 1:
                sources.append((s.time_d[known], s.navlon[known], s.navlat[known]))
        if self.wc is not None and len(self.wc.nav_time_d) > 1:
            sources.append((self.wc.nav_time_d, self.wc.nav_lon, self.wc.nav_lat))
        for t, lon, lat in sources:
            if t[0] <= time_d <= t[-1]:
                break
        else:
            return nan
        x = float(np.interp(time_d, t, lon))
        y = float(np.interp(time_d, t, lat))
        k = int(np.clip(np.searchsorted(t, time_d), 1, len(t) - 1))
        k0, k1 = max(k - 5, 0), min(k + 5, len(t) - 1)
        dt = t[k1] - t[k0]
        if dt <= 0:
            return x, y, np.nan, np.nan
        dn = (lat[k1] - lat[k0]) * 111320.0
        de = (lon[k1] - lon[k0]) * 111320.0 * np.cos(np.deg2rad(y))
        sog = np.hypot(dn, de) / dt * 1.9438445
        cog = float(np.rad2deg(np.arctan2(de, dn)) % 360.0)
        return x, y, float(sog), cog

    def svp(self, time_d):
        """(depth, velocity) of the sound velocity profile in use at time_d, or None.

        Profiles from the *.kmall, else from the *.kmwcd; the last one logged before time_d
        (profiles without a time count as logged at the start), else the first."""
        for source in (self.survey.svps if self.survey is not None else [], self.wc.svps if self.wc else []):
            if source:
                before = [p for p in source if p[0] <= time_d or p[0] <= 0.0]
                t, d, c = (before[-1] if before else source[0])
                return d, c
        return None

    def draft(self, i):
        """Transducer depth (m) for water column ping i from the matched *.kmall ping, or None."""
        j = self.survey_index(i)
        return float(self.survey.sensordepth[j]) if j is not None else None

    def heading_at(self, time_d):
        """Vessel heading (deg) interpolated at time_d from the *.kmall pings; NaN if unavailable."""
        if self.survey is None or len(self.survey) == 0:
            return np.nan
        s = self.survey
        if time_d < s.time_d[0] - 10 or time_d > s.time_d[-1] + 10:
            return np.nan
        h = np.unwrap(np.deg2rad(s.heading))
        return float(np.rad2deg(np.interp(time_d, s.time_d, h)) % 360.0)
