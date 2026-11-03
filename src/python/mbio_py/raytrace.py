#--------------------------------------------------------------------
#    The MB-system:  raytrace.py  1/11/2026
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
"""Ray tracing of water column samples through a sound velocity profile.

Each beam leaves the transducer at its takeoff angle (from angle_across and angle_along);
the ray parameter p = sin(takeoff) / c0 uses the sound speed at the transducer logged with
the water column (the speed the beamformer steered with). The profile is split into thin
layers of constant speed; per layer sin(theta) = p * c (Snell), the one-way travel time and
the horizontal distance are accumulated, and every sample's one-way time (half its two-way
time) is looked up in its beam's table. Rays that turn back (p * c >= 1) end there.
Depths are below the waterline: the trace starts at the transducer depth (draft).
No motion (heave, pitch, roll during the ping) is applied, as for the straight-ray geometry.
"""
import numpy as np


class _Tables:
    def __init__(self, wc, svp_depth, svp_velocity, draft, dz):
        a = np.deg2rad(wc.angle_across)
        b = np.deg2rad(wc.angle_along)
        across, down, along = np.cos(b) * np.sin(a), np.cos(b) * np.cos(a), np.sin(b)
        horiz = np.hypot(across, along)
        self.h_across = np.divide(across, horiz, out=np.zeros_like(horiz), where=horiz > 0)
        self.h_along = np.divide(along, horiz, out=np.zeros_like(horiz), where=horiz > 0)
        p = np.clip(horiz, 0.0, 1.0) / wc.soundspeed            # ray parameter (s/m)

        order = np.argsort(svp_depth)
        d, c = np.asarray(svp_depth, float)[order], np.asarray(svp_velocity, float)[order]
        t_max = 0.5 * float(np.max((wc.start_sample + wc.nsamples) * wc.sample_interval)) if wc.nbeams else 0.0
        z_max = draft + t_max * max(float(c.max()), float(wc.soundspeed.max())) * 1.01
        if dz is None:
            dz = max(0.5, (z_max - draft) / 3000.0)
        nz = max(int(np.ceil((z_max - draft) / dz)), 1)
        self.z = draft + dz * np.arange(nz + 1)                    # layer boundaries
        cm = np.interp(0.5 * (self.z[:-1] + self.z[1:]), d, c)    # speed in each layer

        s = p[:, None] * cm[None, :]                              # sin(theta) per beam and layer
        turned = np.cumsum(s >= 1.0, axis=1) > 0                  # ray turned back at or above
        s = np.where(turned, 0.0, s)
        cos = np.sqrt(1.0 - s * s)
        dt = np.where(turned, np.inf, dz / (cm[None, :] * cos))
        dx = np.where(turned, 0.0, dz * s / cos)
        zero = np.zeros((len(p), 1))
        self.t = np.hstack([zero, np.cumsum(dt, axis=1)])        # one-way time at each boundary
        self.x = np.hstack([zero, np.cumsum(dx, axis=1)])        # horizontal distance

    def locate(self, one_way_time):
        """(across, depth, along) for one-way times, shape (nbeams, n) or (nbeams,)."""
        tt = np.atleast_2d(one_way_time.T).T if one_way_time.ndim == 1 else one_way_time
        depth = np.full(tt.shape, np.nan)
        dist = np.full(tt.shape, np.nan)
        for i in range(tt.shape[0]):
            ti = self.t[i]
            ok = np.isfinite(tt[i]) & (tt[i] <= ti[np.isfinite(ti)][-1])
            fin = np.isfinite(ti)
            depth[i, ok] = np.interp(tt[i, ok], ti[fin], self.z[fin])
            dist[i, ok] = np.interp(tt[i, ok], ti[fin], self.x[i, fin])
        out = (dist * self.h_across[:, None], depth, dist * self.h_along[:, None])
        if one_way_time.ndim == 1:
            out = tuple(o[:, 0] for o in out)
        return out


def raytrace(wc, svp_depth, svp_velocity, draft=0.0, dz=None):
    """Ray traced (across, depth below waterline, along) in m of every amplitude cell of a
    WaterColumnPing, shapes like wc.amplitude, and of each beam's bottom detection (NaN if
    none): returns (cells, detections), each a tuple (across, depth, along)."""
    tables = _Tables(wc, svp_depth, svp_velocity, draft, dz)
    k = np.arange(wc.nsamples_max)[None, :]
    t_cells = 0.5 * (wc.start_sample[:, None] + k) * wc.sample_interval[:, None]
    t_det = np.where(wc.detect_sample > 0, 0.5 * wc.detect_sample * wc.sample_interval, np.nan)
    return tables.locate(t_cells), tables.locate(t_det)
