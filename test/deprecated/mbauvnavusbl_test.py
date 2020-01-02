#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbauvnavusbl command line app."""

import os
import subprocess
import unittest


class MbauvnavusblTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbauvnavusbl'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to Open Navigation File <stdin> for', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('USBL fixes', output)
    self.assertIn('reads a primary navigation file', output)
    self.assertIn('-Uusblfile', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('USBL fixes', output)
    self.assertIn('reads a primary navigation file', output)
    self.assertIn('-Uusblfile', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('usbl format:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
