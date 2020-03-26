#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mblevitus command line app."""

import os
import subprocess
import unittest


class MblevitusTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mblevitus'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('generates an average water velocity profile', output)
    self.assertIn('usage:', output)
    self.assertIn('-Rlon/lat', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('generates an average water velocity profile', output)
    self.assertIn('usage:', output)
    self.assertIn('-Rlon/lat', output)
    self.assertIn('dbg2', output)
    self.assertIn('levitusfile:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
