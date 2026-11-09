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

  def test_canvas(self):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([os.environ["MBNAVEDIT_QT_CANVAS_TEST"], self.kmall], capture_output=True, text=True, errors="replace", env=env)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

  def test_window(self):
    first = os.path.join(self.tmp.name, "a")
    second = os.path.join(self.tmp.name, "b")
    os.makedirs(first)
    os.makedirs(second)
    files = [S.write_line(first)[0], S.write_line(second)[0]]
    datalist = os.path.join(self.tmp.name, "list.mb-1")
    with open(datalist, "w") as f:
      f.write("".join("%s 261\n" % name for name in files))
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([os.environ["MBNAVEDIT_QT_WINDOW_TEST"], files[0], files[1], datalist], capture_output=True, text=True, errors="replace", env=env, timeout=120)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

  def test_export(self):
    image = os.path.join(self.tmp.name, "plots.png")
    subprocess.run([self.program, "--export=" + image, "--size=800x900", "-I", self.kmall], capture_output=True, check=True, env=dict(os.environ, QT_QPA_PLATFORM="offscreen"))
    with open(image, "rb") as f:
      self.assertEqual(f.read(8), b"\x89PNG\r\n\x1a\n")
    self.assertGreater(os.path.getsize(image), 1000)

  def test_draw_counts(self):
    n = self.records()
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    script = os.path.join(self.tmp.name, "script.txt")
    with open(script, "w") as f:
      f.write(self.opened(mode="browse") + "\ndrawcount\npick lon 1\ndrawcount\ndone\n")
    out = subprocess.run([self.program, "--replay=" + script], capture_output=True, text=True, errors="replace", cwd=self.tmp.name, check=True, env=env).stdout
    counts = re.findall(r"drawcount: fill_black (\d+) rect_red (\d+)", out)
    self.assertEqual(len(counts), 2)
    self.assertEqual(int(counts[0][1]), 0)
    self.assertEqual(int(counts[1][1]), 1)
    self.assertEqual(int(counts[0][0]), int(counts[1][0]) + 1)
    self.assertEqual(int(counts[0][0]) % n, 0)

  def attitude(self, out, index):
    m = re.search(r"attitude %d: roll (\S+) pitch (\S+) heave (\S+) draft (\S+) flag (\d) selected (\d+)" % index, out)
    self.assertIsNotNone(m, out)
    return float(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4)), int(m.group(5)), int(m.group(6))

  def test_roll_and_pitch_edits(self):
    base = self.attitude(self.out(self.opened(mode="browse"), "attitude 1", "attitude 3", "done"), 1)
    show = ["show vru 1", "plot"]
    out = self.out(self.opened(), *show, "set roll 2 40", "set pitch 2 -40", "plot", "select roll 2 2", "interpolate", "attitude 2", "undo", "attitude 2", "redo",
                   "deselectall", "select pitch 2 2", "interpolate", "attitude 2", "done")
    after_roll = re.findall(r"attitude 2: roll (\S+) pitch (\S+)", out)
    self.assertEqual(len(after_roll), 3)
    self.assertLess(abs(float(after_roll[0][0])), 30)  # the spike in roll is gone
    self.assertEqual(float(after_roll[0][1]), -40.0)  # pitch was not selected
    self.assertEqual(float(after_roll[1][0]), 40.0)  # undo
    self.assertLess(abs(float(after_roll[2][1])), 30)  # the spike in pitch is gone
    self.assertLess(abs(float(after_roll[2][0])), 30)  # redo kept the first edit
    fields = self.read(".nve").split(b"\r\n")[2].split()
    self.assertEqual(float(fields[12]), float(after_roll[2][0]))
    self.assertEqual(float(fields[13]), float(after_roll[2][1]))
    self.assertTrue(base)

  def test_roll_revert(self):
    original = self.attitude(self.out(self.opened(mode="browse"), "attitude 2", "done"), 2)[0]
    out = self.out(self.opened(mode="browse"), "show vru 1", "plot", "set roll 2 40", "plot", "select roll 2 2", "revert", "attitude 2", "done")
    self.assertAlmostEqual(self.attitude(out, 2)[0], original, 3)

  def test_heave_stays_shown_only(self):
    out = self.out(self.opened(mode="browse"), "show vru 1", "plot", "selectall heave", "state", "selectall roll", "state", "done")
    self.assertEqual(re.findall(r"selected (\d+) ", out)[0], "0")
    self.assertGreater(int(re.findall(r"selected (\d+) ", out)[1]), 0)

  def test_compat_keeps_attitude_shown_only(self):
    out = self.out("compat 1", self.opened(mode="browse"), "show vru 1", "plot", "set roll 2 40", "plot", "selectall roll", "interpolate", "attitude 2", "done")
    self.assertEqual(self.attitude(out, 2)[0], 40.0)

  def test_depth_interpolation_flags_only_in_compat(self):
    lines = ["plot", "set draft 2 90", "select draft 2 2", "interpolate", "attitude 2", "done"]
    self.assertEqual(self.attitude(self.out(self.opened(mode="browse"), *lines), 2)[4], 0)
    self.assertEqual(self.attitude(self.out("compat 1", self.opened(mode="browse"), *lines), 2)[4], 1)

  def test_attitude_points_are_drawn(self):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    script = os.path.join(self.tmp.name, "script.txt")
    def counts(*before):
      with open(script, "w") as f:
        f.write("\n".join(list(before) + [self.opened(mode="browse"), "show vru 1", "plot", "drawcount", "set roll 2 40", "drawcount", "select roll 2 2", "drawcount", "done"]) + "\n")
      out = subprocess.run([self.program, "--replay=" + script], capture_output=True, text=True, errors="replace", cwd=self.tmp.name, check=True, env=env).stdout
      return [(int(a), int(b)) for a, b in re.findall(r"drawcount: fill_black \d+ rect_red (\d+) rect_purple (\d+)", out)]
    new = counts()
    self.assertEqual(new[1][1], new[0][1] + 1)  # changed
    self.assertEqual(new[2], (new[0][0] + 1, new[0][1]))  # selected
    self.assertEqual(counts("compat 1"), [(0, 0)] * 3)

  def test_line_stays_when_originals_are_hidden(self):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    script = os.path.join(self.tmp.name, "script.txt")
    def lines(*before):
      with open(script, "w") as f:
        f.write("\n".join(list(before) + [self.opened(mode="browse"), "plot", "drawcount", "show draft_org 0", "drawcount", "done"]) + "\n")
      out = subprocess.run([self.program, "--replay=" + script], capture_output=True, text=True, errors="replace", cwd=self.tmp.name, check=True, env=env).stdout
      return [int(x) for x in re.findall(r"line_green (\d+)", out)]
    new = lines()
    self.assertEqual(new[0], new[1])  # a line through the points instead
    motif = lines("compat 1")
    self.assertEqual(motif[1], motif[0] - (self.records() - 1))  # none in the Motif program

  def test_despike(self):
    window = "1000000"
    n = self.records()
    out = self.out(self.opened(), "set draft 2 90", "plot", "select lon 0 0", "despike %s 5" % window, "attitude 2", "state", "interpolate", "attitude 2", "state", "despike %s 5" % window, "undo", "attitude 2", "done")
    self.assertRegex(out, r"despike: 1\b")
    self.assertRegex(out, r"despike: 0\b")
    rows = re.findall(r"attitude 2: .*draft (\S+) flag (\d) selected (\d+)", out)
    self.assertEqual(float(rows[0][0]), 90.0)
    self.assertEqual(int(rows[0][2]), 1 << 5)  # only the sonar depth, and the selection in longitude is gone
    self.assertLess(float(rows[1][0]), 80.0)
    self.assertEqual(int(rows[1][1]), 0)
    self.assertEqual(float(rows[2][0]), 90.0)  # one undo brings it back
    self.assertGreaterEqual(n, 4)

  def test_despike_first_and_last_record(self):
    n = self.records()
    out = self.out(self.opened(mode="browse"), "set draft 0 90", "set draft %d 91" % (n - 1), "plot", "despike 1000000 5", "interpolate", "attitude 0", "attitude %d" % (n - 1), "done")
    self.assertRegex(out, r"despike: 2 largest 8\d\.\d+")
    self.assertLess(self.attitude(out, 0)[3], 80.0)
    self.assertLess(self.attitude(out, n - 1)[3], 80.0)

  def test_despike_bad_arguments(self):
    out = self.out(self.opened(mode="browse"), "despike 0 5", "despike 10 0", "show draft 0", "plot", "despike 1000000 5", "done")
    self.assertEqual(len(re.findall(r"despike: 0\b", out)), 3)

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
