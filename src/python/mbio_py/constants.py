#--------------------------------------------------------------------
#    The MB-system:  constants.py  1/11/2026
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
"""MB-System constants used by mbio_py.

Values copied from src/mbio/mb_status.h and src/mbio/mb_define.h;
tests/test_api.py checks them against the headers.
"""

MB_SUCCESS = 1
MB_FAILURE = 0

# record kinds (mb_status.h)
MB_DATA_NONE = 0
MB_DATA_DATA = 1
MB_DATA_COMMENT = 2
MB_DATA_VELOCITY_PROFILE = 6
MB_DATA_NAV = 12
MB_DATA_RUN_PARAMETER = 13
MB_DATA_NAV1 = 29
MB_DATA_NAV2 = 30
MB_DATA_NAV3 = 31
MB_DATA_INSTALLATION = 45
MB_DATA_WATER_COLUMN = 46
MB_DATA_HEAVE = 64
MB_DATA_KINDS = 75

NAV_KINDS = (MB_DATA_NAV, MB_DATA_NAV1, MB_DATA_NAV2, MB_DATA_NAV3)

# errors (mb_status.h): fatal > 0, non-fatal < 0
MB_ERROR_NO_ERROR = 0
MB_ERROR_MEMORY_FAIL = 1
MB_ERROR_OPEN_FAIL = 2
MB_ERROR_BAD_FORMAT = 3
MB_ERROR_EOF = 4
MB_ERROR_BAD_PARAMETER = 12
MB_ERROR_BAD_SYSTEM = 14
MB_ERROR_COMMENT = -5
MB_ERROR_OTHER = -8
MB_ERROR_UNINTELLIGIBLE = -9

# bottom detection types (mb_status.h)
MB_DETECT_UNKNOWN = 0
MB_DETECT_AMPLITUDE = 1
MB_DETECT_PHASE = 2
MB_DETECT_LIDAR = 3
MB_DETECT_PHOTOGRAMMETRY = 4

# beam flags (mb_status.h)
MB_FLAG_NONE = 0x00
MB_FLAG_FLAG = 0x01
MB_FLAG_NULL = 0x01
MB_FLAG_MANUAL = 0x04
MB_FLAG_FILTER = 0x08
MB_FLAG_FILTER2 = 0x10
MB_FLAG_MULTIPICK = 0x20
MB_FLAG_INTERPOLATE = 0x40
MB_FLAG_SONAR = 0x80

# array types for mb_register_array (mb_define.h)
MB_MEM_TYPE_BATHYMETRY = 1
MB_MEM_TYPE_AMPLITUDE = 2
MB_MEM_TYPE_SIDESCAN = 3

MB_COMMENT_MAXLINE = 1944  # mb_define.h

# format ids (check with mbformat -L)
MBF_EM710RAW = 58
MBF_RESON7KR = 88
MBF_RESON7K3 = 89
MBF_EM710RAW = 58
MBF_KEMKMALL = 261


def beam_ok(flag):
    """mb_beam_ok: beam not flagged (works on numpy uint8 arrays)."""
    return (flag & MB_FLAG_FLAG) == 0


def beam_null(flag):
    """mb_beam_check_flag_null: no detection."""
    return flag == MB_FLAG_NULL


def beam_flagged(flag):
    """mb_beam_check_flag: flagged (includes null beams)."""
    return (flag & MB_FLAG_FLAG) != 0


def beam_flagged_sonar(flag):
    """mb_beam_check_flag_sonar: flagged by the sonar."""
    return ((flag & MB_FLAG_SONAR) != 0) & ((flag & MB_FLAG_FLAG) != 0)
