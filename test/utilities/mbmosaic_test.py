#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbmosaic command line app."""

import os
import subprocess
import unittest


class MbmosaicTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbmosaic'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('utility used to mosaic amplitude', output)
    self.assertIn('usage:', output)
    self.assertIn('-Zbathdef', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('utility used to mosaic amplitude', output)
    self.assertIn('usage:', output)
    self.assertIn('-Zbathdef', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('topogridfile:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
