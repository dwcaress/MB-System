#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbsegygrid command line app."""

import os
import subprocess
import unittest


class MbsegygridTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbsegygrid'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Bad trace numbers: 0 0 specified', output)
    self.assertIn('Terminated', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('grids trace data from segy data files', output)
    self.assertIn('usage:', output)
    self.assertIn('-Wmode/start/end', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('grids trace data from segy data files', output)
    self.assertIn('usage:', output)
    self.assertIn('-Wmode/start/end', output)
    self.assertIn('dbg2', output)
    self.assertIn('scale2distance:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
