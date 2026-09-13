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


def _display_locked(display_num):
  """True if /tmp/.X<N>-lock exists, i.e. some X server already claims it."""
  return os.path.exists('/tmp/.X{}-lock'.format(display_num))


class XvfbTestCase(unittest.TestCase):
  """Base class that starts a private Xvfb server for the test class.

  Subclasses can run GUI programs against self.env (which has DISPLAY set)
  without needing a real display attached to the machine.
  """

  # How long to wait for Xvfb to bind its display before giving up. A fixed
  # short sleep here was flaky under load (e.g. running the test suite right
  # after a full rebuild, or with other Xvfb instances competing for startup
  # time), so this polls for readiness instead with a generous ceiling.
  STARTUP_TIMEOUT_SECONDS = 15
  POLL_INTERVAL_SECONDS = 0.05

  @classmethod
  def setUpClass(cls):
    cls.xvfb_path = find_xvfb()
    if not cls.xvfb_path:
      raise unittest.SkipTest('Xvfb not found; skipping GUI smoke test')

    # Pick a high, unlikely-to-collide display number rather than a fixed
    # one, since multiple test binaries may run concurrently under ctest -j.
    # Skip any number that's already locked by another X server (a
    # concurrently-running test, or a stale leftover process) instead of
    # colliding with it.
    display_num = random.randint(100, 999)
    for _ in range(20):
      if not _display_locked(display_num):
        break
      display_num = random.randint(100, 999)
    cls.display = ':{}'.format(display_num)
    cls.env = dict(os.environ)
    cls.env['DISPLAY'] = cls.display

    cls.xvfb_proc = subprocess.Popen(
        [cls.xvfb_path, cls.display, '-screen', '0', '1024x768x24'],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Wait for Xvfb to actually bind the display (signaled by its lock file
    # appearing) rather than hoping a fixed sleep was long enough, and bail
    # out clearly if the process dies instead of hanging until the test's
    # own command eventually fails to connect.
    deadline = time.time() + cls.STARTUP_TIMEOUT_SECONDS
    while time.time() < deadline:
      if _display_locked(display_num):
        break
      if cls.xvfb_proc.poll() is not None:
        raise RuntimeError(
            'Xvfb exited early (code {}) before binding display {}'.format(
                cls.xvfb_proc.returncode, cls.display))
      time.sleep(cls.POLL_INTERVAL_SECONDS)
    else:
      cls.xvfb_proc.kill()
      cls.xvfb_proc.wait(timeout=5)
      raise RuntimeError(
          'Xvfb did not bind display {} within {}s'.format(
              cls.display, cls.STARTUP_TIMEOUT_SECONDS))

    # The lock file appearing means Xvfb has bound the display's Unix
    # socket, but a client connecting in the same instant it appears can
    # still occasionally race the server's own setup; a brief settle delay
    # avoids that without reintroducing a long fixed wait.
    time.sleep(0.2)

  @classmethod
  def tearDownClass(cls):
    proc = getattr(cls, 'xvfb_proc', None)
    if proc is not None:
      proc.terminate()
      try:
        proc.wait(timeout=5)
      except subprocess.TimeoutExpired:
        proc.kill()
