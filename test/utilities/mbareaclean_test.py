#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbareaclean command line app."""

import os
import subprocess
import unittest


class MbAreaCleanTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbareaclean'

  def testNoArgs(self):
    cmd = [self.cmd]
    subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('usage:', output)
    self.assertIn('minbeam/maxbeam', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('usage:', output)
    self.assertIn('minbeam/maxbeam', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('median_filter', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
