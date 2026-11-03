#--------------------------------------------------------------------
#    The MB-system:  print_wc_ping.py  1/11/2026
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
"""Print the water column of one ping.

Usage: python3 print_wc_ping.py file.kmwcd [ping_index]
"""
import sys

import numpy as np

import _common  # noqa: F401
import mbio_py as mb

path = sys.argv[1]
n = int(sys.argv[2]) if len(sys.argv) > 2 else 0
i = -1
with mb.Reader(path) as r:
    for rec in r:
        if rec.water_column is not None:
            i += 1
            if i == n:
                wc = rec.water_column
                break
    else:
        sys.exit(f"only {i + 1} water column pings in {path}")

print(f"ping index {n}: ping number {wc.ping_number}, time {wc.time_d:.6f} {wc.time_i}")
print(f"beams {wc.nbeams}, samples max {wc.nsamples_max}")
for s in wc.split_swaths():
    print(f"swath {s.swath[0]}: {s.nbeams} beams, angle across {s.angle_across[0]:.2f}..{s.angle_across[-1]:.2f} deg,"
          f" along {np.unique(np.round(s.angle_along, 3))} deg")
    print(f"  sample interval {s.sample_interval[0] * 1e3:.4f} ms ({1 / s.sample_interval[0]:.3f} Hz),"
          f" sound speed {s.soundspeed[0]:.2f} m/s, start samples {s.start_sample.min()}..{s.start_sample.max()},"
          f" samples per beam {s.nsamples.min()}..{s.nsamples.max()}")
    det = s.detect_range_m()
    print(f"  bottom detections {np.isfinite(det).sum()}, slant range {np.nanmin(det):.1f}..{np.nanmax(det):.1f} m")
    a = s.amplitude
    print(f"  amplitude dB min {np.nanmin(a):.1f} max {np.nanmax(a):.1f} median {np.nanmedian(a):.1f}")
j = wc.nbeams // 4
print(f"beam {j}: first 10 amplitudes {wc.amplitude[j, :10]}")
