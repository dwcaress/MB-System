#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbrphsbias command line app."""

import os
import subprocess
import unittest


class MbrphsbiasTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbrphsbias'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('sonar soundings to solve for bias parameters', output)
    self.assertIn('usage:', output)
    self.assertIn('-Sbinsize', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('sonar soundings to solve for bias parameters', output)
    self.assertIn('usage:', output)
    self.assertIn('-Sbinsize', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('binsizeset:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
