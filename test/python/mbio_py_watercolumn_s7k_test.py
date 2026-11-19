#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_watercolumn_s7k_test.py  19/11/2026
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
"""Water column of Reson 7k data (format 89, Compressed Water Column record 7042) through
MBIO (mb_extract_wc) and mbio_py, on a synthetic *.s7k file."""

import math
import os
import sys
import tempfile
import unittest

import numpy as np

import mbio_py as mb
from mbio_py import constants as K

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_s7k as S  # noqa: E402


class WaterColumnS7kTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.tmp = tempfile.TemporaryDirectory()
    cls.s7k = S.write(cls.tmp.name)
    with mb.Reader(cls.s7k) as r:
      cls.records = list(r)
    cls.pings = [rec for rec in cls.records if rec.kind == K.MB_DATA_DATA]

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def test_format_from_name(self):
    self.assertEqual(mb.get_format(self.s7k), K.MBF_RESON7K3)

  def test_records(self):
    """Every ping is read with its soundings; the two broken 7042 records are rejected
    with a non-fatal error and only their ping loses the water column."""
    self.assertEqual([rec.ping_number for rec in self.pings], list(range(S.PING0, S.PING0 + S.NPINGS)))
    self.assertTrue(all(rec.ok for rec in self.pings))
    errors = [rec.error for rec in self.records if not rec.ok]
    self.assertEqual(errors, [K.MB_ERROR_UNINTELLIGIBLE] * len(S.BAD_RECORDS))
    for p, rec in enumerate(self.pings):
      self.assertEqual(rec.water_column is None, p in S.NO_WC, S.PINGS[p]["name"])
      self.assertEqual(len(rec.bath), S.NBEAMS + (1 if p == 0 else 0))

  def test_water_column_values(self):
    for p, rec in enumerate(self.pings):
      if p in S.NO_WC:
        continue
      spec = S.PINGS[p]
      wc = rec.water_column
      with self.subTest(ping=spec["name"]):
        self.assertEqual(wc.ping_number, S.PING0 + p)
        self.assertAlmostEqual(wc.time_d, S.ping_time(p), places=5)
        self.assertEqual(wc.nbeams, S.NBEAMS)
        rate = S.FS / S.DOWNSAMPLING if spec["name"] == "downsampled" else S.FS
        roll = float(np.float32(math.radians(S.ROLL)))
        steer = float(np.float32(S.TX_STEER)) if p == S.STEER_PING else 0.0
        for b in range(S.NBEAMS):
          self.assertEqual(wc.swath[b], 0)
          # 7k angles relative to the sonar, turned with the roll whatever the compensation flag says
          angle = float(np.float32(math.radians(S.beam_angle(b))))
          self.assertAlmostEqual(wc.angle_across[b], math.degrees(angle - roll), places=5)
          self.assertAlmostEqual(wc.angle_along[b], math.degrees(steer), places=5)
          self.assertAlmostEqual(wc.sample_interval[b], 1.0 / rate, places=12)
          self.assertEqual(wc.soundspeed[b], S.SOUNDSPEED)
          self.assertEqual(wc.start_sample[b], spec["first"])
          n = S.nsamples(p, b)
          self.assertEqual(wc.nsamples[b], n)
          # the primary detection, converted to samples of the water column
          self.assertAlmostEqual(wc.detect_sample[b], float(np.float32(S.detect_samples(b))) * rate / S.FS,
                                 places=4)
          expected = np.array([S.amplitude_db(p, b, k) for k in range(n)], np.float32)
          if spec["flags"] & 0x0004:  # 8 bit dB values: exact
            np.testing.assert_array_equal(wc.amplitude[b, :n], expected)
          else:
            np.testing.assert_allclose(wc.amplitude[b, :n], expected, rtol=1.0e-6)
          self.assertTrue(np.all(np.isnan(wc.amplitude[b, n:])))

  def test_water_column_file(self):
    wcf = mb.WaterColumnFile(self.s7k)
    self.assertEqual(len(wcf), S.NPINGS - len(S.NO_WC))
    self.assertIsNone(wcf.fill_value)  # the lowest level (magnitude 0 or 1) is valid data
    self.assertEqual(wcf.amp_min, 0.0)
    np.testing.assert_array_equal(wcf.ping_numbers, [S.PING0 + p for p in range(S.NPINGS) if p not in S.NO_WC])

  def test_line(self):
    """A *.s7k is both the survey file and the water column file of its line."""
    line = mb.KongsbergLine(self.s7k)
    self.assertEqual((line.kmall, line.kmwcd), (self.s7k, self.s7k))
    self.assertEqual(line.matched, S.NPINGS - len(S.NO_WC))
    for i in range(len(line.wc)):
      j = line.survey_index(i)
      self.assertEqual(line.survey.ping_number[j], line.wc.ping_numbers[i])
      self.assertAlmostEqual(line.draft(i), S.SENSORDEPTH, places=3)
    t = line.wc.times[1]
    lon, lat, _, _ = line.nav_at(t)
    lat_s, lon_s = S.position(t)
    self.assertAlmostEqual(lat, lat_s, places=7)
    self.assertAlmostEqual(lon, lon_s, places=7)

  def test_soundings(self):
    s = mb.load_survey(self.s7k)
    self.assertEqual(list(s.ping_number), list(range(S.PING0, S.PING0 + S.NPINGS)))
    np.testing.assert_allclose(s.sensordepth, S.SENSORDEPTH, atol=1.0e-3)
    for p in range(S.NPINGS):
      if p == S.STEER_PING:  # transmit steering moves the soundings
        continue
      flag, bath, x, det = s.beamflag[p], s.bath[p], s.bathacrosstrack[p], s.detects[p]
      good = np.array([K.beam_ok(f) for f in flag])
      self.assertEqual(int(good.sum()), S.NBEAMS)
      np.testing.assert_allclose(bath[good], S.SENSORDEPTH + S.BOTTOM_BELOW_TX, atol=1.0e-3)
      # 7k beam angles are positive to starboard, so are MB-System acrosstrack distances
      expected_x = [S.BOTTOM_BELOW_TX * math.tan(math.radians(S.vertical_angle(b))) for b in range(S.NBEAMS)]
      np.testing.assert_allclose(x[good], expected_x, atol=1.0e-2)
      if p != 0:  # with multi-detection mb_detects gives one value per beam, not per sounding
        for b in range(S.NBEAMS):
          self.assertEqual(det[b], K.MB_DETECT_AMPLITUDE if b % 2 == 0 else K.MB_DETECT_PHASE)


if __name__ == "__main__":
  unittest.main()
