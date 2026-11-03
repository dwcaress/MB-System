#--------------------------------------------------------------------
#    The MB-system:  _common.py  1/11/2026
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
"""Shared helpers for the test scripts: import path and figure output."""
import os
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))


def finish(fig, out):
    """Save the figure to out (PNG) or show it."""
    if out:
        fig.savefig(out, dpi=150, bbox_inches="tight")
        print("wrote", out)
    else:
        import matplotlib.pyplot as plt
        plt.show()


def limit_to_data(ax, x, z, amplitude, margin=0.02):
    """Set the axis limits to the cells that have amplitude data."""
    import numpy as np
    ok = np.isfinite(amplitude)
    if ok.any():
        x0, x1, z0, z1 = x[ok].min(), x[ok].max(), z[ok].min(), z[ok].max()
        dx, dz = margin * (x1 - x0), margin * (z1 - z0)
        ax.set_xlim(x0 - dx, x1 + dx)
        ax.set_ylim(z1 + dz, z0 - dz)


def pyplot(out):
    import matplotlib
    if out:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    return plt
