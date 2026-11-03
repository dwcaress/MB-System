#--------------------------------------------------------------------
#    The MB-system:  _lib.py  1/11/2026
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
"""Load libmbio and declare the C signatures used by mbio_py."""
import ctypes as C
import ctypes.util
import os

_I = C.c_int
_D = C.c_double
_PI = C.POINTER(C.c_int)
_PD = C.POINTER(C.c_double)
_PF = C.POINTER(C.c_float)
_PV = C.c_void_p
_PPV = C.POINTER(C.c_void_p)


def _candidates():
    """libmbio locations in search order: MBSYSTEM_LIBMBIO, the library of the MB-System
    installation this package belongs to (_config.py, written at build time), the system
    library search path."""
    env = os.environ.get("MBSYSTEM_LIBMBIO")
    if env:
        yield env
    try:
        from . import _config
        if _config.LIBMBIO:
            yield _config.LIBMBIO
        for name in ("libmbio.dylib", "libmbio.so", "libmbio.dll", "libmbio-0.dll"):
            yield os.path.join(_config.LIBDIR, name)
    except ImportError:
        pass
    for name in ("mbio", "libmbio"):
        found = ctypes.util.find_library(name)
        if found:
            yield found


def _load():
    errors = []
    for path in _candidates():
        try:
            return C.CDLL(path), path
        except OSError as ex:
            errors.append(f"{path}: {ex}")
    raise OSError("libmbio not found; set MBSYSTEM_LIBMBIO to the full path of libmbio\n" + "\n".join(errors))


lib, lib_path = _load()


def _decl(name, argtypes, restype=_I):
    f = getattr(lib, name)
    f.argtypes = argtypes
    f.restype = restype


_decl("mb_get_format", [_I, C.c_char_p, C.c_char_p, _PI, _PI])
_decl("mb_read_init", [_I, C.c_char_p, _I, _I, _I, _PD, _PI, _PI, _D, _D, _PPV, _PD, _PD, _PI, _PI, _PI, _PI])
_decl("mb_close", [_I, _PPV, _PI])
_decl("mb_register_array", [_I, _PV, _I, C.c_size_t, _PPV, _PI])
_decl("mb_get_all", [_I, _PV, _PPV, _PI, _PI, _PD, _PD, _PD, _PD, _PD, _PD, _PD, _PD, _PI, _PI, _PI,
                     _PV, _PV, _PV, _PV, _PV, _PV, _PV, _PV, C.c_char_p, _PI])
_decl("mb_pingnumber", [_I, _PV, C.POINTER(C.c_uint), _PI])
_decl("mb_detects", [_I, _PV, _PV, _PI, _PI, _PI, _PI])
_decl("mb_extract_wcdimensions", [_I, _PV, _PV, _PI, _PI, _PI, _PI])
_decl("mb_extract_wc", [_I, _PV, _PV, _I, _I, _PI, _PI, _PD, _PI, _PI, _PI, _PD, _PD, _PD, _PD, _PI, _PI,
                        _PD, _PF, _PI])
_decl("mb_extract_svp", [_I, _PV, _PV, _PI, _PI, _PD, _PD, _PI])
_decl("mb_error", [_I, _I, C.POINTER(C.c_char_p)])
