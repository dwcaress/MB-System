#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbnavedit_qt_test.py  9/11/2026
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
"""mbnavedit-qt without a window: usage, --dump and editing sessions run with --replay on
a synthetic *.kmall file. Run only when built with -DbuildQt6Tools=ON."""

import os
import re
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "python"))
import synth_kmall as S  # noqa: E402


class MbnaveditQtTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBNAVEDIT_QT"]

  def setUp(self):
    self.tmp = tempfile.TemporaryDirectory()
    self.kmall = S.write_line(self.tmp.name)[0]

  def tearDown(self):
    self.tmp.cleanup()

  def replay(self, *lines):
    script = os.path.join(self.tmp.name, "script.txt")
    with open(script, "w") as f:
      f.write("\n".join(lines) + "\n")
    return subprocess.run([self.program, "--replay=" + script], capture_output=True, text=True, errors="replace", cwd=self.tmp.name, timeout=120)

  def out(self, *lines):
    result = self.replay(*lines)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
    return result.stdout

  def opened(self, previous=0, mode="edit"):
    return "open %s 261 %d %s" % (self.kmall, previous, mode)

  def files(self):
    return sorted(n for n in os.listdir(self.tmp.name) if n != "script.txt" and not n.endswith(".kmwcd"))

  def read(self, suffix):
    with open(self.kmall + suffix, "rb") as f:
      return f.read()

  @staticmethod
  def hashes(out):
    return re.findall(r"data (\w+) model \w+ pixels", out)

  def records(self):
    out = self.out(self.opened(mode="browse"), "state", "done")
    return int(re.search(r"nbuff (\d+)", out).group(1))

  def test_usage(self):
    result = subprocess.run([self.program, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("Program MBNAVEDIT-QT", result.stderr)
    self.assertIn("--use-ping-data", result.stderr)

  def test_dump(self):
    out = subprocess.run([self.program, "--dump", "-I", self.kmall], capture_output=True, text=True, check=True).stdout
    self.assertIn("Format: 261", out)
    self.assertRegex(out, r"Records loaded: \d+")

  def test_saved_edit(self):
    n = self.records()
    self.assertGreaterEqual(n, 4)
    out = self.out(self.opened(), "pick lon 1", "interpolate", "state", "done")
    self.assertRegex(out, r"flagged 1 ")
    data = self.read(".nve")
    lines = data.split(b"\r\n")
    self.assertEqual(lines[-1], b"")  # CRLF after every line
    self.assertEqual(len(lines) - 1, n)
    self.assertEqual(len(lines[0].split()), 16 - 1)
    self.assertFalse(os.path.exists(self.kmall + ".nve.new"))
    self.assertFalse(os.path.exists(self.kmall + ".lck"))
    par = self.read(".par").decode()
    self.assertRegex(par, r"NAVMODE\s+1")
    self.assertRegex(par, r"NAVFORMAT\s+9")
    self.assertRegex(par, r"NAVFILE\s+\S*\.nve")

  def test_no_edits_write_nothing(self):
    before = self.files()
    self.out(self.opened(), "done")
    self.assertEqual(self.files(), before)

  def test_compatibility_writes_without_edits(self):
    self.out("compat 1", self.opened(), "done")
    self.assertGreater(len(self.read(".nve")), 0)

  def test_browse_writes_nothing(self):
    before = self.files()
    self.out(self.opened(mode="browse"), "pick lon 1", "interpolate", "done")
    self.assertEqual(self.files(), before)

  def test_discard_keeps_the_files(self):
    self.out(self.opened(), "pick lon 1", "interpolate", "done")
    nve, par = self.read(".nve"), self.read(".par")
    self.out(self.opened(1), "pick lat 2", "interpolate", "discard")
    self.assertEqual(self.read(".nve"), nve)
    self.assertEqual(self.read(".par"), par)
    self.assertFalse(os.path.exists(self.kmall + ".nve.new"))
    self.assertFalse(os.path.exists(self.kmall + ".lck"))

  def test_previous_edits_are_read(self):
    self.out(self.opened(), "pick lon 1", "interpolate", "state", "done")
    out = self.out(self.opened(1, "browse"), "state", "done")
    self.assertRegex(out, r"sources: nav")
    self.assertNotIn("dialog", out)

  def test_crash_keeps_the_nve(self):
    self.out(self.opened(), "pick lon 1", "interpolate", "done")
    nve = self.read(".nve")
    result = self.replay(self.opened(0), "pick lat 2", "interpolate", "abort")
    self.assertNotEqual(result.returncode, 0)
    self.assertEqual(self.read(".nve"), nve)

  def test_failed_open_leaves_no_lock(self):
    missing = os.path.join(self.tmp.name, "missing.kmall")
    out = self.out("open %s 261 0 edit" % missing)
    self.assertRegex(out, r"open -> status 0")
    self.assertFalse(os.path.exists(missing + ".lck"))
    self.assertFalse(os.path.exists(missing + ".nve.new"))

  def test_answering_no_replaces_the_old_edits(self):
    self.out(self.opened(), "offset 0.01 0.01", "done")
    nve = self.read(".nve")
    out = self.out(self.opened(0), "unsaved", "done")
    self.assertIn("unsaved: 1", out)
    self.assertNotEqual(self.read(".nve"), nve)

  def test_undo_and_redo(self):
    out = self.out(self.opened(), "unsaved", "state", "pick lon 1", "interpolate", "unsaved", "state", "undo", "unsaved", "state", "redo", "unsaved", "state", "discard")
    h = self.hashes(out)
    self.assertEqual(h[0], h[2])
    self.assertEqual(h[1], h[3])
    self.assertNotEqual(h[0], h[1])
    self.assertEqual(re.findall(r"unsaved: (\d)", out), ["0", "1", "0", "1"])
    self.assertIn("undo 'Interpolate'", out)

  def test_undo_of_everything_saves_nothing(self):
    before = self.files()
    self.out(self.opened(), "pick lon 1", "interpolate", "deselectall", "pick speed 1", "usesmg", "deselectall", "offset 0.001 0.002", "undo", "undo", "undo", "done")
    self.assertEqual(self.files(), before)

  def test_each_edit_undoes(self):
    for action, sel in (("interpolate", "pick lat 1"), ("usesmg", "pick speed 1"), ("usecmg", "pick heading 1"), ("revert", "pick draft 1"), ("flag", "pick lon 1"), ("offset 0.1 0.2", "pick lon 1")):
      out = self.out(self.opened(mode="browse"), sel, "state", action, "undo", "state", "done")
      h = self.hashes(out)
      self.assertEqual(h[0], h[1], action)

  def test_new_edit_ends_redo(self):
    out = self.out(self.opened(), "pick lon 1", "interpolate", "undo", "deselectall", "pick lat 1", "interpolate", "redo", "discard")
    self.assertRegex(out, r"(?m)^redo -> status 0")

  def test_action_with_no_change_makes_no_step(self):
    out = self.out(self.opened(), "repeats", "unsaved", "undo", "discard")
    self.assertIn("unsaved: 0", out)
    self.assertRegex(out, r"(?m)^undo -> status 0")

  def test_next_buffer_ends_undo(self):
    out = self.out("buffer 3 1", self.opened(), "pick lon 1", "interpolate", "next", "undo", "unsaved", "discard")
    self.assertRegex(out, r"(?m)^undo -> status 0")
    self.assertIn("unsaved: 1", out)

  def test_save_keeps_editing(self):
    out = self.out(self.opened(), "pick lon 1", "interpolate", "save", "unsaved", "undo", "unsaved", "redo", "unsaved", "done")
    self.assertRegex(out, r"(?m)^save -> status 1")
    self.assertEqual(re.findall(r"unsaved: (\d)", out), ["0", "1", "0"])
    self.assertFalse(os.path.exists(self.kmall + ".nve.new"))

  def test_save_needs_the_whole_file(self):
    out = self.out("buffer 2 1", self.opened(), "save", "discard")
    self.assertRegex(out, r"(?m)^save -> status 0")

  def test_heading_interpolates_across_north(self):
    fixed = self.out(self.opened(mode="browse"), "set heading 0 359", "set heading 1 300", "set heading 2 1", "plot", "pick heading 1", "interpolate", "row 1", "done")
    heading = float(re.search(r"hdg ([\d.]+)", fixed).group(1))
    self.assertTrue(heading > 355.0 or heading < 5.0, heading)
    old = self.out("compat 1", self.opened(mode="browse"), "set heading 0 359", "set heading 1 300", "set heading 2 1", "plot", "pick heading 1", "interpolate", "row 1", "done")
    self.assertGreater(abs(float(re.search(r"hdg ([\d.]+)", old).group(1)) - 180.0), -1.0)
    self.assertLess(abs(float(re.search(r"hdg ([\d.]+)", old).group(1)) - 180.0), 60.0)

  def repeated_time(self):
    self.out("compat 1", self.opened(), "done")
    rows = self.read(".nve").split(b"\r\n")
    rows[2] = b" ".join(rows[1].split()[:7] + rows[2].split()[7:])
    with open(self.kmall + ".nve", "wb") as f:
      f.write(b"\r\n".join(rows))

  def test_time_interpolation_recalculates(self):
    self.repeated_time()
    out = self.out(self.opened(1, "browse"), "row 1", "row 2", "row 3", "fixtime", "row 1", "row 2", "row 3", "state", "done")
    self.assertIn("Duplicate or reverse order time", out)
    rows = re.findall(r"row \d: t ([\d.]+) tint (-?[\d.]+) torg -?[\d.]+ lon \S+ lat \S+ spd \S+ hdg \S+ dft \S+ smg ([\d.]+)", out)
    t = [float(r[0]) for r in rows[3:]]
    self.assertAlmostEqual(float(rows[4][1]), t[1] - t[0], places=3)
    self.assertAlmostEqual(float(rows[5][1]), t[2] - t[1], places=3)
    self.assertGreater(float(rows[4][2]), 0.0)  # was zero with the repeated time
    self.assertRegex(out, r"tsproblem 0")

  def test_delete_bad_times_and_undo(self):
    self.repeated_time()
    out = self.out(self.opened(1, "browse"), "state", "deletebad", "state", "undo", "state", "redo", "state", "done")
    counts = re.findall(r"nbuff (\d+) ", out)
    self.assertEqual(int(counts[1]), int(counts[0]) - 1)
    self.assertEqual(counts[2], counts[0])
    self.assertEqual(counts[3], counts[1])
    h = self.hashes(out)
    self.assertEqual(h[0], h[2])
    self.assertEqual(re.findall(r"tsproblem (\d)", out), ["1", "0", "1", "0"])


if __name__ == "__main__":
  unittest.main()
