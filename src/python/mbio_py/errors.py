#--------------------------------------------------------------------
#    The MB-system:  errors.py  1/11/2026
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
"""MB-System error messages and the exception raised for fatal errors."""
import ctypes as C

from ._lib import lib


def error_message(error):
    """Return the MB-System message for an MB_ERROR_* code (mb_error)."""
    msg = C.c_char_p()
    lib.mb_error(0, int(error), C.byref(msg))
    return msg.value.decode("ascii", "replace") if msg.value else f"MB-System error {error}"


class MBIOError(RuntimeError):
    """A fatal MBIO error (error code > 0)."""

    def __init__(self, function, error):
        self.function = function
        self.error = int(error)
        super().__init__(f"{function}: {error_message(error)} (error {error})")
