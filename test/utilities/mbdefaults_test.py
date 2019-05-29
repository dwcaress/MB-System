#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbdefaults command line app."""

import os
import subprocess
import unittest


class MbdefaultsTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbdefaults'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Current MBIO Default Control Parameters:', output)
    self.assertIn('lonflip:', output)
    self.assertIn('mbview slope magnitude:', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('sets and retrieves', output)
    self.assertIn('parameters', output)
    self.assertIn('-Wproject', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('sets and retrieves', output)
    self.assertIn('parameters', output)
    self.assertIn('-Wproject', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('illuminate_azimuth:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
