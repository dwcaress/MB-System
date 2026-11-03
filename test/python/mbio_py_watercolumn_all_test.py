#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_watercolumn_all_test.py  1/11/2026
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
"""Water column of Kongsberg *.all and *.wcd files through MBIO (formats 58 and 59,
mb_extract_wc, one record per ping) and mbio_py, on a synthetic file (synth_all.py)."""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import numpy as np

import mbio_py as mb
from mbio_py import constants as K

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_all as S  # noqa: E402

TESTDATA = os.path.join(os.environ["MB_SOURCE_DIR"], "test", "utilities", "testdata")
BUILD = os.environ["MB_BINARY_DIR"]


def wc_records(path, fmt=0):
  with mb.Reader(path, fmt) as r:
    return [rec for rec in r if rec.kind == K.MB_DATA_WATER_COLUMN]


class AllWaterColumnTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.tmp = tempfile.TemporaryDirectory()
    cls.wcd = S.write(os.path.join(cls.tmp.name, S.WCD_NAME))

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def check_ping(self, rec, i):
    wc = rec.water_column
    self.assertIsNotNone(wc, i)
    self.assertTrue(rec.ok)
    self.assertEqual(rec.ping_number, S.PINGS[i])
    self.assertEqual(tuple(wc.time_i), S.time_i(i))
    n = S.nbeams_read(i)
    gap = 2 if i == 2 else 0
    self.assertEqual(wc.nbeams, n + gap)
    ibeam = 0
    start = S.START_SAMPLE if i == 4 else 0
    done = 0
    for d in range(1, S.DATAGRAMS[i] + 1):
      if (i, d) in S.LOST:
        if d < S.DATAGRAMS[i]:   # lost inside the swath: two empty beams inside the gap
          for j in range(gap):
            self.assertEqual(wc.nsamples[ibeam + j], 0)
            self.assertEqual(wc.detect_sample[ibeam + j], 0.0)
          last, nxt = S.angle_across(d - 1, S.BEAMS_PER_DATAGRAM - 1), S.angle_across(d + 1, 0)
          self.assertAlmostEqual(wc.angle_across[ibeam], last + 2.0)  # one step of 2 deg
          self.assertAlmostEqual(wc.angle_across[ibeam + 1], nxt - 2.0)
          ibeam += gap
        continue
      for b in range(S.BEAMS_PER_DATAGRAM):
        if done == n:
          break
        self.assertEqual(wc.swath[ibeam], 0)
        self.assertAlmostEqual(wc.angle_across[ibeam], S.angle_across(d, b))  # starboard positive
        self.assertAlmostEqual(wc.angle_along[ibeam], S.tilt(S.sector(d, b)))
        self.assertAlmostEqual(wc.sample_interval[ibeam], S.sample_interval())
        self.assertAlmostEqual(wc.soundspeed[ibeam], 0.1 * S.SOUNDSPEED)
        self.assertEqual(wc.start_sample[ibeam], start)
        self.assertEqual(wc.nsamples[ibeam], S.NSAMPLES)
        self.assertEqual(wc.detect_sample[ibeam], S.detected(d, b))
        np.testing.assert_array_equal(wc.amplitude[ibeam, :S.NSAMPLES], np.array(S.amplitude(i, d, b), np.float32))
        ibeam += 1
        done += 1
    self.assertEqual(ibeam, wc.nbeams)

  def check_file(self, path, fmt=0):
    pings = wc_records(path, fmt)
    self.assertEqual(len(pings), len(S.PINGS))
    for i, rec in enumerate(pings):
      self.check_ping(rec, i)

  def test_values(self):
    self.check_file(self.wcd)

  def test_format_59(self):
    self.check_file(self.wcd, 59)

  def test_big_endian(self):
    self.check_file(S.write(os.path.join(self.tmp.name, "big.wcd"), big_endian=True))

  def test_broken_datagram(self):
    self.check_file(S.write(os.path.join(self.tmp.name, "bad.wcd"), bad_datagram=True))

  def test_other_records_between(self):
    with mb.Reader(self.wcd) as r:
      kinds = [(rec.kind, rec.ping_number) for rec in r if rec.kind in (K.MB_DATA_WATER_COLUMN, K.MB_DATA_NAV)]
    self.assertEqual(kinds[0], (K.MB_DATA_WATER_COLUMN, S.PINGS[0]))
    self.assertEqual(kinds[1][0], K.MB_DATA_NAV)  # returned while ping 1 is being assembled
    self.assertEqual(kinds[2], (K.MB_DATA_WATER_COLUMN, S.PINGS[1]))

  def test_water_column_file(self):
    wcf = mb.WaterColumnFile(self.wcd)
    self.assertEqual(len(wcf), len(S.PINGS))
    self.assertEqual(wcf.format, K.MBF_EM710RAW)
    self.assertIsNone(wcf.fill_value)  # -128 is a valid level here, however common
    self.assertEqual(wcf.amp_min, 0.5 * -128 - S.TVG_OFFSET)
    self.assertEqual(len(wcf.nav_time_d), 1)
    self.assertAlmostEqual(wcf.nav_lat[0], S.LAT)
    self.assertAlmostEqual(wcf.nav_lon[0], S.LON)

  def test_line_pairing(self):
    with tempfile.TemporaryDirectory() as tmp:
      wcd = S.write(os.path.join(tmp, S.WCD_NAME))
      shutil.copy(os.path.join(TESTDATA, S.SURVEY_SAMPLE), tmp)
      for path in (wcd, os.path.join(tmp, S.ALL_NAME)):
        line = mb.KongsbergLine(path)
        self.assertTrue(line.kmall.endswith(".all"))
        self.assertTrue(line.kmwcd.endswith(".wcd"))
        self.assertEqual(len(line.survey), 2)
        self.assertEqual(len(line.wc), len(S.PINGS))
        self.assertEqual(line.matched, 2)
        self.assertEqual(line.survey_index(1), 1)
        self.assertIsNone(line.survey_index(2))

  def test_all_holding_the_water_column(self):
    with tempfile.TemporaryDirectory() as tmp:
      path = S.write(os.path.join(tmp, "0001_20111219_210356_x.all"))
      line = mb.KongsbergLine(path)
      self.assertEqual(line.kmall, path)
      self.assertEqual(line.kmwcd, path)
      self.assertEqual(len(line.wc), len(S.PINGS))

  def test_all_without_water_column(self):
    with tempfile.TemporaryDirectory() as tmp:
      shutil.copy(os.path.join(TESTDATA, S.SURVEY_SAMPLE), tmp)
      line = mb.KongsbergLine(os.path.join(tmp, S.ALL_NAME))
      self.assertIsNone(line.wc)
      self.assertEqual(len(line.survey), 2)

  def test_round_trip(self):
    """mbpreprocess writes format 59 with the original datagrams, correct sizes included;
    format 59 reads the same pings back."""
    with tempfile.TemporaryDirectory() as tmp:
      path = S.write(os.path.join(tmp, "synth.all"), bad_datagram=False)
      subprocess.run([os.path.join(BUILD, "src", "utilities", "mbpreprocess"), "--input=" + path, "--format=58"], cwd=tmp,
                     check=True, capture_output=True)
      out = os.path.join(tmp, "synth.mb59")
      written = [record for kind, record in S.walk(out) if kind == 0x6B]
      # the source without the ping cut at the end of the file, whose last datagram gets shorter
      source = [record for kind, record in S.walk_until_cut(path) if kind == 0x6B]
      self.assertEqual(written[:len(source)], source)
      self.assertEqual(len(written), len(source) + 1)
      self.check_file(out)


if __name__ == "__main__":
  unittest.main()
