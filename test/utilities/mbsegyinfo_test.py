#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbsegyinfo command line app."""

import os
import subprocess
import unittest


class MbsegyinfoTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbsegyinfo'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to open segy file', e.output)
      self.assertIn(b'mb_segy_read_init', e.output)
      self.assertIn(b'SEGY File <> not initialized for reading', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('lists table data from a segy data file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Llonflip', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('lists table data from a segy data file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Llonflip', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('read_file:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
