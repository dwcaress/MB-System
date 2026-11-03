#--------------------------------------------------------------------
#    The MB-system:  plot_wc_ping.py  1/11/2026
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
"""Plot the water column of one ping, each swath separately, with the bottom detections
of the water column beams.

Usage: python3 plot_wc_ping.py file.kmwcd [ping_index] [out.png]
"""
import sys

import numpy as np

import _common
import mbio_py as mb

path = sys.argv[1]
n = int(sys.argv[2]) if len(sys.argv) > 2 else 0
out = sys.argv[3] if len(sys.argv) > 3 else None
plt = _common.pyplot(out)

wcf = mb.WaterColumnFile(path)
wc = wcf.ping(n)
swaths = wc.split_swaths()
fig, axes = plt.subplots(1, len(swaths), figsize=(6 * len(swaths), 5), squeeze=False)
for ax, s in zip(axes[0], swaths):
    x, z, _ = s.xyz()
    m = ax.pcolormesh(x, z, s.amplitude, shading="nearest", cmap="viridis", vmin=-60, vmax=0)
    dx, dz, _ = s.detect_xyz()
    ax.plot(dx, dz, ".", color="red", ms=2, label="WC bottom detection")
    _common.limit_to_data(ax, x, z, s.amplitude)
    ax.set_aspect("equal")
    ax.set_xlabel("across track (m, starboard +)")
    ax.set_ylabel("depth below transducer (m)")
    ax.set_title(f"ping {wc.ping_number} swath {s.swath[0]}")
    ax.legend(loc="lower right", fontsize=8)
fig.colorbar(m, ax=axes[0].tolist(), label="amplitude (dB)")
fig.suptitle(f"{path.split('/')[-1]}  ping index {n}")
_common.finish(fig, out)
