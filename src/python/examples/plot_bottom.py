#--------------------------------------------------------------------
#    The MB-system:  plot_bottom.py  1/11/2026
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
"""Plot the soundings of one *.kmall ping: acrosstrack vs depth, coloured by detection method,
flagged soundings hollow, null beams left out.

Usage: python3 plot_bottom.py file.kmall [ping_index] [out.png]
"""
import sys

import numpy as np

import _common
import mbio_py as mb
from mbio_py import constants as K

path = sys.argv[1]
n = int(sys.argv[2]) if len(sys.argv) > 2 else 0
out = sys.argv[3] if len(sys.argv) > 3 else None
plt = _common.pyplot(out)

s = mb.load_survey(path)
flag, z, x, det = s.beamflag[n], s.bath[n], s.bathacrosstrack[n], s.detects[n]
fig, ax = plt.subplots(figsize=(9, 4.5))
for code, name, colour in ((K.MB_DETECT_AMPLITUDE, "amplitude", "red"), (K.MB_DETECT_PHASE, "phase", "darkblue"),
                           (K.MB_DETECT_UNKNOWN, "unknown", "gray")):
    good = (det == code) & K.beam_ok(flag)
    bad = (det == code) & K.beam_flagged(flag) & ~K.beam_null(flag)
    ax.plot(x[good], z[good], "o", ms=3, color=colour, label=f"{name} ({good.sum()})")
    ax.plot(x[bad], z[bad], "o", ms=3, mfc="none", color=colour, label=f"{name} flagged ({bad.sum()})")
ax.invert_yaxis()
ax.set_xlabel("across track (m, starboard +)")
ax.set_ylabel("depth below waterline (m)")
ax.set_title(f"{path.split('/')[-1]} ping {s.ping_number[n]}: {len(z)} beams, "
             f"{K.beam_null(flag).sum()} null")
ax.legend(fontsize=8)
_common.finish(fig, out)
