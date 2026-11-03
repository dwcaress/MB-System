#--------------------------------------------------------------------
#    The MB-system:  plot_paired.py  1/11/2026
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
"""Plot one water column ping with the soundings of the matching *.kmall ping on top,
both as depth below the waterline (transducer depth from the *.kmall ping).

Usage: python3 plot_paired.py file.kmwcd|file.kmall [ping_index] [out.png]
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

line = mb.KongsbergLine(path)
if line.wc is None:
    sys.exit("no *.kmwcd found for " + path)
wc = line.wc.ping(n)
j = line.survey_index(n)
draft = line.survey.sensordepth[j] if j is not None else 0.0
lon, lat, sog, cog = line.nav_at(wc.time_d)

swaths = wc.split_swaths()
fig, axes = plt.subplots(1, len(swaths), figsize=(6 * len(swaths), 5), squeeze=False)
for ax, s in zip(axes[0], swaths):
    x, z, _ = s.xyz()
    m = ax.pcolormesh(x, z + draft, s.amplitude, shading="nearest", cmap="viridis", vmin=-60, vmax=0)
    if j is not None:
        flag, bz, bx, det = (line.survey.beamflag[j], line.survey.bath[j], line.survey.bathacrosstrack[j],
                             line.survey.detects[j])
        # soundings of all swaths of the ping are in one MBIO ping; keep those of this swath by
        # their order (MBIO keeps the receive fans in order, beams of fan k follow fan k-1)
        nfan = len(bz) // len(swaths)
        sel = slice(nfan * int(s.swath[0]), nfan * (int(s.swath[0]) + 1))
        f, zz, xx, dd = flag[sel], bz[sel], bx[sel], det[sel]
        for code, colour in ((K.MB_DETECT_AMPLITUDE, "red"), (K.MB_DETECT_PHASE, "darkblue")):
            good = (dd == code) & K.beam_ok(f)
            bad = (dd == code) & K.beam_flagged(f) & ~K.beam_null(f)
            ax.plot(xx[good], zz[good], "o", ms=2.5, color=colour)
            ax.plot(xx[bad], zz[bad], "o", ms=2.5, mfc="none", color=colour)
    _common.limit_to_data(ax, x, z + draft, s.amplitude)
    ax.set_aspect("equal")
    ax.set_xlabel("across track (m, starboard +)")
    ax.set_ylabel("depth below waterline (m)")
    ax.set_title(f"swath {s.swath[0]}")
fig.colorbar(m, ax=axes[0].tolist(), label="amplitude (dB)")
fig.suptitle(f"ping {wc.ping_number}  {lat:.6f} {lon:.6f}  SOG {sog:.1f} kn  COG {cog:.1f}"
             f"  heading {line.heading_at(wc.time_d):.1f}  ({'kmall ping ' + str(j) if j is not None else 'no kmall ping'})")
_common.finish(fig, out)
