#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbmakeplatform command line app."""

import os
import subprocess
import unittest


class MbmakeplatformTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbmakeplatform'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('', output.strip())

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('creates or modifies an MB-System platform file', output)
    self.assertIn('usage:', output)
    self.assertIn('--input=plffile', output)
    self.assertIn('--modify-time-latency-model=file', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '--help', '-verbose', '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('creates or modifies an MB-System platform file', output)
    self.assertIn('usage:', output)
    self.assertIn('--input=plffile', output)
    self.assertIn('--modify-time-latency-model=file', output)
    # No extra dbg2 info.

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
