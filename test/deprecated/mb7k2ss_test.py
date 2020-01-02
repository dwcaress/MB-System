#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mb7k2ss command line app."""

import os
import subprocess
import unittest


class Mb7k2ssTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mb7k2ss'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Low Sidescan', e.output)
      self.assertIn(b'Unable to open data list file:', e.output)
      self.assertIn(b'datalist.mb-1', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(1, e.returncode)
      self.assertIn(b'mb7k2ss extracts sidescan sonar', e.output)
      self.assertIn(b'MBF_MBLDEOIH', e.output)
    self.assertTrue(raised)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(1, e.returncode)
      self.assertIn(b'mb7k2ss extracts sidescan sonar', e.output)
      self.assertIn(b'dbg2', e.output)
      self.assertIn(b'lonflip', e.output)
    self.assertTrue(raised)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
