#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbwcdview_test.py  1/11/2026
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
"""mbwcdview (C++/Qt6) without a window: usage, --dump and --export on a synthetic
*.kmall/*.kmwcd pair. Run only when built with -DbuildWCDView=ON."""

import os
import re
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_kmall as S  # noqa: E402


class MbwcdviewTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBWCDVIEW"]
    cls.tmp = tempfile.TemporaryDirectory()
    cls.kmall, cls.kmwcd = S.write_line(cls.tmp.name)

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def test_usage(self):
    result = subprocess.run([self.program, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("usage: mbwcdview", result.stderr)

  def test_dump(self):
    out = subprocess.run([self.program, "--dump", "-I", self.kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    self.assertIn(S.KMALL_NAME, out)
    self.assertIn("water column pings %d, matched %d" % (S.NPINGS - 1, S.NPINGS - 1), out)
    self.assertIn("fill value -94.0 dB, amplitude -55.0 .. -10.0 dB", out)
    self.assertIn("ping number %d" % (S.PING0 + 2), out)
    ratio = re.search(r"straight ([0-9.]+), ray traced ([0-9.]+)", out)
    self.assertIsNotNone(ratio, out)
    # flat seafloor, constant sound speed: both geometries agree and put the detections on the
    # soundings, apart from the 1-2 deg along track tilt of the sectors (cos 2 deg = 0.9994)
    self.assertAlmostEqual(float(ratio.group(1)), float(ratio.group(2)), places=4)
    self.assertAlmostEqual(float(ratio.group(1)), 1.0, delta=0.001)

  def test_export(self):
    image = os.path.join(self.tmp.name, "ping.png")
    subprocess.run([self.program, "-I", self.kmall, "--ping=1", "--bottom", "--wc-bottom", "--export=" + image,
                    "--size=800x600"], check=True)
    with open(image, "rb") as f:
      self.assertEqual(f.read(8), b"\x89PNG\r\n\x1a\n")


if __name__ == "__main__":
  unittest.main()
