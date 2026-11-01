#--------------------------------------------------------------------
#    The MB-system:  watercolumn.py  1/11/2026
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
"""Water column data through MBIO (mb_extract_wcdimensions, mb_extract_wc)."""
import ctypes as C
from dataclasses import dataclass, field
from typing import Optional

import numpy as np

from . import constants as K
from ._lib import lib

_PI = C.POINTER(C.c_int)
_PD = C.POINTER(C.c_double)


@dataclass
class WaterColumnPing:
    """Water column of one ping (all receive fans), as returned by mb_extract_wc.

    Per beam: swath (along track index, 0 = aftmost), angle_across (deg from
    vertical, positive starboard), angle_along (deg, positive forward),
    sample_interval (s), soundspeed (m/s), start_sample, nsamples,
    detect_sample (two-way bottom range in samples, 0 = none).
    amplitude: (nbeams, nsamples_max) float32 dB, NaN beyond nsamples of a beam;
    column k of beam i is sample number start_sample[i] + k.
    """
    time_d: float
    time_i: tuple
    swath: np.ndarray
    angle_across: np.ndarray
    angle_along: np.ndarray
    sample_interval: np.ndarray
    soundspeed: np.ndarray
    start_sample: np.ndarray
    nsamples: np.ndarray
    detect_sample: np.ndarray
    amplitude: np.ndarray = field(repr=False)
    ping_number: Optional[int] = None

    @property
    def nbeams(self):
        return len(self.swath)

    @property
    def nsamples_max(self):
        return self.amplitude.shape[1]

    def range_m(self):
        """Slant range (m) of every amplitude cell, shape like amplitude."""
        k = np.arange(self.nsamples_max)[None, :]
        t = (self.start_sample[:, None] + k) * self.sample_interval[:, None]
        return 0.5 * self.soundspeed[:, None] * t

    def detect_range_m(self):
        """Slant range (m) of each beam's bottom detection, NaN if none."""
        r = 0.5 * self.soundspeed * self.detect_sample * self.sample_interval
        return np.where(self.detect_sample > 0, r, np.nan)

    def _project(self, r):
        a = np.deg2rad(self.angle_across)[:, None] if np.ndim(r) == 2 else np.deg2rad(self.angle_across)
        b = np.deg2rad(self.angle_along)[:, None] if np.ndim(r) == 2 else np.deg2rad(self.angle_along)
        return r * np.cos(b) * np.sin(a), r * np.cos(b) * np.cos(a), r * np.sin(b)

    def xyz(self):
        """(across, down, along) in m from the transducer for every cell; straight rays, no
        sound speed profile."""
        return self._project(self.range_m())

    def detect_xyz(self):
        """(across, down, along) in m of each beam's bottom detection (NaN if none)."""
        return self._project(self.detect_range_m())

    def swaths(self):
        """Sorted swath indexes present in the ping."""
        return np.unique(self.swath)

    def select(self, beams):
        """New WaterColumnPing with only the given beams (index array or mask)."""
        beams = np.asarray(beams)
        a = self.amplitude[beams]
        ns = self.nsamples[beams]
        a = a[:, :max(int(ns.max()), 1)] if len(ns) else a[:, :0]
        return WaterColumnPing(self.time_d, self.time_i, self.swath[beams], self.angle_across[beams],
                               self.angle_along[beams], self.sample_interval[beams], self.soundspeed[beams],
                               self.start_sample[beams], ns, self.detect_sample[beams], a, self.ping_number)

    def split_swaths(self):
        """List of WaterColumnPing, one per swath (dual swath data has two)."""
        return [self.select(self.swath == s) for s in self.swaths()]


class _WCBuffers:
    """Reusable arrays passed to mb_extract_wc."""

    def __init__(self):
        self.nbeams_alloc = 0
        self.nsamples_alloc = 0

    def _ensure(self, nbeams, nsamples):
        if nbeams > self.nbeams_alloc or nsamples > self.nsamples_alloc:
            self.nbeams_alloc = max(nbeams, self.nbeams_alloc)
            self.nsamples_alloc = max(nsamples, self.nsamples_alloc)
            nb = self.nbeams_alloc
            self.swath = np.zeros(nb, np.int32)
            self.start = np.zeros(nb, np.int32)
            self.ns = np.zeros(nb, np.int32)
            self.across = np.zeros(nb)
            self.along = np.zeros(nb)
            self.interval = np.zeros(nb)
            self.c = np.zeros(nb)
            self.detect = np.zeros(nb)
            self.amp = np.zeros((nb, self.nsamples_alloc), np.float32)

    def extract(self, verbose, mbio, store):
        """Return a WaterColumnPing for the current record, or None if it has none."""
        kind, nbeams, nsmax, err = C.c_int(), C.c_int(), C.c_int(), C.c_int()
        if lib.mb_extract_wcdimensions(verbose, mbio, store, C.byref(kind), C.byref(nbeams), C.byref(nsmax),
                                       C.byref(err)) != K.MB_SUCCESS:
            return None
        self._ensure(nbeams.value, nsmax.value)
        time_i = (C.c_int * 7)()
        time_d = C.c_double()
        ip = lambda a: a.ctypes.data_as(_PI)  # noqa: E731
        dp = lambda a: a.ctypes.data_as(_PD)  # noqa: E731
        status = lib.mb_extract_wc(verbose, mbio, store, self.nbeams_alloc, self.nsamples_alloc, C.byref(kind),
                                   time_i, C.byref(time_d), C.byref(nbeams), C.byref(nsmax), ip(self.swath),
                                   dp(self.across), dp(self.along), dp(self.interval), dp(self.c), ip(self.start),
                                   ip(self.ns), dp(self.detect), self.amp.ctypes.data_as(C.POINTER(C.c_float)),
                                   C.byref(err))
        if status != K.MB_SUCCESS:
            return None
        n, m = nbeams.value, nsmax.value
        ns = self.ns[:n].copy()
        amp = self.amp[:n, :m].copy()
        amp[np.arange(m)[None, :] >= ns[:, None]] = np.nan
        return WaterColumnPing(time_d.value, tuple(time_i), self.swath[:n].copy(), self.across[:n].copy(),
                               self.along[:n].copy(), self.interval[:n].copy(), self.c[:n].copy(),
                               self.start[:n].copy(), ns, self.detect[:n].copy(), amp)


class WaterColumnFile:
    """All water column pings of one file in memory, for random access (viewer slider).

    Amplitudes are kept as float16 (exact for 0.5 dB steps) without padding;
    ping(i) returns a full WaterColumnPing (float32).
    fill_value: the lowest amplitude if it is a fill value (more than 0.1 % of the samples
    and the next 0.5 dB step practically absent, as the logged -128 of Kongsberg data),
    else None. amp_min/amp_max exclude the fill value.
    nav_time_d/nav_lon/nav_lat: the file's own position records (may be empty).
    """

    def __init__(self, path, format=0, progress=None):
        from .reader import Reader
        self.path = path
        self._geom = []
        self._offsets = [0]
        chunks = []
        self.other_kinds = {}
        self.svps = []    # (time_d, depth, velocity) of the file's sound velocity profiles
        nav = []
        with Reader(path, format) as r:
            self.format = r.format
            for rec in r:
                wc = rec.water_column
                if wc is None:
                    self.other_kinds[rec.kind] = self.other_kinds.get(rec.kind, 0) + 1
                    if rec.kind == K.MB_DATA_NAV and rec.ok and (rec.navlon != 0.0 or rec.navlat != 0.0):
                        nav.append((rec.time_d, rec.navlon, rec.navlat))
                    elif rec.svp_depth is not None:
                        self.svps.append((rec.time_d, rec.svp_depth, rec.svp_velocity))
                    continue
                valid = np.arange(wc.nsamples_max)[None, :] < wc.nsamples[:, None]
                data = wc.amplitude[valid].astype(np.float16)
                chunks.append(data)
                self._offsets.append(self._offsets[-1] + data.size)
                wc.amplitude = None
                self._geom.append(wc)
                if progress is not None:
                    progress(len(self._geom), min(1.0, r.wc_samples_read / max(r.file_size, 1)))
        self._amp = np.concatenate(chunks) if chunks else np.zeros(0, np.float16)
        self.times = np.array([g.time_d for g in self._geom])
        self.ping_numbers = np.array([g.ping_number for g in self._geom])
        nav = np.asarray(nav, float).reshape(-1, 3)
        self.nav_time_d, self.nav_lon, self.nav_lat = nav[:, 0], nav[:, 1], nav[:, 2]

        # value histogram (float16 bit patterns) for the range and the fill value
        self.fill_value = None
        self.amp_min = self.amp_max = np.nan
        if self._amp.size:
            counts = np.bincount(self._amp.view(np.uint16), minlength=65536)
            with np.errstate(invalid="ignore"):
                values = np.arange(65536, dtype=np.uint16).view(np.float16).astype(np.float64)
            present = (counts > 0) & np.isfinite(values)
            order = np.argsort(values[present])
            v, c = values[present][order], counts[present][order]
            if len(v) > 1 and c[0] > 0.001 * self._amp.size and c[1] < 0.01 * c[0]:
                self.fill_value = float(v[0])
                v = v[1:]
            if len(v):
                self.amp_min, self.amp_max = float(v[0]), float(v[-1])
        self.range_max_m = max((float(np.max((g.start_sample + g.nsamples) * g.sample_interval * g.soundspeed) / 2)
                                for g in self._geom if g.nbeams), default=0.0)

    def __len__(self):
        return len(self._geom)

    def swaths(self, i):
        """Sorted swath indexes of ping i."""
        return np.unique(self._geom[i].swath)

    def ping(self, i):
        g = self._geom[i]
        data = self._amp[self._offsets[i]:self._offsets[i + 1]].astype(np.float32)
        nsmax = int(g.nsamples.max()) if g.nbeams else 0
        amp = np.full((g.nbeams, nsmax), np.nan, np.float32)
        amp[np.arange(nsmax)[None, :] < g.nsamples[:, None]] = data
        return WaterColumnPing(g.time_d, g.time_i, g.swath, g.angle_across, g.angle_along, g.sample_interval,
                               g.soundspeed, g.start_sample, g.nsamples, g.detect_sample, amp, g.ping_number)

    @property
    def nbytes(self):
        return self._amp.nbytes
