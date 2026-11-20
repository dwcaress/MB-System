#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_reson7k3_reader_test.py  20/11/2026
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
"""The Reson 7k format 89 reader (mbr_reson7k3.c) on broken and rewritten copies of the
synthetic *.s7k line of synth_s7k.py: record sizes below the frame header, beam and
detection counts beyond the arrays, a round trip through the format 89 writer (file
catalog), and a sequence of 7012 Ping Motion records printed with --debug-record-contents."""

import os
import re
import struct
import subprocess
import sys
import tempfile
import unittest

import numpy as np

import mbio_py as mb
from mbio_py import constants as K

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_s7k as S  # noqa: E402


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
  """Set the checksum of a record after its contents were changed."""
  struct.pack_into("<I", record, len(record) - 4, sum(record[:-4]) & 0xFFFFFFFF)
  return record


def write_records(path, recs):
  with open(path, "wb") as f:
    for record in recs:
      f.write(record)
  return path


def read(path):
  with mb.Reader(path, format=89) as r:
    return list(r)


def pings(recs):
  return [rec for rec in recs if rec.kind == K.MB_DATA_DATA and rec.ok]


class Reson7k3ReaderTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.tmp = tempfile.TemporaryDirectory()
    cls.s7k = S.write(cls.tmp.name)
    cls.recs = records(cls.s7k)
    cls.original = read(cls.s7k)
    cls.numbers = [rec.ping_number for rec in pings(cls.original)]
    cls.utilities = os.path.join(os.environ["MB_BINARY_DIR"], "src", "utilities")

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def path(self, name):
    return os.path.join(self.tmp.name, name)

  def nth(self, recs, kind, n):
    """Index in recs of the n-th record of type kind."""
    return [i for i, record in enumerate(recs) if rtype(record) == kind][n]

  def test_line(self):
    """The unchanged line gives every ping once."""
    self.assertEqual(self.numbers, [S.PING0 + p for p in range(S.NPINGS)])

  def test_short_size(self):
    """A record frame whose size is below the 68 bytes of frame header and checksum is
    skipped like any broken header, and reading goes on (before: the rest of the record
    was read with a wrapped length, and the line ended)."""
    recs = [bytearray(r) for r in self.recs]
    i = self.nth(recs, 7000, 1)
    fake = bytearray(recs[i][:64])
    struct.pack_into("<I", fake, 8, 10)
    recs.insert(i, fake)
    got = read(write_records(self.path("short_size.s7k"), recs))
    self.assertEqual([rec.ping_number for rec in pings(got)], self.numbers)

  def test_oversized_counts(self):
    """A 7004 record with more beams than MBIO holds (2048) and a 7027 record with more
    detections (2560) are rejected as unintelligible instead of being read past their
    arrays; the 7004 of the ping before stays in use, and the ping whose 7027 was rejected
    has no detections and is dropped."""
    recs = [bytearray(r) for r in self.recs]
    i = self.nth(recs, 7004, 2)
    struct.pack_into("<I", recs[i], 64 + 8, 100000)
    checksum(recs[i])
    j = self.nth(recs, 7027, 3)
    struct.pack_into("<I", recs[j], 64 + 14, 100000)
    checksum(recs[j])
    got = read(write_records(self.path("oversized.s7k"), recs))
    errors = [rec.error for rec in got if not rec.ok]
    errors_before = [rec.error for rec in self.original if not rec.ok]
    self.assertEqual(len(errors), len(errors_before) + 2)
    self.assertEqual(errors.count(K.MB_ERROR_UNINTELLIGIBLE),
                     errors_before.count(K.MB_ERROR_UNINTELLIGIBLE) + 2)
    numbers = [rec.ping_number for rec in pings(got)]
    self.assertEqual(numbers, [n for n in self.numbers if n != S.PING0 + 3])
    before = {rec.ping_number: rec for rec in pings(self.original)}
    ping2 = [rec for rec in pings(got) if rec.ping_number == S.PING0 + 2][0]
    np.testing.assert_array_equal(ping2.bath, before[S.PING0 + 2].bath)

  def test_catalog_round_trip(self):
    """The format 89 writer adds a 7300 file catalog and writes the last ping after the
    sensor records that follow it. The reader follows the catalog and stops at its end;
    it used to read on from there and return the last ping a second time."""
    copy = self.path("copy.mb89")
    subprocess.run([os.path.join(self.utilities, "mbcopy"), "-F89/89", "-I", self.s7k, "-O", copy],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    self.assertIn(7300, [rtype(r) for r in records(copy)])
    got = pings(read(copy))
    self.assertEqual([rec.ping_number for rec in got], self.numbers)
    self.assertEqual([rec.time_d for rec in got], [rec.time_d for rec in pings(self.original)])

  def test_ping_motion(self):
    """7012 Ping Motion records are read whole every time (the arrays used to be read only
    when they grew), and --debug-record-contents prints the record just read (it used to
    print the previous record of that type)."""
    recs = []
    for record in self.recs:
      recs.append(bytearray(record))
      if rtype(record) == 7027:
        p = struct.unpack_from("<I", record, 64 + 8)[0] - S.PING0
        n = 5
        body = struct.pack("<QIHIHIf", 0, S.PING0 + p, 0, n, 2, 0, S.FS)
        body += struct.pack("<%df" % n, *[0.001 * (p + 1) * (k + 1) for k in range(n)])
        motion = bytearray(record[:64]) + body + bytes(4)
        struct.pack_into("<I", motion, 8, len(motion))
        struct.pack_into("<I", motion, 32, 7012)
        struct.pack_into("<II", motion, 12, 0, 0)  # no optional data
        recs.append(checksum(motion))
    path = write_records(self.path("ping_motion.s7k"), recs)
    run = subprocess.run([os.path.join(self.utilities, "mbinfo"), "-F89", "-I", path,
                          "--debug-record-contents=7012"], capture_output=True, text=True, check=True)
    listings = run.stderr.split("mbsys_reson7k3_print_PingMotion")[1:]
    self.assertEqual(len(listings), S.NPINGS)
    for p, listing in enumerate(listings):
      number = int(re.search(r"ping_number:\s+(\d+)", listing).group(1))
      self.assertEqual(number, S.PING0 + p)
      rolls = [float(v) for v in re.findall(r"^\s+\d+\s+(\S+)\s+\S+\s+\S+$", listing, re.M)]
      np.testing.assert_allclose(rolls, [0.001 * (p + 1) * (k + 1) for k in range(5)], rtol=1.0e-5)


if __name__ == "__main__":
  unittest.main()
