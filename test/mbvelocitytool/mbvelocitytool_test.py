#!/usr/bin/env python3

"""Tests for the mbvelocitytool interactive (Motif) GUI app.

mbvelocitytool opens an X display before parsing arguments, so even -h
needs a working DISPLAY; this test runs it under a private Xvfb server. See
xvfb_helper.py for details, and note the test is skipped rather than failed
when no Xvfb binary is available.
"""

import os
import subprocess
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from xvfb_helper import XvfbTestCase


class MbvelocitytoolTest(XvfbTestCase):

  def setUp(self):
    self.cmd = '../../src/mbvelocitytool/mbvelocitytool'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(
        cmd, env=self.env, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program MBVELOCITYTOOL', output)
    self.assertIn('interactive water velocity profile editor', output)


if __name__ == '__main__':
  unittest.main()
