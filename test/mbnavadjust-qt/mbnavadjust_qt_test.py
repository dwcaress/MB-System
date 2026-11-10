#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbnavadjust_qt_test.py  10/11/2026
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
"""mbnavadjust-qt on a synthetic two-survey project: usage, --dump and --replay. The expected rows and
statuses are what the Motif code in mbnavadjust_callbacks.c gave on this project. Run only when built
with -DbuildQt6Tools=ON."""

import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.environ["SYNTH_KMALL_DIR"])
import synth_kmall as S  # noqa: E402

PROGRAM = os.environ["MBNAVADJUST_QT"]
MERGE = os.environ["MBNAVADJUSTMERGE"]
TOOLS = os.path.dirname(os.environ["MBINFO"])
VIEW_TEST = os.environ["MBNAVADJUST_QT_VIEW_TEST"]
WINDOW_TEST = os.environ["MBNAVADJUST_QT_WINDOW_TEST"]

CROSSINGS = [
    "U     0 00:000:000 01:001:000   1  0",
    "U     1 00:000:001 01:001:000  69  0",
    "U     2 00:000:000 01:001:001  37  0",
    "UX    3 00:000:001 01:001:001  72  0",
    "U     4 00:000:000 01:001:002  50  0",
    "U     5 00:000:001 01:001:002  50  0",
    "U     6 01:001:000 01:001:002   6  0",
]


def write_lines(directory):
    """Two synthetic survey lines that cross."""
    S.NPINGS = 400
    S.NBEAMS = 64
    t0_base = S.T0
    files = []
    for name, (dlat, dlon, shift) in {"0001": (1.0e-5, 1.0e-5, 0.0), "0002": (-1.0e-5, 1.0e-5, 3000.0)}.items():
        line_dir = os.path.join(directory, name)
        os.makedirs(line_dir)
        S.LINE = name
        S.KMALL_NAME = name + "_20250101_120000.kmall"
        S.KMWCD_NAME = name + "_20250101_120001.kmwcd"
        S.T0 = t0_base + shift
        mid = S.ping_time(S.PING0 + S.NPINGS // 2)
        S.position = lambda t, dlat=dlat, dlon=dlon, mid=mid: (S.LAT0 + dlat * (t - mid), S.LON0 + dlon * (t - mid))
        files.append(S.write_line(line_dir)[0])
    return files


class MbnavadjustQtTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.top = tempfile.mkdtemp(prefix="mbnavadjust_qt_test")
        cls.env = dict(os.environ)
        cls.env["PATH"] = TOOLS + os.pathsep + cls.env["PATH"]
        base = os.path.join(cls.top, "base")
        os.makedirs(base)
        with open(os.path.join(base, "list.mb-1"), "w") as out:
            for path in write_lines(base):
                out.write("%s 261\n" % path)
        subprocess.run(
            [MERGE, "--create-project=" + os.path.join(base, "p.nvh"), "--section-length=2",
             "--section-soundings=20000", "--import=" + os.path.join(base, "list.mb-1") + ":-1", "--find-crossings"],
            cwd=base, env=cls.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        cls.base = base
        cls.count = 0

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.top, ignore_errors=True)

    def project(self):
        """A fresh copy of the project; the project file holds its own location."""
        cls = type(self)
        cls.count += 1
        work = os.path.join(cls.top, "run%d" % cls.count)
        os.makedirs(work)
        shutil.copy(os.path.join(cls.base, "p.nvh"), work)
        shutil.copytree(os.path.join(cls.base, "p.dir"), os.path.join(work, "p.dir"))
        path = os.path.join(work, "p.nvh")
        with open(path, encoding="latin-1") as stream:
            text = stream.read()
        text = re.sub(r"^PATH\t.*$", "PATH\t" + work + "/", text, count=1, flags=re.M)
        text = re.sub(r"^HOME\t.*$", "HOME\t" + path, text, count=1, flags=re.M)
        text = re.sub(r"^DATADIR\t.*$", "DATADIR\t" + work + "/p.dir", text, count=1, flags=re.M)
        with open(path, "w", encoding="latin-1") as stream:
            stream.write(text)
        return path

    def script(self, path, text):
        script = path + ".script"
        with open(script, "w") as stream:
            stream.write(text)
        return script

    def replay(self, path, commands):
        script = self.script(path, "open " + path + "\n" + commands)
        run = subprocess.run([PROGRAM, "--replay=" + script], cwd=os.path.dirname(path), env=self.env,
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, universal_newlines=True)
        self.assertEqual(run.returncode, 0, run.stdout[-2000:])
        return run.stdout.split("--- state\n")[1:]

    @staticmethod
    def values(state, key):
        return [line[len(key) + 2:] for line in state.splitlines() if line.startswith(key + ": ")]

    def enabled(self, state, widget):
        return self.values(state, "enabled").count(widget + " 1") == 1

    def test_usage(self):
        out = subprocess.check_output([PROGRAM, "--help"], universal_newlines=True)
        self.assertIn("Program mbnavadjust-qt", out)
        self.assertIn("--input=project", out)
        self.assertNotEqual(subprocess.call([PROGRAM, "--nosuchoption"], stderr=subprocess.DEVNULL), 0)

    def test_bad_script_line(self):
        path = self.project()
        script = path + ".script"
        with open(script, "w") as stream:
            stream.write("nosuchcommand\n")
        self.assertNotEqual(subprocess.call([PROGRAM, "--replay=" + script], stderr=subprocess.DEVNULL), 0)

    def test_dump_shows_every_list(self):
        path = self.project()
        run = subprocess.run([PROGRAM, "--input=" + path, "--dump"], cwd=os.path.dirname(path), env=self.env,
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, universal_newlines=True)
        self.assertEqual(run.returncode, 0)
        self.assertEqual(run.stdout.count("--- state\n"), 16)
        self.assertIn("list-title: Crossings:", run.stdout)
        self.assertIn("list-title: >50% Crossings:", run.stdout)
        self.assertIn("list-title: Sorted Ties:", run.stdout)

    def test_list_and_enabled_states(self):
        path = self.project()
        files, crossings = self.replay(path, "state\nlist crossings\nstate\n")
        self.assertEqual(self.values(files, "list-title"), ["Data Files:"])
        self.assertEqual(self.values(files, "list-rows"), ["2"])
        self.assertIn("00:0000  good      3  0.0  0.0 g-ties:0  0001/0001_20250101_120000.kmall",
                      self.values(files, "row"))
        self.assertEqual(self.values(crossings, "row"), CROSSINGS)
        self.assertEqual(self.values(crossings, "list-selected"), ["none"])
        status = self.values(crossings, "status")
        self.assertIn("Number of Crossings Found:                7     Selected File:      -1", status)
        self.assertFalse(self.enabled(crossings, "pushButton_showcrossings"))
        self.assertTrue(self.enabled(crossings, "pushButton_showdata"))
        self.assertTrue(self.enabled(crossings, "pushButton_autopick"))
        self.assertFalse(self.enabled(crossings, "pushButton_open"))
        self.assertTrue(self.enabled(crossings, "pushButton_close"))

    def test_add_tie_is_saved(self):
        path = self.project()
        states = self.replay(path, "list crossings\nclick 1\nstate\nnext\nstate\nminmisfit\naddtie\nstate\nwrite\n")
        self.assertEqual(self.values(states[0], "selection")[0].split()[-3:], ["0", "tie", "-1"])
        self.assertEqual(self.values(states[0], "window")[0], "mode 1 status 2 current 0 -1 -1 -1")
        self.assertEqual(self.values(states[1], "window")[0], "mode 1 status 2 current 1 -1 -1 -1")
        self.assertIn("Crossing: 1 of 7", self.values(states[1], "crossing"))
        after = states[2]
        self.assertTrue(any(line.startswith("Number of Ties Set:") and line.endswith(" 1") for line in self.values(after, "status")))
        self.assertTrue(self.values(after, "row")[1].startswith("*"))
        self.assertTrue(self.values(after, "row")[1].endswith(" 1"))
        self.assertTrue(self.enabled(after, "pushButton_naverr_deletetie"))

        reopened = self.replay(path, "state\n")[0]
        self.assertTrue(any(line.startswith("Number of Ties Set:") and line.endswith(" 1") for line in self.values(reopened, "status")))

    def test_step_past_the_only_crossing(self):
        # The Motif plot function reads project.crossings[-1] here.
        path = self.project()
        state = self.replay(path, "list truecrossings\nclick 1\nnext\nstate\n")[0]
        self.assertEqual(self.values(state, "window")[0], "mode 1 status 2 current -1 -1 -1 -1")

    def test_mouse_on_the_contour_plot(self):
        path = self.project()
        states = self.replay(path, "list crossings\nclick 2\nstate\ncontpress 1 300 300\ncontmove 340 280\n"
                                   "contrelease 1 340 280\nstate\ncontpress 2 200 200\ncontmove 400 400\n"
                                   "contrelease 2 400 400\nstate\n")
        moved = [float(value) for value in self.values(states[1], "working")[0].split()[:2]]
        start = [float(value) for value in self.values(states[0], "working")[0].split()[:2]]
        self.assertGreater(moved[0], start[0])
        self.assertGreater(moved[1], start[1])
        self.assertTrue(self.enabled(states[2], "pushButton_naverr_fullsize"))

    def test_contours_from_layers_match_shared_code(self):
        path = self.project()
        run = subprocess.run([PROGRAM, "--replay=" + self.script(path, "open %s\nlist crossings\nclick 2\ncontourcheck\n"
                                                                       "contpress 1 300 300\ncontmove 330 280\n"
                                                                       "contrelease 1 330 280\ncontourcheck\n" % path)],
                             cwd=os.path.dirname(path), env=self.env, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                             universal_newlines=True)
        self.assertEqual(run.returncode, 0)
        self.assertEqual(re.findall(r"contour-difference: (\d+)", run.stdout), ["0", "0"])

    def test_export_writes_the_plots(self):
        path = self.project()
        prefix = path + "_export"
        subprocess.check_call([PROGRAM, "--input=" + path, "--export=" + prefix, "--crossing=1"], cwd=os.path.dirname(path),
                              env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for name in ("contour", "misfit", "zoffset"):
            with open("%s_%s.png" % (prefix, name), "rb") as stream:
                self.assertEqual(stream.read(8), b"\x89PNG\r\n\x1a\n", name)

    def test_model_plot_and_widgets(self):
        path = self.project()
        self.replay(path, "list crossings\nclick 2\nminmisfit\naddtie\nsettie\nnext\nminmisfit\naddtie\nsettie\nnext\n"
                          "minmisfit\naddtie\nsettie\ndismiss\ninvert\nwrite\n")
        states = self.replay(path, "list ties\nclick 1\nmodelshow\nstate\nmodelstyle tieoffsets\nstate\n")
        self.assertEqual(len(self.values(states[0], "model")), 2)
        self.assertNotEqual(self.values(states[0], "plots")[0].split("|")[3].split(), ["0", "0", "0", "0"])
        run = subprocess.run([VIEW_TEST, path], cwd=os.path.dirname(path), env=self.env, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, universal_newlines=True)
        self.assertEqual(run.returncode, 0, run.stdout[-3000:])

    def test_main_window(self):
        path = self.project()
        self.replay(path, "list crossings\nclick 2\nminmisfit\naddtie\nsettie\nnext\nminmisfit\naddtie\nsettie\nnext\n"
                          "minmisfit\naddtie\nsettie\ndismiss\ninvert\nwrite\n")
        run = subprocess.run([WINDOW_TEST, path], cwd=os.path.dirname(path), env=self.env, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, universal_newlines=True)
        self.assertEqual(run.returncode, 0, run.stdout[-4000:])


if __name__ == "__main__":
    unittest.main()
