#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbminirovnav command line app."""

import os
import subprocess
import unittest


class MbminirovnavTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbminirovnav'

  def testNoArgs(self):
    cmd = [self.cmd]
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      output = e.output.decode()
      self.assertEqual(1, e.returncode)
    # This might not throw an exception.

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('USBL tracking and CTD day files from the MBARI', output)
    self.assertIn('usage:', output)
    self.assertIn('--rov-dive-start=yyyymmddhhmmss', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '--help', '--verbose', '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('USBL tracking and CTD day files from the MBARI', output)
    self.assertIn('usage:', output)
    self.assertIn('--rov-dive-start=yyyymmddhhmmss', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
