#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_watercolumn_test.py  1/11/2026
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
"""Water column and survey line reading through MBIO (format 261 water column only
files, mb_extract_wc) and mbio_py, on a synthetic *.kmall/*.kmwcd pair."""

import math
import os
import sys
import tempfile
import unittest

import numpy as np

import mbio_py as mb
from mbio_py import constants as K

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_kmall as S  # noqa: E402


class WaterColumnTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.tmp = tempfile.TemporaryDirectory()
    cls.kmall, cls.kmwcd = S.write_line(cls.tmp.name)

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def records(self, path):
    with mb.Reader(path) as r:
      return list(r)

  def test_record_kinds(self):
    kinds = [rec.kind for rec in self.records(self.kmwcd)]
    self.assertEqual(kinds.count(K.MB_DATA_WATER_COLUMN), S.NPINGS - 1)
    self.assertEqual(kinds.count(K.MB_DATA_DATA), 0)
    kinds = [rec.kind for rec in self.records(self.kmall)]
    self.assertEqual(kinds.count(K.MB_DATA_DATA), S.NPINGS)
    self.assertTrue(all(rec.ok for rec in self.records(self.kmwcd)))

  def test_water_column_values(self):
    pings = [rec for rec in self.records(self.kmwcd) if rec.kind == K.MB_DATA_WATER_COLUMN]
    last = S.PING0 + S.NPINGS - 1
    for rec in pings:
      wc = rec.water_column
      ping = rec.ping_number
      self.assertEqual(wc.ping_number, ping)
      self.assertAlmostEqual(wc.time_d, S.ping_time(ping), places=5)  # time of fan 0
      fans = (0,) if ping == last else (0, 1)  # the last ping lost fan 1
      self.assertEqual(wc.nbeams, S.NBEAMS * len(fans))
      for i in range(wc.nbeams):
        fan, b = divmod(i, S.NBEAMS)
        self.assertEqual(wc.swath[i], fan)
        self.assertAlmostEqual(wc.angle_across[i], -S.beam_angle(b), places=4)  # starboard positive
        self.assertAlmostEqual(wc.angle_along[i], 1.0 + fan, places=5)
        self.assertAlmostEqual(wc.sample_interval[i], 1.0 / S.FS, places=9)
        self.assertAlmostEqual(wc.soundspeed[i], S.SOUNDSPEED, places=3)
        self.assertEqual(wc.start_sample[i], S.START_SAMPLE if b % 2 else 0)
        self.assertEqual(wc.nsamples[i], S.NSAMPLES)
        self.assertAlmostEqual(wc.detect_sample[i], S.detect_samples(b), places=3)
        expected = np.array([0.5 * S.ampl_raw(ping, fan, b, k) - S.TVG_OFFSET for k in range(S.NSAMPLES)],
                            np.float32)
        np.testing.assert_array_equal(wc.amplitude[i, :S.NSAMPLES], expected)

  def test_water_column_file(self):
    wcf = mb.WaterColumnFile(self.kmwcd)
    self.assertEqual(len(wcf), S.NPINGS - 1)
    self.assertEqual(wcf.fill_value, 0.5 * -128 - S.TVG_OFFSET)
    self.assertEqual(wcf.amp_min, 0.5 * S.RARE_RAW - S.TVG_OFFSET)
    self.assertEqual(wcf.amp_max, 0.5 * 40 - S.TVG_OFFSET)
    self.assertEqual(len(wcf.svps), 1)
    self.assertGreater(len(wcf.nav_time_d), 10)
    with mb.Reader(self.kmwcd) as r:
      first = next(rec.water_column for rec in r if rec.water_column is not None)
    np.testing.assert_array_equal(wcf.ping(0).amplitude, first.amplitude)

  def test_line_pairing_and_navigation(self):
    line = mb.KongsbergLine(self.kmwcd)  # names 1 s apart
    self.assertTrue(line.kmall.endswith(S.KMALL_NAME))
    self.assertEqual(line.matched, S.NPINGS - 1)
    for i in range(len(line.wc)):
      j = line.survey_index(i)
      self.assertEqual(line.survey.ping_number[j], line.wc.ping_numbers[i])
      self.assertEqual(line.draft(i), S.DRAFT)
    t = line.wc.times[1] + 0.25
    lon, lat, sog_kn, cog = line.nav_at(t)
    lat_s, lon_s = S.position(t)
    self.assertAlmostEqual(lat, lat_s, places=9)
    self.assertAlmostEqual(lon, lon_s, places=9)
    north = 1.0e-5 * 111320.0
    east = north * math.cos(math.radians(lat))
    self.assertAlmostEqual(sog_kn, math.hypot(north, east) * 1.9438445, places=3)
    self.assertAlmostEqual(cog, math.degrees(math.atan2(east, north)), places=3)
    self.assertAlmostEqual(line.heading_at(t), S.HEADING, places=4)
    depth, velocity = line.svp(t)
    self.assertTrue(np.all(np.asarray(velocity) == S.SOUNDSPEED))

  def test_soundings(self):
    s = mb.load_survey(self.kmall)
    self.assertEqual(list(s.ping_number), list(range(S.PING0, S.PING0 + S.NPINGS)))
    flag, bath, det = s.beamflag[0], s.bath[0], s.detects[0]
    self.assertEqual(len(bath), 2 * S.NBEAMS)
    self.assertTrue(K.beam_null(flag[S.REJECTED]))
    self.assertTrue(K.beam_flagged_sonar(flag[S.EXTRA]))
    good = np.array([K.beam_ok(f) for f in flag])
    self.assertEqual(int(good.sum()), 2 * S.NBEAMS - 2)
    np.testing.assert_allclose(bath[good], S.DRAFT + S.BOTTOM_BELOW_TX, atol=1.0e-3)
    for b in range(S.NBEAMS):
      self.assertEqual(det[b], K.MB_DETECT_AMPLITUDE if b % 2 == 0 else K.MB_DETECT_PHASE)

  def test_raytrace_constant_profile(self):
    line = mb.KongsbergLine(self.kmwcd)
    p = line.wc.ping(0).split_swaths()[0]
    depth, velocity = line.svp(p.time_d)
    cells, det = mb.raytrace(p, depth, velocity, S.DRAFT)
    x, z, _ = p.xyz()
    valid = np.isfinite(p.amplitude)
    np.testing.assert_allclose(cells[0][valid], x[valid], atol=0.01)
    np.testing.assert_allclose(cells[1][valid], z[valid] + S.DRAFT, atol=0.01)
    np.testing.assert_allclose(det[1], S.DRAFT + S.BOTTOM_BELOW_TX * np.cos(np.radians(p.angle_along)), atol=0.01)
    np.testing.assert_allclose(det[0], -S.BOTTOM_BELOW_TX * np.tan(np.radians([S.beam_angle(b) for b in
                                                                             range(S.NBEAMS)])) *
                               np.cos(np.radians(p.angle_along)), atol=0.01)

  def test_raytrace_two_layers(self):
    """Snell's law across a step from 1500 to 1550 m/s at 100 m depth."""
    c1, c2, h, theta1 = 1500.0, 1550.0, 100.0, 30.0
    n = 400
    p = mb.WaterColumnPing(
        time_d=0.0, time_i=(0,) * 7, swath=np.zeros(1, int), angle_across=np.array([theta1]),
        angle_along=np.zeros(1), sample_interval=np.array([0.001]), soundspeed=np.array([c1]),
        start_sample=np.zeros(1, int), nsamples=np.array([n]), detect_sample=np.zeros(1),
        amplitude=np.zeros((1, n), np.float32))
    (x, z, _), _ = mb.raytrace(p, [0.0, h, h + 1.0e-6, 2000.0], [c1, c1, c2, c2], 0.0, dz=0.1)
    s1 = math.sin(math.radians(theta1))
    s2 = s1 * c2 / c1
    t1 = h / (c1 * math.sqrt(1 - s1 * s1))
    for k in range(0, n, 20):
      t = 0.5 * k * 0.001
      if t <= t1:
        zx = (c1 * t * math.sqrt(1 - s1 * s1), c1 * t * s1)
      else:
        zx = (h + c2 * (t - t1) * math.sqrt(1 - s2 * s2), h * s1 / math.sqrt(1 - s1 * s1) + c2 * (t - t1) * s2)
      self.assertAlmostEqual(z[0, k], zx[0], delta=0.2)
      self.assertAlmostEqual(x[0, k], zx[1], delta=0.2)


if __name__ == "__main__":
  unittest.main()
