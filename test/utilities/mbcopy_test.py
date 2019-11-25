#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbcopy command line app."""

import os
import subprocess
import unittest


class MbcopyTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbcopy'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'MBIO Error returned from function <mb_format>', e.output)
      self.assertIn(b'initialization failed', e.output)

    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('copies an input swath sonar data file to an output', output)
    self.assertIn('-Qsleep_factor', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('copies an input swath sonar data file to an output', output)
    self.assertIn('-Qsleep_factor', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('fbtversion:', output)

  def testTooManyStripModes(self):
    cmd = [self.cmd, '-n', '-n', '-n']
    try:
      output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
      self.assertFalse(output)  # Should always fail if it gets here.
    except subprocess.CalledProcessError as e:
      self.assertEqual(9, e.returncode)
      output = e.output.decode()
      self.assertIn('Gave -n more than twice.', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
