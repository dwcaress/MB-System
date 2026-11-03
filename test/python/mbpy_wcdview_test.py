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
and matplotlib installed, an offscreen session on a synthetic *.kmall/*.kmwcd pair."""

import importlib.util
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_kmall as S  # noqa: E402

SOURCE = os.environ["MB_SOURCE_DIR"]
LAUNCHER = os.path.join(SOURCE, "src", "python", "mbpy_wcdview")
HAVE_GUI = all(importlib.util.find_spec(m) is not None for m in ("PySide6", "matplotlib"))


class MbpyWcdviewTest(unittest.TestCase):

  def test_usage(self):
    result = subprocess.run([sys.executable, LAUNCHER, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("usage: mbpy_wcdview", result.stderr)

  @unittest.skipUnless(HAVE_GUI, "PySide6 and matplotlib not installed")
  def test_offscreen_session(self):
    os.environ["QT_QPA_PLATFORM"] = "offscreen"
    sys.path.insert(0, os.path.join(SOURCE, "src", "python", "wcd_viewer"))
    import wcd_viewer
    from PySide6.QtWidgets import QApplication
    app = QApplication.instance() or QApplication([])
    with tempfile.TemporaryDirectory() as tmp:
      kmall, kmwcd = S.write_line(tmp)
      w = wcd_viewer.MainWindow()
      w.open_file(kmwcd)
      self.assertEqual(len(w.steps), 2 * (S.NPINGS - 1) - 1)  # the last ping has one swath
      self.assertEqual(w.line.matched, S.NPINGS - 1)
      self.assertTrue(w.chk_raytrace.isChecked())
      w.chk_bottom.setChecked(True)
      w.chk_wcbottom.setChecked(True)
      w.slider.setValue(2)
      self.assertIn("Ping %d swath 0" % (S.PING0 + 2), w.meta_label.text())
      self.assertIn("kmall soundings", w.statusBar().currentMessage())
      row = w.pick(0.0, S.DRAFT + 0.5 * S.BOTTOM_BELOW_TX)
      self.assertIsNotNone(row)
      self.assertEqual(row[-1], "Heading")
      image = os.path.join(tmp, "ping.png")
      w.save_image(image)
      self.assertTrue(os.path.getsize(image) > 0)
      w.close()
    del app


if __name__ == "__main__":
  unittest.main()
