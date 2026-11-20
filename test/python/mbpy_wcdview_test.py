#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_wcdview_test.py  1/11/2026
#
#    Copyright (c) 2026 by
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
"""The Python water column viewer (mbpy_wcdview, wcd_viewer.py): usage always; with PySide6
and matplotlib installed, an offscreen session on a synthetic *.kmall/*.kmwcd pair, *.wcd
file and Reson 7k *.s7k file."""

import importlib.util
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_all as A  # noqa: E402
import synth_kmall as S  # noqa: E402
import synth_s7k as R  # noqa: E402

SOURCE = os.environ["MB_SOURCE_DIR"]
LAUNCHER = os.path.join(SOURCE, "src", "python", "mbpy_wcdview")
HAVE_GUI = all(importlib.util.find_spec(m) is not None for m in ("PySide6", "matplotlib"))


class MbpyWcdviewTest(unittest.TestCase):

  def test_usage(self):
    result = subprocess.run([sys.executable, LAUNCHER, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("usage: mbpy_wcdview", result.stderr)

  @unittest.skipUnless(HAVE_GUI, "PySide6 and matplotlib not installed")
  def test_redraws_keep_one_colour_bar(self):
    """Every redraw (slider step, replay tick, check box, spin box) updates the one colour bar,
    so that the earlier swath images are freed: a new colour bar per redraw kept all the earlier
    ones and their images alive, and the 985th redraw raised RecursionError."""
    os.environ["QT_QPA_PLATFORM"] = "offscreen"
    sys.path.insert(0, os.path.join(SOURCE, "src", "python", "wcd_viewer"))
    import gc
    import weakref
    import wcd_viewer
    from matplotlib.collections import QuadMesh
    from matplotlib.colorbar import Colorbar
    from PySide6.QtWidgets import QApplication
    app = QApplication.instance() or QApplication([])

    def colour_bars(w):
      gc.collect()
      return sum(isinstance(o, Colorbar) and o.ax is w.cax for o in gc.get_objects())
    with tempfile.TemporaryDirectory() as tmp:
      kmall, kmwcd = S.write_line(tmp)
      w = wcd_viewer.MainWindow()
      w.open_file(kmwcd)
      image = weakref.ref(next(m for m in w.ax.collections if isinstance(m, QuadMesh)))
      for step in range(1, 51):
        w.on_slide(step % len(w.steps))  # the slider's slot, so that exceptions reach the test
      self.assertEqual(colour_bars(w), 1)
      self.assertIsNone(image(), "the swath image of the first drawing is still alive")
      self.assertEqual(sum(isinstance(m, QuadMesh) for m in w.cax.collections), 1)
      self.assertEqual(w.cax.get_ylabel(), "Amplitude (dB)")
      # the colour bar follows the amplitude limits
      w.amin.setValue(w.amin.value() + 5.0)
      w.amax.setValue(w.amax.value() - 5.0)
      for limit, expected in zip(w.cax.get_ylim(), (w.amin.value(), w.amax.value())):
        self.assertAlmostEqual(limit, expected, places=6)
      # no colour bar while a line without water column is open; the next line has it again
      survey_only = os.path.join(tmp, "survey_only")
      os.mkdir(survey_only)
      only_kmall, only_kmwcd = S.write_line(survey_only)
      os.remove(only_kmwcd)
      w.open_file(only_kmall)
      self.assertEqual(w.steps, [])
      self.assertFalse(w.cax.get_visible())
      w.open_file(kmwcd)
      self.assertTrue(w.cax.get_visible())
      self.assertEqual(w.cax.get_ylabel(), "Amplitude (dB)")
      self.assertEqual(colour_bars(w), 1)
      w.close()
    del app

  @unittest.skipUnless(HAVE_GUI, "PySide6 and matplotlib not installed")
  def test_offscreen_session(self):
    os.environ["QT_QPA_PLATFORM"] = "offscreen"
    sys.path.insert(0, os.path.join(SOURCE, "src", "python", "wcd_viewer"))
    import wcd_viewer
    from PySide6.QtWidgets import QApplication
    app = QApplication.instance() or QApplication([])
    # an exception in a slot is only printed: print and collect them, and fail at the end
    errors = []
    excepthook = sys.excepthook

    def collect(*exc):
      errors.append(exc)
      excepthook(*exc)
    sys.excepthook = collect
    self.addCleanup(setattr, sys, "excepthook", excepthook)
    with tempfile.TemporaryDirectory() as tmp:
      kmall, kmwcd = S.write_line(tmp)
      w = wcd_viewer.MainWindow()
      w.open_file(kmwcd)
      self.assertEqual(len(w.steps), 2 * (S.NPINGS - 1) - 1)  # the last ping has one swath
      self.assertEqual(w.line.matched, S.NPINGS - 1)
      self.assertTrue(w.chk_raytrace.isChecked())
      # the fill value is no data only before the first echo of a beam: NFILL fill samples
      # start every beam, and the one inside beam 1 is drawn with the lowest colour
      import numpy as np
      w.redraw()
      amp = w._last_grid["amp"]
      self.assertTrue(np.isnan(amp[:, :S.NFILL]).all())
      self.assertAlmostEqual(float(amp[1, S.INNER_FILL]), 0.5 * -128 - S.TVG_OFFSET, places=3)
      w.chk_bottom.setChecked(True)
      w.chk_wcbottom.setChecked(True)
      w.slider.setValue(2)
      self.assertIn("Ping %d swath 0" % (S.PING0 + 2), w.meta_label.text())
      self.assertIn("soundings", w.statusBar().currentMessage())
      row = w.pick(0.0, S.DRAFT + 0.5 * S.BOTTOM_BELOW_TX)
      self.assertIsNotNone(row)
      self.assertEqual(row[-1], "Heading")
      image = os.path.join(tmp, "ping.png")
      w.save_image(image)
      self.assertTrue(os.path.getsize(image) > 0)
      w.close()
      # a *.wcd without its *.all, with a ping that lost a datagram
      w = wcd_viewer.MainWindow()
      w.open_file(A.write(os.path.join(tmp, A.WCD_NAME)))
      self.assertEqual(len(w.steps), len(A.PINGS))
      self.assertIsNone(w.line.survey)
      w.slider.setValue(2)
      self.assertIn("Ping %d swath 0" % A.PINGS[2], w.meta_label.text())
      w.close()
      # a Reson 7k *.s7k holds the survey pings and the water column
      w = wcd_viewer.MainWindow()
      w.open_file(R.write(tmp))
      nwc = R.NPINGS - len(R.NO_WC)
      self.assertEqual(len(w.steps), nwc)
      self.assertEqual(w.line.matched, nwc)
      # the sonar is far below the surface: the depth window starts just above it
      self.assertAlmostEqual(w.dmin.value(), R.SENSORDEPTH - 0.1 * w.line.wc.range_max_m, delta=0.01)
      # ping 0 has more detections (soundings) than beams
      w.chk_bottom.setChecked(True)
      self.assertIn("soundings: %d" % (R.NBEAMS + 1), w.statusBar().currentMessage())
      w.chk_wcbottom.setChecked(True)
      w.slider.setValue(1)
      self.assertIn("Ping %d swath 0" % (R.PING0 + 1), w.meta_label.text())
      self.assertIn("soundings: %d" % R.NBEAMS, w.statusBar().currentMessage())
      w.close()
      # a water column ping without its survey ping, deep below the surface: drawn at the
      # transducer depth interpolated between the pings around it (3010 and 3030 m), inside
      # the default window
      deep = os.path.join(tmp, "deep")
      os.mkdir(deep)
      w = wcd_viewer.MainWindow()
      w.open_file(S.write_line(deep, draft=3000.0, draft_step=10.0, skip_mrz=(S.PING0 + 2,))[1])
      self.assertAlmostEqual(w.dmin.value(), 3000.0 - 0.1 * w.line.wc.range_max_m, delta=0.01)
      w.slider.setValue(2)
      self.assertIn("Ping %d swath 0" % (S.PING0 + 2), w.meta_label.text())
      self.assertIn("draft interpolated", w.ax.get_ylabel())
      self.assertTrue(np.isfinite(w._last_grid["amp"]).any())
      # the first sample of the even beams (start sample 0) is at the transducer
      self.assertAlmostEqual(float(np.nanmin(w._last_grid["depth"])), 3020.0, delta=0.01)
      # the two-line depth label fits the plot height, also after the window is resized
      label = w.ax.yaxis.label
      w.show()
      for size, fontsize in (((1200, 400), None), ((1200, 900), label.get_fontsize())):
        w.resize(*size)
        app.processEvents()
        renderer = w.canvas.get_renderer()
        self.assertLessEqual(label.get_window_extent(renderer).height, w.ax.get_window_extent(renderer).height + 0.5)
        if fontsize is not None:
          self.assertEqual(label.get_fontsize(), fontsize)
      w.close()
      # a ship: the window starts at the surface
      ship = os.path.join(tmp, "ship")
      os.mkdir(ship)
      w = wcd_viewer.MainWindow()
      w.open_file(S.write_line(ship, draft=300.0)[1])
      self.assertEqual(w.dmin.value(), 0.0)
      w.close()
    del app
    self.assertEqual(errors, [], "".join(str(e[1]) for e in errors))


if __name__ == "__main__":
  unittest.main()
