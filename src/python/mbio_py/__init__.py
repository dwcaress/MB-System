#--------------------------------------------------------------------
#    The MB-system:  __init__.py  1/11/2026
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
"""mbio_py: Python access to MB-System's MBIO library (libmbio) through ctypes.

Reader            record by record reading of any MBIO format
WaterColumnPing   water column of one ping (mb_extract_wc)
WaterColumnFile   all water column pings of a file in memory
KongsbergLine     a *.kmall + *.kmwcd pair of one survey line
"""
from . import constants
from ._lib import lib_path
from .errors import MBIOError, error_message
from .raytrace import raytrace
from .reader import Reader, Record, get_format
from .survey import KongsbergLine, SurveyPings, find_companion, load_survey
from .watercolumn import WaterColumnFile, WaterColumnPing

__all__ = ["constants", "lib_path", "MBIOError", "error_message", "Reader", "Record", "get_format",
           "KongsbergLine", "SurveyPings", "find_companion", "load_survey", "WaterColumnFile",
           "WaterColumnPing", "raytrace"]
