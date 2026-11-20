#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbwcdview_test.py  1/11/2026
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
"""mbwcdview (C++/Qt6) without a window: usage, --dump and --export on a synthetic
*.kmall/*.kmwcd pair, *.wcd file and Reson 7k *.s7k file. Run only when built with
-DbuildQt6Tools=ON."""

import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import synth_all as A  # noqa: E402
import synth_kmall as S  # noqa: E402
import synth_s7k as R  # noqa: E402


def read_png(path):
  """Width, height, bytes per pixel and rows of an 8 bit RGB or RGBA PNG without interlace, as Qt writes them."""
  with open(path, "rb") as f:
    data = f.read()
  pos, header, idat = 8, None, b""
  while pos < len(data):
    length, kind = struct.unpack(">I4s", data[pos:pos + 8])
    if kind == b"IHDR":
      header = struct.unpack(">IIBBBBB", data[pos + 8:pos + 8 + length])
    elif kind == b"IDAT":
      idat += data[pos + 8:pos + 8 + length]
    pos += 12 + length
  width, height, depth, color, _, _, interlace = header
  if depth != 8 or color not in (2, 6) or interlace != 0:
    raise ValueError("unsupported PNG %s" % (header,))
  bpp = 3 if color == 2 else 4
  stride = width * bpp
  raw = zlib.decompress(idat)
  rows, prev = [], bytearray(stride)
  for y in range(height):
    start = y * (stride + 1)
    kind, line = raw[start], bytearray(raw[start + 1:start + 1 + stride])
    for i in range(stride):
      a = line[i - bpp] if i >= bpp else 0
      b = prev[i]
      c = prev[i - bpp] if i >= bpp else 0
      if kind == 1:
        line[i] = (line[i] + a) & 0xff
      elif kind == 2:
        line[i] = (line[i] + b) & 0xff
      elif kind == 3:
        line[i] = (line[i] + ((a + b) >> 1)) & 0xff
      elif kind == 4:
        p = a + b - c
        pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
        line[i] = (line[i] + (a if pa <= pb and pa <= pc else (b if pb <= pc else c))) & 0xff
    rows.append(line)
    prev = line
  return width, height, bpp, rows


class MbwcdviewTest(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.program = os.environ["MBWCDVIEW"]
    cls.tmp = tempfile.TemporaryDirectory()
    cls.kmall, cls.kmwcd = S.write_line(cls.tmp.name)

  @classmethod
  def tearDownClass(cls):
    cls.tmp.cleanup()

  def test_usage(self):
    result = subprocess.run([self.program, "-H"], capture_output=True, text=True)
    self.assertEqual(result.returncode, 0)
    self.assertIn("usage: mbwcdview", result.stderr)

  def test_dump(self):
    out = subprocess.run([self.program, "--dump", "-I", self.kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    self.assertIn(S.KMALL_NAME, out)
    self.assertIn("water column pings %d, matched %d" % (S.NPINGS - 1, S.NPINGS - 1), out)
    self.assertIn("fill value -94.0 dB, amplitude -55.0 .. -10.0 dB", out)
    self.assertIn("ping number %d" % (S.PING0 + 2), out)
    ratio = re.search(r"straight ([0-9.]+), ray traced ([0-9.]+)", out)
    self.assertIsNotNone(ratio, out)
    # flat seafloor, constant sound speed: both geometries agree and put the detections on the
    # soundings, apart from the 1-2 deg along track tilt of the sectors (cos 2 deg = 0.9994)
    self.assertAlmostEqual(float(ratio.group(1)), float(ratio.group(2)), places=4)
    self.assertAlmostEqual(float(ratio.group(1)), 1.0, delta=0.001)

  def test_export(self):
    image = os.path.join(self.tmp.name, "ping.png")
    subprocess.run([self.program, "-I", self.kmall, "--ping=1", "--bottom", "--wc-bottom", "--export=" + image,
                    "--size=800x600"], check=True)
    with open(image, "rb") as f:
      self.assertEqual(f.read(8), b"\x89PNG\r\n\x1a\n")

  def test_export_draws_swath(self):
    # the swath is drawn on the graphics card (QRhi or OpenGL): the image area of the
    # plot holds many colours of the colour map, where axes and labels have none
    image = os.path.join(self.tmp.name, "swath.png")
    subprocess.run([self.program, "-I", self.kmall, "--ping=1", "--export=" + image, "--size=400x300"], check=True)
    width, height, bpp, rows = read_png(image)
    # image area of WcdPlot::plotRect(): margins 72 left, 104 right, 16 top, 46 + 20 bottom
    colours = set()
    for y in range(16 + 4, height - 66 - 4):
      row = rows[y]
      for x in range(72 + 4, width - 104 - 4):
        r, g, b = row[x * bpp], row[x * bpp + 1], row[x * bpp + 2]
        if max(r, g, b) - min(r, g, b) > 16:
          colours.add((r, g, b))
    self.assertGreater(len(colours), 20)

  def test_dump_wcd(self):
    wcd = A.write(os.path.join(self.tmp.name, A.WCD_NAME))
    out = subprocess.run([self.program, "--dump", "-I", wcd, "--ping=2"], capture_output=True, text=True,
                         check=True).stdout
    self.assertIn("water column pings %d, matched 0" % len(A.PINGS), out)
    self.assertIn("amplitude %.1f .. " % (0.5 * -128 - A.TVG_OFFSET), out)
    self.assertIn("fill value none", out)
    self.assertIn("ping number %d" % A.PINGS[2], out)
    self.assertIn("beams %d," % (A.nbeams_read(2) + 2), out)  # two empty beams mark the lost datagram

  def test_dump_s7k(self):
    """A *.s7k holds the survey pings and the water column; its soundings are the detections,
    which the water column gives in fractions of a sample, so the beams are compared with the
    soundings of their travel time (with an extra detection early in ping 0, by index most outer
    beams would get a neighbour's sounding, metres away)."""
    s7k = R.write(self.tmp.name)
    out = subprocess.run([self.program, "--dump", "-I", s7k, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    nwc = R.NPINGS - len(R.NO_WC)
    self.assertIn("survey file: %s\nwater column file: %s" % (s7k, s7k), out)
    self.assertIn("survey pings %d," % R.NPINGS, out)
    self.assertIn("water column pings %d, matched %d" % (nwc, nwc), out)
    self.assertIn("fill value none", out)
    self.assertIn("ping number %d" % (R.PING0 + 1), out)
    # the sonar is far below the surface: the depth window starts just above it
    window = re.search(r"longest slant range ([0-9.]+) m, depth window from ([0-9.]+) m", out)
    self.assertIsNotNone(window, out)
    self.assertAlmostEqual(float(window.group(2)), R.SENSORDEPTH - 0.1 * float(window.group(1)), delta=0.11)
    ratio = re.search(r"straight ([0-9.]+), ray traced ([0-9.]+)", out)
    self.assertIsNotNone(ratio, out)
    # flat seafloor, constant sound speed, no profile: the detections lie on the soundings
    self.assertAlmostEqual(float(ratio.group(1)), 1.0, delta=1.0e-4)
    self.assertEqual(ratio.group(1), ratio.group(2))
    across = re.search(r"sounding\|: straight ([0-9.]+) m", out)
    self.assertIsNotNone(across, out)
    self.assertLess(float(across.group(1)), 0.05)
    self.assertIn("by travel time: %d (detections in fractions of a sample), 0 (" % R.NBEAMS, out)

  def plot_colours(self, image):
    """Colours in the image area of the plot (as in test_export_draws_swath)."""
    width, height, bpp, rows = read_png(image)
    colours = set()
    for y in range(16 + 4, height - 66 - 4):
      row = rows[y]
      for x in range(72 + 4, width - 104 - 4):
        r, g, b = row[x * bpp], row[x * bpp + 1], row[x * bpp + 2]
        if max(r, g, b) - min(r, g, b) > 16:
          colours.add((r, g, b))
    return colours

  def test_export_s7k(self):
    s7k = R.write(self.tmp.name)
    image = os.path.join(self.tmp.name, "s7k.png")
    subprocess.run([self.program, "-I", s7k, "--ping=0", "--bottom", "--wc-bottom", "--export=" + image,
                    "--size=400x300"], check=True)
    self.assertTrue(os.path.getsize(image) > 0)
    # without the overlays, whose markers alone have many colours: the swath is drawn
    subprocess.run([self.program, "-I", s7k, "--ping=0", "--export=" + image, "--size=400x300"], check=True)
    self.assertGreater(len(self.plot_colours(image)), 20)

  def test_dump_extra_detections(self):
    """With an extra detection in the first receive fan and a sound speed gradient, straight
    rays put the detections on the (straight ray) soundings and ray tracing through the
    gradient moves the outer ones away: the pairs found once serve both geometries (paired in
    each geometry the ray traced comparison found no soundings). test_dump_pairing checks
    the pairs themselves."""
    directory = os.path.join(self.tmp.name, "extra")
    os.mkdir(directory)
    kmall, kmwcd = S.write_line(directory, extra=True, extra_fan=0, svp_gradient=40.0)
    out = subprocess.run([self.program, "--dump", "-I", kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    across = re.search(r"sounding\|: straight ([0-9.na]+) m, ray traced ([0-9.na]+) m", out)
    self.assertIsNotNone(across, out)
    self.assertLess(float(across.group(1)), 0.5)
    self.assertGreater(float(across.group(2)), 5.0)

  def test_dump_pairing(self):
    """Soundings that a sonar's ray tracing moved toward nadir, here by a fifth of their across
    track distance, lie nearer in direction to another beam's detection than to their own;
    with the extra detection in the first fan they are not in beam order either. Each beam
    finds its own sounding by travel time, so the outer beams' water column detections (on
    straight rays) lie a fifth of their across track distance outside their soundings."""
    directory = os.path.join(self.tmp.name, "pairing")
    os.mkdir(directory)
    kmall, kmwcd = S.write_line(directory, extra=True, extra_fan=0, sounding_scale=0.8)
    out = subprocess.run([self.program, "--dump", "-I", kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    ratio = re.search(r"straight ([0-9.na]+), ray traced ([0-9.na]+)", out)
    self.assertIsNotNone(ratio, out)
    self.assertAlmostEqual(float(ratio.group(1)), 1.0, delta=0.001)
    across = re.search(r"sounding\|: straight ([0-9.na]+) m, ray traced ([0-9.na]+) m", out)
    self.assertIsNotNone(across, out)
    outer = 0.2 * S.BOTTOM_BELOW_TX * math.tan(math.radians(S.ANGLE_MAX))
    self.assertAlmostEqual(float(across.group(1)), outer, delta=1.0)
    self.assertEqual(across.group(1), across.group(2))  # constant sound speed: ray tracing changes nothing
    self.assertIn("by travel time: %d (detections in fractions of a sample), 0 (whole samples, ties by beam angle), "
                  "0 (whole samples, ties by direction)" % (2 * S.NBEAMS), out)

  def whole_samples_dump(self, name, **options):
    """--dump of the pairing line of test_dump_pairing with its water column detections in
    whole samples, as *.all and *.wcd files give them; returns the output and the outer beams'
    across track difference to expect when each beam takes its own sounding (the detection on
    straight rays at the whole sample before the sounding's travel time)."""
    directory = os.path.join(self.tmp.name, name)
    os.mkdir(directory)
    kmall, kmwcd = S.write_line(directory, extra=True, extra_fan=0, sounding_scale=0.8, whole_samples=True,
                                **options)
    out = subprocess.run([self.program, "--dump", "-I", kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    x = S.BOTTOM_BELOW_TX * math.tan(math.radians(S.ANGLE_MAX))
    det = S.detect_samples(0)
    return out, x * int(det) / det - 0.8 * x

  def test_dump_pairing_whole_samples(self):
    """Whole samples do not give the soundings' travel times exactly: each beam takes the
    sounding nearest in time, of those within a sample the one nearest in beam angle (the
    beam angles MBIO gives agree with those of the water column), its own."""
    out, outer = self.whole_samples_dump("whole")
    self.assertIn("by travel time: 0 (detections in fractions of a sample), %d (whole samples, ties by beam angle), "
                  "0 (whole samples, ties by direction)" % (2 * S.NBEAMS), out)
    across = re.search(r"sounding\|: straight ([0-9.na]+) m", out)
    self.assertIsNotNone(across, out)
    self.assertAlmostEqual(float(across.group(1)), outer, delta=1.0)

  def test_dump_pairing_whole_samples_angles_off(self):
    """As test_dump_pairing_whole_samples, with the soundings' beam angles 7.5 deg off those of
    the water column (as MBIO's for kmall, computed with the roll at the echo, can be): ties go
    by direction, and each beam still takes its own sounding."""
    out, outer = self.whole_samples_dump("whole_off", rx_angle_offset=7.5)
    self.assertIn("by travel time: 0 (detections in fractions of a sample), 0 (whole samples, ties by beam angle), "
                  "%d (whole samples, ties by direction)" % (2 * S.NBEAMS), out)
    across = re.search(r"sounding\|: straight ([0-9.na]+) m", out)
    self.assertIsNotNone(across, out)
    self.assertAlmostEqual(float(across.group(1)), outer, delta=1.0)

  def test_unmatched_deep(self):
    """A water column ping without its survey ping, from a sonar deep below the surface whose
    depth changes from ping to ping, is drawn at the transducer depth interpolated from the
    survey pings around it, inside the default depth window."""
    directory = os.path.join(self.tmp.name, "deep")
    os.mkdir(directory)
    kmall, kmwcd = S.write_line(directory, draft=3000.0, draft_step=10.0, skip_mrz=(S.PING0 + 2,))
    out = subprocess.run([self.program, "--dump", "-I", kmwcd, "--ping=1"], capture_output=True, text=True,
                         check=True).stdout
    self.assertIn("water column pings %d, matched %d" % (S.NPINGS - 1, S.NPINGS - 2), out)
    # half way between the pings before (3010 m) and after (3030 m) it
    self.assertIn("ping number %d" % (S.PING0 + 2), out)
    self.assertIn("transducer depth 3020.000 m (interpolated)", out)
    window = re.search(r"longest slant range ([0-9.]+) m, depth window from ([0-9.]+) m", out)
    self.assertIsNotNone(window, out)
    self.assertAlmostEqual(float(window.group(2)), 3000.0 - 0.1 * float(window.group(1)), delta=0.11)
    image = os.path.join(directory, "unmatched.png")
    subprocess.run([self.program, "-I", kmwcd, "--ping=1", "--export=" + image, "--size=400x300"], check=True)
    self.assertGreater(len(self.plot_colours(image)), 20)

  def test_dump_no_data(self):
    """An empty file or one without data of its format has neither survey nor water column:
    --dump fails with a message."""
    directory = os.path.join(self.tmp.name, "nodata")
    os.mkdir(directory)
    for name, content in (("empty.kmall", b""), ("garbage.kmall", b"no sonar data here\n" * 50),
                          ("empty.s7k", b""), ("garbage.s7k", b"no sonar data here\n" * 50)):
      path = os.path.join(directory, name)
      with open(path, "wb") as f:
        f.write(content)
      result = subprocess.run([self.program, "--dump", "-I", path], capture_output=True, text=True)
      self.assertNotEqual(result.returncode, 0, name)
      self.assertIn("no water column or survey data", result.stderr, name)

  def test_ship_window(self):
    """A ship's transducer is shallower than the longest slant range: the window starts at the
    surface."""
    directory = os.path.join(self.tmp.name, "ship")
    os.mkdir(directory)
    kmall, kmwcd = S.write_line(directory, draft=300.0)
    out = subprocess.run([self.program, "--dump", "-I", kmall], capture_output=True, text=True,
                         check=True).stdout
    window = re.search(r"longest slant range ([0-9.]+) m, depth window from ([0-9.]+) m", out)
    self.assertIsNotNone(window, out)
    self.assertGreater(300.0, 0.1 * float(window.group(1)))  # the rule before started below the surface
    self.assertEqual(float(window.group(2)), 0.0)

  def test_window(self):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([os.environ["MBWCDVIEW_WINDOW_TEST"], self.kmall, self.tmp.name], capture_output=True,
                            text=True, cwd=self.tmp.name, env=env, timeout=300)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
    self.assertIn(" 0 failed", result.stdout)

  def test_embed(self):
    # two copies of the line, because each tool opens its own files
    a, b = (os.path.join(self.tmp.name, name) for name in ("embed_a", "embed_b"))
    os.mkdir(a)
    os.mkdir(b)
    file_a = S.write_line(a)[0]
    file_b = S.write_line(b)[0]
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([os.environ["MBWCDVIEW_EMBED_TEST"], file_a, file_b, self.tmp.name], capture_output=True,
                            text=True, cwd=self.tmp.name, env=env, timeout=300)
    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
    self.assertIn(" 0 failed", result.stdout)


if __name__ == "__main__":
  unittest.main()
