#!/usr/bin/env python3

"""Tests for mbgrdviz.

Unlike the other Motif GUI editors, mbgrdviz parses -h/--help before
opening an X display, so this test runs it directly without Xvfb.
"""

import subprocess
import unittest


class MbgrdvizTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/mbgrdviz/mbgrdviz'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program MBgrdviz', output)
    self.assertIn('interactive 2D/3Dvizualization of GMT grids', output)


if __name__ == '__main__':
  unittest.main()
