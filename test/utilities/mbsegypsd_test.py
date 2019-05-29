#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbsegypsd command line app."""

import os
import subprocess
import unittest


class MbsegypsdTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbsegypsd'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Bad trace numbers: 0 0 specified', output)
    self.assertIn('Terminated', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('power spectral density function', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tsweep[/delay]', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('power spectral density function', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tsweep[/delay]', output)
    self.assertIn('dbg2', output)
    self.assertIn('frequencyscale:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
