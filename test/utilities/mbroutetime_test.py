#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbroutetime command line app."""

import os
import subprocess
import unittest


class MbroutetimeTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbroutetime'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open route file <> for reading', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('list of the times when a survey hit the waypoints', output)
    self.assertIn('usage:', output)
    self.assertIn('-Urangethreshold', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('list of the times when a survey hit the waypoints', output)
    self.assertIn('usage:', output)
    self.assertIn('-Urangethreshold', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('rangethreshold:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
