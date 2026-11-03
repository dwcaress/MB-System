#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbvelocitytool_qt_test.py  2/11/2026
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
"""mbvelocitytool-qt without a window: usage, --dump, the --save options and --export on a synthetic
*.kmall file with a flat seafloor. Run only when built with -DbuildQt6Tools=ON."""

import os
import math
import re
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_kmall as S  # noqa: E402


def residual_table(text):
  i = text.index("Current Bathymetry Depth Range:")
  return text[i:]


def residual_rows(text):
  return [line.split() for line in residual_table(text).splitlines() if re.match(r"^ +\d+ +-?\d", line)]


class MbvelocitytoolQtTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBVELOCITYTOOL_QT"]
    cls.tmp = tempfile.TemporaryDirectory()
    cls.kmall, _ = S.write_line(cls.tmp.name)

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def run_tool(self, *args, check=True):
    return subprocess.run([self.program] + list(args), capture_output=True, text=True, check=check, cwd=self.tmp.name)

  def write_profile(self, name, rows):
    path = os.path.join(self.tmp.name, name)
    with open(path, "w") as f:
      f.write("# a comment\n")
      for depth, velocity in rows:
        f.write("%f %f\n" % (depth, velocity))
    return path

  def test_usage(self):
    result = self.run_tool("-H")
    self.assertIn("Program MBVELOCITYTOOL-QT", result.stderr)
    self.assertIn("interactive water velocity profile editor", result.stderr)
    self.assertIn("usage: mbvelocitytool-qt", result.stderr)

  def test_export_image(self):
    image = os.path.join(self.tmp.name, "plots.png")
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    subprocess.run([self.program, "-I", self.kmall, "--export=" + image, "--size=900x600"], check=True, cwd=self.tmp.name, env=env)
    with open(image, "rb") as f:
      header = f.read(24)
    self.assertEqual(header[:8], b"\x89PNG\r\n\x1a\n")
    self.assertEqual(struct.unpack(">II", header[16:24]), (900, 600))

  def test_export_of_a_missing_file_fails(self):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    image = os.path.join(self.tmp.name, "none.png")
    result = subprocess.run([self.program, "-I", self.kmall + ".none", "-F261", "--export=" + image], capture_output=True,
                            cwd=self.tmp.name, env=env)
    self.assertNotEqual(result.returncode, 0)
    self.assertFalse(os.path.exists(image))

  def test_missing_file(self):
    result = self.run_tool("--dump", "-I", self.kmall + ".none", "-F261", check=False)
    self.assertNotEqual(result.returncode, 0)

  def test_dump_default_profile(self):
    out = self.run_tool("--dump", "-I", self.kmall).stdout
    self.assertIn("Format: 261, travel times read", out)
    self.assertIn("Pings loaded: %d," % S.NPINGS, out)
    self.assertIn("Edit profile: new, 6 nodes", out)
    self.assertIn("failed 0", out)
    # the old tool wrote its Levitus profile into the current directory
    self.assertFalse(os.path.exists(os.path.join(self.tmp.name, "mbvt_levitus_tmp.svp")))
    # a flat seafloor at the sound speed of the data: the depths are on the straight line, apart
    # from the 1-2 deg along track tilt of the sectors, which is under 1 m at this depth
    rows = [row for row in residual_rows(out) if int(row[6]) > 0]
    self.assertGreater(len(rows), 0)
    for row in rows:
      self.assertEqual(int(row[6]), S.NPINGS, row)
      self.assertLess(abs(float(row[4])), 1.0, row)
      self.assertAlmostEqual(float(row[3]), S.BOTTOM_BELOW_TX, delta=5.0, msg=row)

  def test_threads_give_the_same_table(self):
    one = self.run_tool("--dump", "-I", self.kmall, "--threads=1").stdout
    many = self.run_tool("--dump", "-I", self.kmall, "--threads=3").stdout
    self.assertEqual(residual_table(one), residual_table(many))

  def test_raypaths(self):
    out = self.run_tool("--dump", "-I", self.kmall, "--raypaths").stdout
    self.assertRegex(out, r"Raypaths: xmax [0-9.]+ depth [0-9.]+ \.\. [0-9.]+")

  def test_edit_profile_is_used_for_the_first_residuals(self):
    path = self.write_profile("fast.svp", [(0, 1500), (250, 1500), (500, 1600), (12000, 1600)])
    plain = residual_rows(self.run_tool("--dump", "-I", self.kmall).stdout)
    fast = residual_rows(self.run_tool("--dump", "-I", self.kmall, "-W", path).stdout)
    self.assertNotEqual(plain, fast)

  def test_short_profile_is_extended_as_mbprocess(self):
    # the cast ends above the seafloor: the rays continue at the last velocity, not stop at the bottom of the cast
    short = self.write_profile("short.svp", [(0, 1500), (50, 1500), (100, 1500)])
    out = self.run_tool("--dump", "-I", self.kmall, "-W", short).stdout
    self.assertIn("failed 0, stopped at the bottom of the profile 0", out)
    self.assertAlmostEqual(float(residual_rows(out)[0][3]), S.BOTTOM_BELOW_TX, delta=5.0)

  def test_profile_starting_below_the_sonar_is_traced(self):
    deep = self.write_profile("deep.svp", [(100, 1500), (200, 1500), (300, 1500)])
    out = self.run_tool("--dump", "-I", self.kmall, "-W", deep).stdout
    self.assertIn("failed 0,", out)

  def test_untidy_profile(self):
    # duplicate and decreasing depths are ignored for the ray tracing, as mbprocess does
    untidy = self.write_profile("untidy.svp", [(0, 1500), (10, 1500), (10, 1500), (5, 1500), (20, 1500), (12000, 1500)])
    self.assertIn("failed 0,", self.run_tool("--dump", "-I", self.kmall, "-W", untidy).stdout)

  def test_empty_profile_is_an_error(self):
    empty = self.write_profile("empty.svp", [])
    self.assertNotEqual(self.run_tool("--dump", "-I", self.kmall, "-W", empty, check=False).returncode, 0)

  def test_bad_angle_mode(self):
    self.assertNotEqual(self.run_tool("--dump", "-I", self.kmall, "--angle-mode=7", check=False).returncode, 0)

  def test_listed_profile_is_a_display_profile(self):
    listed = self.kmall + "_000.svp"
    with open(listed, "w") as f:
      f.write("0.0 1500.0\n100.0 1490.0\n")
    try:
      out = self.run_tool("--dump", "-I", self.kmall).stdout
      self.assertIn("%s, 2 nodes" % os.path.basename(listed), out.replace(os.path.dirname(listed) + os.sep, ""))
    finally:
      os.remove(listed)

  def test_lowercase_short_options(self):
    # as in mbvelocitytool: -s is the display profile, it does not save anything
    listed = self.write_profile("shown.svp", [(0, 1500), (100, 1490)])
    saved = self.kmall + ".svp"
    if os.path.exists(saved):
      os.remove(saved)
    out = self.run_tool("--dump", "-i", self.kmall, "-s", listed, "-b", "2000/1/1/0/0/0", "-e", "2100/1/1/0/0/0").stdout
    self.assertIn("shown.svp, 2 nodes", out)
    self.assertFalse(os.path.exists(saved))

  def test_profile_stored_in_the_file(self):
    out = self.run_tool("--dump", "-I", self.kmall).stdout
    self.assertIn("Profiles stored in the file: 1", out)
    self.assertRegex(out, r"Swath SVP 1: \d{4}-\d\d-\d\d \d\d:\d\d:\d\d, 2 nodes")

  def test_repeated_records_count_once(self):
    with open(self.kmall, "rb") as f:
      data = f.read()
    other = ((0.0, 1490.0), (100.0, 1480.0), (5000.0, 1540.0))
    # records of different lengths, one repeated right after itself and one later: mbsvplist compares only part of them
    close = ((0.0, 1490.0), (100.0, 1480.0), (5000.0, 1540.5))
    records = S.svp(S.T0 - 5.0, other) + S.svp(S.T0 - 4.0, other) + S.svp(S.T0 - 3.5, close) + S.svp(S.T0 - 3.0, other)
    path = os.path.join(self.tmp.name, "repeats.kmall")
    with open(path, "wb") as f:
      f.write(records + data)
    out = self.run_tool("--dump", "-I", path).stdout
    self.assertIn("Profiles stored in the file: 3", out)  # other, close and the one of the line
    self.assertEqual(len(re.findall(r"Swath SVP \d+:", out)), 3)
    self.assertEqual(len(re.findall(r"Swath SVP \d+: .*, 3 nodes", out)), 2)

  def test_listed_copy_of_a_stored_profile_is_not_shown_twice(self):
    listed = self.kmall + "_000.svp"
    with open(listed, "w") as f:
      f.write("0.00 %.2f\n5000.00 %.2f\n" % (S.SOUNDSPEED, S.SOUNDSPEED))
    try:
      out = self.run_tool("--dump", "-I", self.kmall).stdout
      self.assertIn("Swath SVP 1:", out)
      self.assertNotIn(os.path.basename(listed), out)
    finally:
      os.remove(listed)

  def test_window(self):
    # two long stored profiles that differ, crossing near the surface, and the short one of the line
    deep = [(10.0 * i, 1500.0 + 15.0 * math.sin(i / 10.0)) for i in range(120)] + [(5000.0, 1500.0)]
    other = [(8.0 * i, 1500.0 - 15.0 * math.sin(i / 10.0)) for i in range(150)] + [(5000.0, 1500.0)]
    with open(self.kmall, "rb") as f:
      data = f.read()
    path = os.path.join(self.tmp.name, "window.kmall")
    with open(path, "wb") as f:
      f.write(S.svp(S.T0 - 5.0, deep) + S.svp(S.T0 - 4.0, other) + data)
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([os.environ["MBVELOCITYTOOL_QT_WINDOW_TEST"], path, self.kmall, self.tmp.name], capture_output=True,
                            text=True, cwd=self.tmp.name, env=env, timeout=300)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
    self.assertIn(" 0 failed", result.stdout)

  def test_save(self):
    saved = os.path.join(self.tmp.name, "edit.svp")
    self.run_tool("-I", self.kmall, "--save-edit-svp=" + saved, "--save-swath-svp", "--save-residuals", "--angle-mode=2")

    with open(saved) as f:
      text = f.read()
    self.assertIn("## Output by Program MBVELOCITYTOOL-QT", text)
    self.assertIn("## Number of SVP Points: 6", text)
    self.assertEqual(len([line for line in text.splitlines() if not line.startswith("#")]), 6)

    for suffix in (".svp", ".sbo", ".sbao"):
      self.assertTrue(os.path.exists(self.kmall + suffix), suffix)
    with open(self.kmall + ".sbo") as f:
      self.assertIn("## Number of Static Beam Offset Points:", f.read())
    with open(self.kmall + ".par") as f:
      par = f.read()
    self.assertRegex(par, r"SVPMODE\s+1")
    self.assertRegex(par, r"SVPFILE\s+\S*" + re.escape(os.path.basename(self.kmall)) + r"\.svp")
    self.assertRegex(par, r"ANGLEMODE\s+2")
    self.assertRegex(par, r"STATICMODE\s+1")
    self.assertRegex(par, r"STATICFILE\s+\S*" + re.escape(os.path.basename(self.kmall)) + r"\.sbo")


if __name__ == "__main__":
  unittest.main()
