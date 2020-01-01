#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbinsreprocess command line app."""

import os
import subprocess
import unittest


class MbinsreprocessTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbinsreprocess'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(1, e.returncode)
      self.assertIn(b'Unable to open log file <stdin> for reading', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('reads an INS navigation file', output)
    self.assertIn('usage:', output)
    self.assertIn('--output=filename', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '--help', '--verbose', '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('reads an INS navigation file', output)
    self.assertIn('usage:', output)
    self.assertIn('--output=filename', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('ofile:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
