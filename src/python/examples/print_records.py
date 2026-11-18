#--------------------------------------------------------------------
#    The MB-system:  print_records.py  1/11/2026
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
"""Print the record kinds and errors of a swath file, and the first record of each kind.

Usage: python3 print_records.py file [format]
"""
import sys
from collections import Counter

import _common  # noqa: F401
import mbio_py as mb

path = sys.argv[1]
fmt = int(sys.argv[2]) if len(sys.argv) > 2 else 0
kinds, errors, first = Counter(), Counter(), {}
with mb.Reader(path, fmt) as r:
    print(f"{path}: format {r.format}, libmbio {mb.lib_path}")
    for rec in r:
        kinds[rec.kind] += 1
        if rec.error:
            errors[rec.error] += 1
        first.setdefault(rec.kind, rec)
for k, n in sorted(kinds.items()):
    rec = first[k]
    line = f"kind {k:3d}: {n:7d} records, first at {rec.time_d:.6f}"
    if k == mb.constants.MB_DATA_DATA:
        line += (f", ping {rec.ping_number}, {len(rec.bath)} beams, lon {rec.navlon:.6f} lat {rec.navlat:.6f}"
                 f" heading {rec.heading:.2f} sensordepth {rec.sensordepth:.2f}")
    elif k == mb.constants.MB_DATA_WATER_COLUMN:
        wc = rec.water_column
        line += f", ping {rec.ping_number}, {wc.nbeams} beams, {wc.nsamples_max} samples max"
    elif k in mb.constants.NAV_KINDS:
        line += f", lon {rec.navlon:.6f} lat {rec.navlat:.6f} speed {rec.speed:.2f} km/h heading {rec.heading:.2f}"
    elif k == mb.constants.MB_DATA_COMMENT:
        line += f": {rec.comment[:60]}"
    print(line)
for e, n in sorted(errors.items()):
    print(f"non-fatal error {e} ({mb.error_message(e)}): {n} records")
