#--------------------------------------------------------------------
#    The MB-system:  reader.py  1/11/2026
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
"""Sequential reading of swath files through MBIO (mb_read_init, mb_get_all, mb_close)."""
import ctypes as C
import os
from dataclasses import dataclass, field
from typing import Optional

import numpy as np

from . import constants as K
from ._lib import lib
from .errors import MBIOError
from .watercolumn import WaterColumnPing, _WCBuffers

# sound velocity profile buffers (the kmall driver holds up to 2000 points)
_SVP_ALLOC = 10000

# file suffixes that mb_get_format() does not recognise on purpose
_SUFFIX_FORMATS = {".kmwcd": K.MBF_KEMKMALL}


def get_format(path):
    """Return the MBIO format id of a file name (0 if unknown); *.kmwcd gives 261."""
    suffix = os.path.splitext(path)[1].lower()
    if suffix in _SUFFIX_FORMATS:
        return _SUFFIX_FORMATS[suffix]
    fmt, err = C.c_int(0), C.c_int(0)
    root = C.create_string_buffer(4096)
    lib.mb_get_format(0, os.fsencode(path), root, C.byref(fmt), C.byref(err))
    return fmt.value


@dataclass
class Record:
    """One record returned by mb_get_all, with its arrays copied.

    Units as in MBIO: time_d seconds since 1970, navlon/navlat degrees,
    speed km/h, heading degrees, distance km, altitude/sensordepth m,
    bath m positive down, bathacrosstrack m positive starboard.
    error is the (non-fatal) MB_ERROR_* code of the record, 0 if none.
    """
    kind: int
    error: int
    time_d: float
    time_i: tuple
    navlon: float
    navlat: float
    speed: float
    heading: float
    distance: float
    altitude: float
    sensordepth: float
    beamflag: np.ndarray = field(repr=False)
    bath: np.ndarray = field(repr=False)
    bathacrosstrack: np.ndarray = field(repr=False)
    bathalongtrack: np.ndarray = field(repr=False)
    amp: np.ndarray = field(repr=False)
    comment: str = ""
    ping_number: Optional[int] = None
    detects: Optional[np.ndarray] = field(default=None, repr=False)
    water_column: Optional[WaterColumnPing] = field(default=None, repr=False)
    svp_depth: Optional[np.ndarray] = field(default=None, repr=False)     # m, sound velocity profile
    svp_velocity: Optional[np.ndarray] = field(default=None, repr=False)  # m/s (kind MB_DATA_VELOCITY_PROFILE)

    @property
    def ok(self):
        return self.error == K.MB_ERROR_NO_ERROR


def _array(handle, ctype, n):
    if n <= 0 or not handle.value:
        return np.zeros(0, dtype=ctype)
    ptr = C.cast(handle.value, C.POINTER(np.ctypeslib.as_ctypes_type(ctype)))
    return np.ctypeslib.as_array(ptr, shape=(n,)).copy()


class Reader:
    """Read a swath file record by record through MBIO.

    with Reader("0080_20250618_223434.kmwcd") as r:
        for rec in r:
            if rec.water_column is not None: ...

    format 0 means: from the file name (mb_get_format; *.kmwcd -> 261).
    Fatal errors raise MBIOError; non-fatal ones are returned in Record.error.
    Survey and water column records carry ping_number; survey records carry
    detects (MB_DETECT_*); water column data is extracted when present.
    """

    def __init__(self, path, format=0, verbose=0, timegap=1.0e6):
        self.path = path
        self.verbose = verbose
        self.format = format if format else get_format(path)
        if self.format == 0:
            raise MBIOError("mb_get_format " + path, K.MB_ERROR_BAD_FORMAT)
        self.file_size = os.path.getsize(path)
        bounds = (C.c_double * 4)(-360.0, 360.0, -90.0, 90.0)
        btime = (C.c_int * 7)(1962, 2, 21, 10, 30, 0, 0)
        etime = (C.c_int * 7)(2062, 2, 21, 10, 30, 0, 0)
        btime_d, etime_d = C.c_double(), C.c_double()
        nbath, namp, nss, err = C.c_int(), C.c_int(), C.c_int(), C.c_int()
        self._mbio = C.c_void_p()
        status = lib.mb_read_init(verbose, os.fsencode(path), self.format, 1, 0, bounds, btime, etime, 0.0,
                                  timegap, C.byref(self._mbio), C.byref(btime_d), C.byref(etime_d),
                                  C.byref(nbath), C.byref(namp), C.byref(nss), C.byref(err))
        if status != K.MB_SUCCESS:
            self._mbio = C.c_void_p()
            raise MBIOError("mb_read_init " + path, err.value)

        # output arrays owned by MBIO, reallocated by it when beam counts grow
        self._h = {name: C.c_void_p() for name in
                   ("beamflag", "bath", "amp", "bathacrosstrack", "bathalongtrack", "ss", "ssacrosstrack",
                    "ssalongtrack")}
        types = {"beamflag": (K.MB_MEM_TYPE_BATHYMETRY, 1), "bath": (K.MB_MEM_TYPE_BATHYMETRY, 8),
                 "amp": (K.MB_MEM_TYPE_AMPLITUDE, 8), "bathacrosstrack": (K.MB_MEM_TYPE_BATHYMETRY, 8),
                 "bathalongtrack": (K.MB_MEM_TYPE_BATHYMETRY, 8), "ss": (K.MB_MEM_TYPE_SIDESCAN, 8),
                 "ssacrosstrack": (K.MB_MEM_TYPE_SIDESCAN, 8), "ssalongtrack": (K.MB_MEM_TYPE_SIDESCAN, 8)}
        for name, (mtype, size) in types.items():
            if lib.mb_register_array(verbose, self._mbio, mtype, size, C.byref(self._h[name]),
                                     C.byref(err)) != K.MB_SUCCESS:
                self.close()
                raise MBIOError("mb_register_array", err.value)
        self._store = C.c_void_p()
        self._comment = C.create_string_buffer(K.MB_COMMENT_MAXLINE)
        self._wc = _WCBuffers()
        self.wc_samples_read = 0

    def close(self):
        if self._mbio:
            err = C.c_int()
            lib.mb_close(self.verbose, C.byref(self._mbio), C.byref(err))
            self._mbio = C.c_void_p()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def __del__(self):
        self.close()

    def __iter__(self):
        while True:
            rec = self.read()
            if rec is None:
                return
            yield rec

    def read(self):
        """Read the next record; None at end of file."""
        if not self._mbio:
            return None
        kind, err = C.c_int(), C.c_int()
        time_i = (C.c_int * 7)()
        d = {n: C.c_double() for n in ("time_d", "navlon", "navlat", "speed", "heading", "distance", "altitude",
                                         "sensordepth")}
        nbath, namp, nss = C.c_int(), C.c_int(), C.c_int()
        h = self._h
        status = lib.mb_get_all(self.verbose, self._mbio, C.byref(self._store), C.byref(kind), time_i,
                                *(C.byref(d[n]) for n in ("time_d", "navlon", "navlat", "speed", "heading",
                                                          "distance", "altitude", "sensordepth")),
                                C.byref(nbath), C.byref(namp), C.byref(nss),
                                h["beamflag"], h["bath"], h["amp"], h["bathacrosstrack"], h["bathalongtrack"],
                                h["ss"], h["ssacrosstrack"], h["ssalongtrack"], self._comment, C.byref(err))
        if err.value == K.MB_ERROR_EOF:
            return None
        if err.value > 0:
            raise MBIOError("mb_get_all " + self.path, err.value)

        rec = Record(kind=kind.value, error=err.value if status != K.MB_SUCCESS else 0,
                     time_d=d["time_d"].value, time_i=tuple(time_i), navlon=d["navlon"].value,
                     navlat=d["navlat"].value, speed=d["speed"].value, heading=d["heading"].value,
                     distance=d["distance"].value, altitude=d["altitude"].value,
                     sensordepth=d["sensordepth"].value,
                     beamflag=_array(h["beamflag"], np.uint8, nbath.value),
                     bath=_array(h["bath"], np.float64, nbath.value),
                     bathacrosstrack=_array(h["bathacrosstrack"], np.float64, nbath.value),
                     bathalongtrack=_array(h["bathalongtrack"], np.float64, nbath.value),
                     amp=_array(h["amp"], np.float64, namp.value),
                     comment=self._comment.value.decode("latin-1") if kind.value == K.MB_DATA_COMMENT else "")

        if status == K.MB_SUCCESS and rec.kind == K.MB_DATA_VELOCITY_PROFILE:
            # mb_extract_svp has no array size argument: use buffers larger than any format's profile
            depth, velocity = np.zeros(_SVP_ALLOC), np.zeros(_SVP_ALLOC)
            nsvp, sk = C.c_int(0), C.c_int()
            if lib.mb_extract_svp(self.verbose, self._mbio, self._store, C.byref(sk), C.byref(nsvp),
                                  depth.ctypes.data_as(C.POINTER(C.c_double)),
                                  velocity.ctypes.data_as(C.POINTER(C.c_double)),
                                  C.byref(err)) == K.MB_SUCCESS and 0 < nsvp.value <= _SVP_ALLOC:
                rec.svp_depth, rec.svp_velocity = depth[:nsvp.value].copy(), velocity[:nsvp.value].copy()

        if status == K.MB_SUCCESS and rec.kind in (K.MB_DATA_DATA, K.MB_DATA_WATER_COLUMN):
            ping = C.c_uint()
            lib.mb_pingnumber(self.verbose, self._mbio, C.byref(ping), C.byref(err))
            rec.ping_number = ping.value
            if rec.kind == K.MB_DATA_DATA and nbath.value > 0:
                det = np.zeros(nbath.value, np.int32)
                nb, dk = C.c_int(), C.c_int()
                if lib.mb_detects(self.verbose, self._mbio, self._store, C.byref(dk), C.byref(nb),
                                  det.ctypes.data_as(C.POINTER(C.c_int)), C.byref(err)) == K.MB_SUCCESS:
                    rec.detects = det[:nb.value]
            rec.water_column = self._wc.extract(self.verbose, self._mbio, self._store)
            if rec.water_column is not None:
                rec.water_column.ping_number = rec.ping_number
                self.wc_samples_read += int(rec.water_column.nsamples.sum())
        return rec
