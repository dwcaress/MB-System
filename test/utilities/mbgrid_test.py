#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbgrid command line app."""

import os
import subprocess
import unittest


class MbGridTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbgrid'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid:\n--------------------------")
    print(output)
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid -h:\n--------------------------")
    print(output)
    self.assertIn('Version', output)
    self.assertIn('used to grid bathymetry, amplitude', output)
    self.assertIn('curvature algorithm', output)
    self.assertIn('usage:', output)
    self.assertIn('-Xextend', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid -h -V -V:\n--------------------------")
    print(output)
    self.assertIn('Version', output)
    self.assertIn('used to grid bathymetry, amplitude', output)
    self.assertIn('curvature algorithm', output)
    self.assertIn('usage:', output)
    self.assertIn('-Xextend', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('minormax_weighted_mean_threshold:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
