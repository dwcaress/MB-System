#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_reader_test.py  1/11/2026
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
"""mbio_py.Reader against mblist of the same build: ping times, depths and acrosstrack
distances of every beam of sample files in several formats."""

import os
import subprocess
import unittest

import numpy as np

import mbio_py as mb
from mbio_py import constants as K

# sample file (test/utilities/testdata) and format
SAMPLES = [
    ("mb11/TN136HS.309.snipped.mb11", 11),
    ("mb21/TN136HS.309.snipped.mb21", 21),
    ("mb24/TN136HS.309.snipped.mb24", 24),
    ("mb42/TN136HS.309.snipped.mb42", 42),
    ("mb51/tahoe_98_134.0.em1000.mb51", 51),
    ("mb54/TN136HS.309.snipped.mb54", 54),
    ("mb56/nbp1403-filtered-em120.mb56", 56),
    ("mb58/0008_20111219_210356_METEOR_EM122.snipped.all", 58),
    ("mb71/TN136HS.309.snipped.mb71", 71),
    ("mb171/TN136HS.309.snipped.mb171", 171),
    ("mb173/NBP0209.snipped.a77", 173),
]


class ReaderTest(unittest.TestCase):

  def setUp(self):
    self.testdata = os.path.join(os.environ["MB_SOURCE_DIR"], "test", "utilities", "testdata")
    self.mblist = os.path.join(os.environ["MB_BINARY_DIR"], "src", "utilities", "mblist")

  def check(self, sample, fmt):
    path = os.path.join(self.testdata, sample)
    out = subprocess.run([self.mblist, "-F%d" % fmt, "-I" + path, "-MA", "-Q", "-OMzD"], capture_output=True,
                         text=True, check=True).stdout
    expected = np.array([[float(v) for v in line.split()] for line in out.splitlines() if line.strip()])
    t, z, x = [], [], []
    with mb.Reader(path, fmt) as r:
      for rec in r:
        if rec.kind == K.MB_DATA_DATA and rec.ok:
          t += [rec.time_d] * len(rec.bath)
          z += list(rec.bath)
          x += list(rec.bathacrosstrack)
    self.assertGreater(len(t), 0, sample)
    self.assertEqual(len(t), len(expected), sample)
    np.testing.assert_allclose(t, expected[:, 0], atol=1.0e-6, err_msg=sample)
    np.testing.assert_allclose(z, expected[:, 1], atol=1.0e-4, err_msg=sample)  # mblist prints 4 decimals
    np.testing.assert_allclose(x, expected[:, 2], atol=1.0e-4, err_msg=sample)

  def test_samples(self):
    for sample, fmt in SAMPLES:
      with self.subTest(sample=sample):
        self.check(sample, fmt)

  def test_format_from_name(self):
    self.assertEqual(mb.get_format("0080_20250618_223434.kmwcd"), K.MBF_KEMKMALL)
    self.assertEqual(mb.get_format("0080_20250618_223434.kmall"), K.MBF_KEMKMALL)
    self.assertEqual(mb.get_format("TN136HS.309.snipped.mb21"), 21)


if __name__ == "__main__":
  unittest.main()
