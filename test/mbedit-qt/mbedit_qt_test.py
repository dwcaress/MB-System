#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbedit_qt_test.py  2/11/2026
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
"""mbedit-qt on a synthetic *.kmall file: usage, --dump, --export, a session in --replay compared with
what the Motif mbedit code writes, the saving model, and the window test program. Run only when built
with -DbuildQt6Tools=ON."""

import hashlib
import math
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_kmall as S  # noqa: E402

HEADER = 1024

# One session run through the unmodified mbedit_prog.c without X11, on this synthetic file. The flags after
# each step and the SHA-256 of the .esf and .esf.stream without their headers are what that code gave.
SESSION = """legacy 1
open line.kmall 261 0 edit
flags
toggle 600 200
flags
pick 500 300
erase 650 250
restore 650 250
erase 620 300
grab 400 100 480 200
toggle 640 350
bad
good
toggle 500 150
left
toggle 700 250
right
toggle 640 300
zero
flags
step 5
flagview
step 5
flagview
unflagview
step 3
unflagall
step -2
pings 25
view 1
toggle 500 300
view 0
pings 10
filter 1 1 5 3 1 2 1 3 4 1 100 300 1 40 -40
flags
quit
"""
EXPECTED_CRC = ['10622225487573310549', '2682625277257691130', '8342860203784730180', '651391491570750264']
EXPECTED_ESF = "3db202a3769c439febf5a0efe784e94dd92acdb5a3d942479e0a68df90d7577d"
EXPECTED_STREAM = "5ca245e244335ea6f53d863a1589b74a783ac46a74108f2ee025058f7177584c"


def records(path):
  with open(path, "rb") as f:
    body = f.read()[HEADER:]
  return [struct.unpack(">dii", body[i : i + 16]) for i in range(0, len(body), 16)]


def body_sha(path):
  with open(path, "rb") as f:
    return hashlib.sha256(f.read()[HEADER:]).hexdigest()


class MbeditQtTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBEDIT_QT"]
    cls.tmp = tempfile.TemporaryDirectory()
    S.NPINGS = 40
    cls.kmall, _ = S.write_line(cls.tmp.name)

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def folder(self, name):
    path = os.path.join(self.tmp.name, name)
    os.mkdir(path)
    shutil.copy(self.kmall, os.path.join(path, "line.kmall"))
    return path

  def replay(self, folder, script):
    with open(os.path.join(folder, "script.txt"), "w") as f:
      f.write(script)
    return subprocess.run([self.program, "--replay=script.txt"], cwd=folder, capture_output=True, text=True, check=True).stdout

  def test_usage(self):
    result = subprocess.run([self.program, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("Program mbedit-qt", result.stderr)
    for option in ("--begin-time", "--browse", "--end-time", "--format", "--gui-mode", "--input", "--run-mbprocess", "--use-edit-save", "--verbose",
                   "--dump", "--export", "--replay"):
      self.assertIn(option, result.stderr)

  def test_dump(self):
    d = self.folder("dump")
    out = subprocess.run([self.program, "--dump", "-Iline.kmall"], cwd=d, capture_output=True, text=True, check=True).stdout.splitlines()
    self.assertEqual(len(out), 41)
    fields = out[1].split()
    self.assertEqual(int(fields[3]), 2 * S.NBEAMS)
    self.assertEqual(sorted(os.listdir(d)), ["line.kmall"])

  def test_export(self):
    d = self.folder("export")
    subprocess.run([self.program, "--export=plot.png", "--size=500x300", "-Iline.kmall"], cwd=d, capture_output=True, check=True,
                   env=dict(os.environ, QT_QPA_PLATFORM="offscreen"))
    with open(os.path.join(d, "plot.png"), "rb") as f:
      self.assertEqual(f.read(8), b"\x89PNG\r\n\x1a\n")
    self.assertEqual(sorted(os.listdir(d)), ["line.kmall", "plot.png"])

  def test_session_as_mbedit(self):
    d = self.folder("session")
    out = self.replay(d, SESSION)
    self.assertEqual(re.findall(r"^flags: .* crc (\d+)", out, re.M), EXPECTED_CRC)
    self.assertEqual(body_sha(os.path.join(d, "line.kmall.esf")), EXPECTED_ESF)
    self.assertEqual(body_sha(os.path.join(d, "line.kmall.esf.stream")), EXPECTED_STREAM)

  def test_no_changes_writes_nothing(self):
    d = self.folder("nochange")
    self.replay(d, "open line.kmall 261 0 edit\nquit\n")
    self.assertEqual(sorted(os.listdir(d)), ["line.kmall", "script.txt"])

  def test_discard_keeps_everything(self):
    d = self.folder("discard")
    self.replay(d, "open line.kmall 261 0 edit\ntoggle 600 200\nquit\n")
    with open(os.path.join(d, "line.kmall.esf"), "rb") as f:
      before = f.read()
    self.assertEqual(len(records(os.path.join(d, "line.kmall.esf"))), 1)
    self.assertEqual(len(before), HEADER + 16)
    history = sorted(os.listdir(os.path.join(d, "mbhistory")))
    out = self.replay(d, "open line.kmall 261 1 edit\ntoggle 500 300\nchanges\ndiscard\n")
    self.assertIn("changes: 1 unsaved 1", out)
    with open(os.path.join(d, "line.kmall.esf"), "rb") as f:
      self.assertEqual(f.read(), before)
    self.assertEqual(sorted(os.listdir(os.path.join(d, "mbhistory"))), history)
    self.assertFalse(os.path.exists(os.path.join(d, "line.kmall.lck")))

  @unittest.skipIf(sys.platform == "win32", "the replay has no abort on Windows")
  def test_crash_and_recover(self):
    d = self.folder("crash")
    self.replay(d, "open line.kmall 261 0 edit\ntoggle 600 200\nquit\n")
    with open(os.path.join(d, "line.kmall.esf"), "rb") as f:
      before = f.read()
    script = "open line.kmall 261 1 edit\ntoggle 500 300\nflags\nabort\n"
    with open(os.path.join(d, "script.txt"), "w") as f:
      f.write(script)
    out = subprocess.run([self.program, "--replay=script.txt"], cwd=d, capture_output=True, text=True).stdout
    want = re.findall(r"^flags: .* crc (\d+)", out, re.M)[-1]
    with open(os.path.join(d, "line.kmall.esf"), "rb") as f:
      self.assertEqual(f.read(), before)
    self.assertTrue(os.path.exists(os.path.join(d, "line.kmall.lck")))
    out = self.replay(d, "open line.kmall 261 1 edit\nquit\n")
    self.assertRegex(out, r"(?m)^open\s+status 0")
    out = self.replay(d, "unlock line.kmall\nrecover line.kmall 261 1 edit\nflags\nquit\n")
    self.assertEqual(re.findall(r"^flags: .* crc (\d+)", out, re.M)[-1], want)
    self.assertEqual(len(records(os.path.join(d, "line.kmall.esf"))), 2)

  def test_next_buffer_keeps_edits(self):
    d = self.folder("retained")
    out = self.replay(d, "buffer 20 10\nopen line.kmall 261 0 browse\nstep 15\nbeam 17 8\nquit\n")
    m = re.search(r"beam: ping 17 beam 8 flag 0x\w+ org 0x\w+ x (-?\d+) y (-?\d+)", out)
    x, y = m.group(1), m.group(2)
    self.replay(d, f"open line.kmall 261 0 edit\nstep 15\ntoggle {x} {y}\nquit\n")
    self.assertEqual(len(records(os.path.join(d, "line.kmall.esf"))), 1)
    out = self.replay(d, f"buffer 20 10\nopen line.kmall 261 1 edit\nstep 15\ntoggle {x} {y}\nnext\nbeam 7 8\nquit\n")
    self.assertRegex(out, r"beam: ping 7 beam 8 flag 0x00")
    self.assertEqual(records(os.path.join(d, "line.kmall.esf")), [])

  def test_undo(self):
    d = self.folder("undo")
    edits = "toggle 600 200\nerase 650 250\ngrab 400 100 700 300\nbad\nfilter 0 10 5 1 0 15 1 3 4 0 0 0 0 0 0\nzero\nlasso 400 100 800 100 800 400 400 400\n"
    out = self.replay(d, "open line.kmall 261 0 edit\nflags\n" + edits + "flags\n" + "undo\n" * 7 + "flags\nchanges\nundo\n" + "redo\n" * 8 + "flags\nquit\n")
    crc = re.findall(r"^flags: .* crc (\d+)", out, re.M)
    self.assertEqual(crc[0], crc[2])  # all undone
    self.assertNotEqual(crc[0], crc[1])
    self.assertEqual(crc[1], crc[3])  # all done again
    self.assertIn("changes: 0 unsaved 0", out)
    self.assertRegex(out, r"(?m)^undo\s+status 0")  # nothing left to undo
    with_undo = records(os.path.join(d, "line.kmall.esf"))
    d2 = self.folder("no_undo")
    self.replay(d2, "open line.kmall 261 0 edit\n" + edits + "quit\n")
    self.assertEqual(with_undo, records(os.path.join(d2, "line.kmall.esf")))

    # everything undone: nothing to save
    d3 = self.folder("undo_all")
    self.replay(d3, "open line.kmall 261 0 edit\ntoggle 600 200\nundo\nquit\n")
    self.assertEqual(sorted(os.listdir(d3)), ["line.kmall", "script.txt"])

    # a new edit ends redo, and Next Buffer ends undo
    d4 = self.folder("undo_buffer")
    out = self.replay(d4, "buffer 20 10\nopen line.kmall 261 0 browse\ntoggle 600 200\nundo\ntoggle 600 200\nredo\nnext\nundo\nquit\n")
    self.assertEqual(re.findall(r"(?m)^(undo|redo)\s+status (\d)", out), [("undo", "1"), ("redo", "0"), ("undo", "0")])

  def test_lasso_stays_inside(self):
    d = self.folder("lasso")
    out = self.replay(d, "open line.kmall 261 0 browse\nbeam 4 8\nbeam 4 9\nquit\n")
    x, y = re.search(r"beam: ping 4 beam 8 flag 0x\w+ org 0x\w+ x (-?\d+) y (-?\d+)", out).groups()
    x, y = int(x), int(y)
    box = f"{x - 2} {y - 2} {x + 2} {y - 2} {x + 2} {y + 2} {x - 2} {y + 2}"
    out = self.replay(d, f"open line.kmall 261 0 browse\nlasso {box}\nbeam 4 8\nbeam 4 9\nquit\n")
    self.assertRegex(out, r"beam: ping 4 beam 8 flag 0x05")
    self.assertRegex(out, r"beam: ping 4 beam 9 flag 0x00")

  def test_filter_preview_equals_apply(self):
    d = self.folder("preview")
    script = "open line.kmall 261 0 browse\ntoggle 600 200\n"
    for f in ("1 1 5 3 0 15 0 0 0 0 0 0 0 0 0", "0 10 5 1 1 2 0 0 0 0 0 0 0 0 0", "0 10 5 1 0 15 1 3 4 0 0 0 0 0 0", "0 10 5 1 0 15 1 20 4 0 0 0 0 0 0",
              "0 10 5 1 0 15 0 0 0 1 100 300 0 0 0", "0 10 5 1 0 15 0 0 0 0 0 0 1 40 -40", "1 1 5 3 1 2 1 3 4 1 100 300 1 40 -40", "0 10 5 1 0 15 0 0 0 0 0 0 0 0 0"):
      script += f"step 3\npreviewfilter {f}\nflags\nfilter {f}\nflags\n"
    out = self.replay(d, script + "quit\n")
    preview = re.findall(r"^preview: flag \d+ unflag \d+ pings \d+ crc (\d+)", out, re.M)
    flags = re.findall(r"^flags: .* crc (\d+)", out, re.M)
    self.assertEqual(len(preview), 8)
    self.assertEqual(preview, flags[1::2])
    self.assertEqual(flags[2::2], flags[1:-1:2])  # a preview changes no flags
    self.assertGreater(len(set(flags)), 3)

  def test_preview_edit_file(self):
    d = self.folder("esf")
    self.replay(d, "open line.kmall 261 0 edit\ntoggle 600 200\npick 500 300\nwriteedits current.esf\nquit\n")
    before = records(os.path.join(d, "line.kmall.esf"))
    self.assertEqual(records(os.path.join(d, "current.esf")), before)
    self.assertEqual(len(before), 2)
    out = self.replay(d, "open line.kmall 261 0 edit\nchanges\npreviewesf current.esf\nflags\npreviewesf current.esf accept\nchanges\nflags\nundo\nchanges\nredo\nquit\n")
    self.assertIn("previewesf: ok 1 flag 2 unflag 0 pings 2 complete 1", out)
    self.assertEqual(records(os.path.join(d, "line.kmall.esf")), before)

  def test_flags_by_reason(self):
    d = self.folder("reason")
    out = self.replay(d, "open line.kmall 261 0 browse\ntoggle 600 200\npick 500 300\nfilter 0 10 5 1 0 15 1 5 6 0 0 0 0 0 0\nunflagreason 1 2\nflags\nundo\nflags\nunflagreason 2 0\nquit\n")
    self.assertRegex(out, r"count: 2\n")
    crc = re.findall(r"^flags: .* manual (\d+) filter (\d+) filter2 (\d+)", out, re.M)
    self.assertEqual(crc[0][0], "0")
    self.assertEqual(crc[1][0], "2")
    self.assertGreater(int(crc[0][2]), 0)
    self.assertRegex(out, r"count: \d+\n[^\n]*\n")

  def test_suspects(self):
    d = self.folder("suspects")
    out = self.replay(d, "open line.kmall 261 0 browse\nflags\nsuspects\nflags\nquit\n")
    crc = re.findall(r"^flags: .* crc (\d+)", out, re.M)
    self.assertEqual(crc[0], crc[1])  # the ranking changes nothing
    # a flat seafloor has no spikes, but the rejected beam of the data is a gap in each ping
    scores = [float(x) for x in re.findall(r"^suspect: \d+ ([\d.]+)", out, re.M)]
    self.assertEqual(scores, sorted(scores, reverse=True))

  def test_plot_width_is_capped(self):
    d = os.path.join(self.tmp.name, "wide")
    os.mkdir(d)
    saved = S.ANGLE_MAX
    S.ANGLE_MAX = 89.5  # about 57 km to each side
    kmall, _ = S.write_line(d)
    S.ANGLE_MAX = saved
    shutil.copy(kmall, os.path.join(d, "line.kmall"))
    out = self.replay(d, "open line.kmall 261 0 browse\nquit\n")
    self.assertRegex(out, r"(?m)^open\s+status 1 .*width 40000 ")
    out = self.replay(d, "legacy 1\nopen line.kmall 261 0 browse\nquit\n")
    self.assertNotRegex(out, r"(?m)^open\s+status 1 .*width 40000 ")

  def test_window(self):
    d = os.path.join(self.tmp.name, "window")
    os.mkdir(d)
    saved = S.REJECTED, S.EXTRA
    S.REJECTED, S.EXTRA = -1, -1  # no beams flagged in the data
    kmall, _ = S.write_line(d)
    S.REJECTED, S.EXTRA = saved
    first = os.path.join(d, "a.kmall")
    second = os.path.join(d, "b.kmall")
    shutil.copy(kmall, first)
    shutil.copy(kmall, second)
    env = dict(os.environ)
    env.setdefault("QT_QPA_PLATFORM", "offscreen")
    env["PATH"] = os.path.dirname(os.environ["MBCLEAN"]) + os.pathsep + env.get("PATH", "")
    grid = self.grid(d, "a.kmall", "reference", "-A2")
    result = subprocess.run([os.environ["MBEDIT_QT_WINDOW_TEST"], first, second, grid], cwd=d, capture_output=True, text=True, errors="replace", env=env, timeout=300)
    self.assertEqual(result.returncode, 0, "\n".join(l for l in result.stdout.splitlines() if l.startswith("FAIL") or "checks" in l))

  def grid(self, folder, swath, name, kind):
    """a grid of the line itself"""
    # not name.mb-1: mbgrid writes that file itself
    datalist = os.path.join(folder, name + "_input.mb-1")
    with open(datalist, "w") as f:
      f.write(swath + " 261\n")
    env = dict(os.environ)
    env["PATH"] = os.path.dirname(os.environ["MBGRID"]) + os.pathsep + env.get("PATH", "")
    subprocess.run([os.environ["MBGRID"], "-I", datalist, kind, "-F1", "-E50/50/meters!", "-C5", "-O", name], cwd=folder, env=env, capture_output=True, check=True)
    return os.path.join(folder, name + ".grd")

  def test_reference_surface(self):
    """A depth grid and a topography grid of the same soundings give the same differences."""
    d = self.folder("reference")
    depth = self.grid(d, "line.kmall", "depth", "-A1")
    topography = self.grid(d, "line.kmall", "topography", "-A2")
    values = []
    for grid in (depth, topography):
      script = os.path.join(d, "script.txt")
      with open(script, "w") as f:
        f.write("open line.kmall 261 0 browse\nreference %s\n" % grid)
        f.writelines("residual %d %d\n" % (i, j) for i in (5, 20, 35) for j in (S.NBEAMS // 4, S.NBEAMS // 2, 3 * S.NBEAMS // 4))
        f.write("quit\n")
      result = subprocess.run([self.program, "--replay=" + script], cwd=d, capture_output=True, text=True, errors="replace")
      values.append([float(x) for x in re.findall(r"^residual: (\S+)", result.stdout, re.M)])
    self.assertEqual(len(values[0]), 9)
    self.assertEqual(str(values[0]), str(values[1]))
    finite = [v for v in values[0] if not math.isnan(v)]
    self.assertGreater(len(finite), 0)
    # a gridded flat seafloor agrees with its soundings; a wrong sign or an empty grid would be hundreds of meters off
    self.assertLess(max(abs(v) for v in finite), 50.0)

  def test_reference_bad_file(self):
    d = self.folder("reference_bad")
    with open(os.path.join(d, "bad.grd"), "w") as f:
      f.write("not a grid\n")
    script = os.path.join(d, "script.txt")
    with open(script, "w") as f:
      f.write("open line.kmall 261 0 browse\nreference bad.grd\nquit\n")
    result = subprocess.run([self.program, "--replay=" + script], cwd=d, capture_output=True, text=True, errors="replace")
    self.assertIn("Unable to read the grid", result.stdout)
    self.assertEqual(result.returncode, 0)


if __name__ == "__main__":
  unittest.main()
