#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbsvpselect command line app."""

import os
import subprocess
import unittest


class MbsvpselectTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbsvpselect'

  # testNoArgs needs input.

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('chooses and implements the best available sound', output)
    self.assertIn('usage:', output)
    self.assertIn('-P3/range/1', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('chooses and implements the best available sound', output)
    self.assertIn('usage:', output)
    self.assertIn('-P3/range/1', output)
    self.assertIn('dbg2', output)
    self.assertIn('svplist:', output)
    self.assertIn('zero_test:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
