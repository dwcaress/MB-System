#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbneptune2esf command line app."""

import os
import subprocess
import unittest


class Mbneptune2esfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbneptune2esf'

  # testNoArgs hangs.

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('Simrad Neptune BinStat rules file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Rrules', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('Simrad Neptune BinStat rules file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Rrules', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('speedmin:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
