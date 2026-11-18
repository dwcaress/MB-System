#--------------------------------------------------------------------
#    The MB-system:  plot_nav.py  1/11/2026
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
"""Plot the track, speed and heading of a *.kmall.

Usage: python3 plot_nav.py file.kmall [out.png]
"""
import sys

import numpy as np

import _common
import mbio_py as mb

path = sys.argv[1]
out = sys.argv[2] if len(sys.argv) > 2 else None
plt = _common.pyplot(out)

s = mb.load_survey(path)
t = (s.time_d - s.time_d[0]) / 60.0
fig, ax = plt.subplots(1, 3, figsize=(15, 4.5))
if len(s.nav_time_d):
    ax[0].plot(s.nav_lon, s.nav_lat, "-", color="0.6", lw=1, label="position records")
ax[0].plot(s.navlon, s.navlat, ".", ms=3, label="pings")
ax[0].set_xlabel("longitude")
ax[0].tick_params(axis="x", labelrotation=45)
ax[0].set_ylabel("latitude")
ax[0].legend(fontsize=8)
ax[1].plot(t, s.speed / 1.852, ".-", ms=3)
ax[1].set_xlabel("minutes")
ax[1].set_ylabel("speed (kn)")
ax[2].plot(t, s.heading, ".-", ms=3)
ax[2].set_xlabel("minutes")
ax[2].set_ylabel("heading (deg)")
fig.suptitle(f"{path.split('/')[-1]}: {len(s)} pings, {len(s.nav_time_d)} position records")
_common.finish(fig, out)
