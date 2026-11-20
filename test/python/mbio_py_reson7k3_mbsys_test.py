#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_reson7k3_mbsys_test.py  20/11/2026
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
"""The Reson 7k format 89 data structure module (mbsys_reson7k3.c) on rewritten copies of the
synthetic *.s7k lines of synth_s7k.py: the multi-detection priority of 7047 Segmented Raw
Detection records in mb_detects, and mbpreprocess leaving out pings without attitude."""

import os
import struct
import subprocess
import sys
import tempfile
import unittest

import mbio_py as mb
from mbio_py import constants as K

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_s7k as S  # noqa: E402

SEGMENTS = 64 + 36        # start of the 7047 segment descriptors (record frame, record type header)
SEGMENT_FLAGS = 55        # offset of the flags in a 68 byte segment descriptor
DETECTIONS = SEGMENTS + 68  # start of the 32 byte detections of the one segment of synth_s7k
DETECTION_FLAGS = 12      # offset of the flags in a detection


def records(path):
  """The 7k records of a file, in order, as bytearrays."""
  with open(path, "rb") as f:
    data = f.read()
  out, k = [], 0
  while k + 64 <= len(data):
    size = struct.unpack_from("<I", data, k + 8)[0]
    out.append(bytearray(data[k:k + size]))
    k += size
  return out


def rtype(record):
  return struct.unpack_from("<I", record, 32)[0]


def checksum(record):
  struct.pack_into("<I", record, len(record) - 4, sum(record[:-4]) & 0xFFFFFFFF)
  return record


def write_records(path, recs):
  with open(path, "wb") as f:
    for record in recs:
      f.write(record)
  return path


class Reson7k3MbsysTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.tmp = tempfile.TemporaryDirectory()
    cls.utilities = os.path.join(os.environ["MB_BINARY_DIR"], "src", "utilities")

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def test_detects_7047_priority(self):
    """The priority of a 7047 detection (flags bits 9-12, used when multi-detection is
    enabled for its segment, segment flags bit 4) is returned in bits 8-11 of its
    detection type, as for 7027 records (it used to be taken from the last 7027 record)."""
    recs = records(S.write_cases(self.tmp.name))
    segmented = [i for i, record in enumerate(recs) if rtype(record) == 7047]
    self.assertEqual(len(segmented), 1)
    record = recs[segmented[0]]
    flags = struct.unpack_from("<I", record, SEGMENTS + SEGMENT_FLAGS)[0]
    struct.pack_into("<I", record, SEGMENTS + SEGMENT_FLAGS, flags | 0x10)
    beam = 3
    at = DETECTIONS + 32 * beam + DETECTION_FLAGS
    struct.pack_into("<I", record, at, struct.unpack_from("<I", record, at)[0] | (2 << 9))
    checksum(record)
    path = write_records(os.path.join(self.tmp.name, "priority.s7k"), recs)
    ping = S.PING0 + S.CASE0 + [i for i, case in enumerate(S.CASES) if case.get("s7047")][0]
    with mb.Reader(path, format=89) as r:
      got = [rec for rec in r if rec.kind == K.MB_DATA_DATA and rec.ping_number == ping]
    self.assertEqual(len(got), 1)
    det = list(got[0].detects)
    self.assertEqual(len(det), S.NBEAMS)
    self.assertEqual([d >> 8 for d in det], [2 if b == beam else 0 for b in range(S.NBEAMS)])

  def test_missing_attitude(self):
    """Without any attitude records mbpreprocess leaves the pings out and says so; they used
    to be written with zero roll and pitch because the bathymetry calculation reset the
    error that marks them. Reading the file still returns every ping."""
    directory = os.path.join(self.tmp.name, "noattitude")
    os.mkdir(directory)
    recs = [record for record in records(S.write(directory)) if rtype(record) != 1012]
    path = write_records(os.path.join(directory, "noattitude.s7k"), recs)
    with mb.Reader(path, format=89) as r:
      numbers = [rec.ping_number for rec in r if rec.kind == K.MB_DATA_DATA and rec.ok]
    self.assertEqual(numbers, [S.PING0 + p for p in range(S.NPINGS)])
    run = subprocess.run([os.path.join(self.utilities, "mbpreprocess"), "--input=" + path, "--format=89",
                          "--nav-async=29", "--sensordepth-async=59", "--heading-async=17",
                          "--attitude-async=56"], cwd=directory, capture_output=True, text=True, check=True)
    self.assertIn("survey records of %s not written" % path, run.stderr)
    output = os.path.join(directory, "noattitude.mb89")
    with mb.Reader(output, format=89) as r:
      written = [rec for rec in r if rec.kind == K.MB_DATA_DATA]
    self.assertEqual(written, [])


if __name__ == "__main__":
  unittest.main()
