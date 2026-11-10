#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbeditviz_qt_test.py  8/11/2026
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
"""mbeditviz-qt from the command line: the usage text. Run only when built with
-DbuildQt6Tools=ON."""

import os
import subprocess
import unittest


class MbeditvizQtTest(unittest.TestCase):

  def test_usage(self):
    result = subprocess.run([os.environ["MBEDITVIZ_QT"], "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("Program mbeditviz-qt", result.stderr)
    for option in ("--delete-input-file", "--format", "--help", "--input", "--simple-mean-grid", "--verbose"):
      self.assertIn(option, result.stderr)


if __name__ == "__main__":
  unittest.main()
