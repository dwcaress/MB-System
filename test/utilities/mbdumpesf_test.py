#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbdumpesf command line app."""

import os
import subprocess
import unittest


class MbdumpesfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbdumpesf'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('', output.strip())

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('edit save file and dumps', output)
    self.assertIn('as an ascii table to stdout', output)
    self.assertIn('usage:', output)
    self.assertIn('--ignore-unflag', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('edit save file and dumps', output)
    self.assertIn('as an ascii table to stdout', output)
    self.assertIn('usage:', output)
    self.assertIn('--ignore-unflag', output)
    # -V -V does not do anything.
    self.assertNotIn('dbg2', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
