#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbgrdviz_qt_test.py  7/11/2026
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
"""mbgrdviz-qt from the command line: the usage text and a bad option. The
program parses its options before it opens a window, so no display is needed.
Run only when built with -DbuildQt6Tools=ON."""

import os
import subprocess
import unittest


class MbgrdvizQtTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBGRDVIZ_QT"]

  def test_usage(self):
    result = subprocess.run([self.program, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("Program mbgrdviz-qt", result.stderr)
    self.assertIn("interactive 2D/3Dvizualization of GMT grids", result.stderr)
    for option in ("--grid-file", "--help", "--overlay-file", "--test", "--verbose"):
      self.assertIn(option, result.stderr)

  def test_bad_option(self):
    result = subprocess.run([self.program, "--no-such-option"], capture_output=True, text=True)
    self.assertNotEqual(result.returncode, 0)
    self.assertIn("usage:", result.stderr)
    self.assertIn("Program <mbgrdviz-qt> Terminated", result.stderr)


if __name__ == "__main__":
  unittest.main()
