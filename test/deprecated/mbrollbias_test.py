#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbrollbias command line app."""

import os
import subprocess
import unittest


class MbrollbiasTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbrollbias'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(12, e.returncode)
      self.assertIn(b'Grid bounds not properly specified:', e.output)
      self.assertIn(b'0.0', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('assess roll bias of swath', output)
    self.assertIn('starboard', output)
    self.assertIn('usage:', output)
    self.assertIn('-Jfile2', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('assess roll bias of swath', output)
    self.assertIn('starboard', output)
    self.assertIn('usage:', output)
    self.assertIn('-Jfile2', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('grid y dimension:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
