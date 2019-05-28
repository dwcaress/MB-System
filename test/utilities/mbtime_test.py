#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbtime command line app."""

import os
import subprocess
import unittest


class MbtimeTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbtime'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('1970/01/01/00/00/00.000000', output.strip())

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('converts time values between epoch seconds', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tyear/month/day/hour/minute/second', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('converts time values between epoch seconds', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tyear/month/day/hour/minute/second', output)
    self.assertIn('dbg2', output)
    self.assertIn('time_d:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
