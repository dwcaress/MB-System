#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbhsdump command line app."""

import os
import subprocess
import unittest


class MbhsdumpTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbhsdump'

  # Hangs: testNoArgs

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('information contained in data records', output)
    self.assertIn('usage:', output)
    self.assertIn('-Okind', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('information contained in data records', output)
    self.assertIn('usage:', output)
    self.assertIn('-Okind', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('mb_data_velocity_profile_list:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
