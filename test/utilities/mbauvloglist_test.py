#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbauvloglist command line app."""

import os
import subprocess
import unittest


class MbAuvLogListTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbauvloglist'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open log file <> for reading', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('lists table data from an MBARI AUV mission log file', output)
    self.assertIn('-Fprintformat', output)


  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('lists table data from an MBARI AUV mission log file', output)
    self.assertIn('-Fprintformat', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('nprintfields', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
