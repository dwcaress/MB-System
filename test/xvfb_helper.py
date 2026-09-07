#!/usr/bin/env python3

"""Shared helper for smoke tests of Motif GUI programs that call
XtOpenDisplay() before parsing arguments, so even '-h'/'--help' requires a
working X display.

Tests that use XvfbTestCase are skipped (not failed) when no Xvfb binary is
available, since not every development or CI machine has one installed.
"""

import os
import random
import shutil
import subprocess
import time
import unittest


def find_xvfb():
  """Locates the Xvfb binary, checking PATH and the common XQuartz location."""
  found = shutil.which('Xvfb')
  if found:
    return found
  xquartz_xvfb = '/opt/X11/bin/Xvfb'
  if os.path.exists(xquartz_xvfb):
    return xquartz_xvfb
  return None


class XvfbTestCase(unittest.TestCase):
  """Base class that starts a private Xvfb server for the test class.

  Subclasses can run GUI programs against self.env (which has DISPLAY set)
  without needing a real display attached to the machine.
  """

  @classmethod
  def setUpClass(cls):
    cls.xvfb_path = find_xvfb()
    if not cls.xvfb_path:
      raise unittest.SkipTest('Xvfb not found; skipping GUI smoke test')

    # Pick a high, unlikely-to-collide display number rather than a fixed
    # one, since multiple test binaries may run concurrently under ctest -j.
    display_num = random.randint(100, 999)
    cls.display = ':{}'.format(display_num)
    cls.env = dict(os.environ)
    cls.env['DISPLAY'] = cls.display

    cls.xvfb_proc = subprocess.Popen(
        [cls.xvfb_path, cls.display, '-screen', '0', '1024x768x24'],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    # Give Xvfb a moment to bind the display before programs try to open it.
    time.sleep(1.5)

  @classmethod
  def tearDownClass(cls):
    proc = getattr(cls, 'xvfb_proc', None)
    if proc is not None:
      proc.terminate()
      try:
        proc.wait(timeout=5)
      except subprocess.TimeoutExpired:
        proc.kill()
