#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbswplspreprocess command line app."""

import os
import subprocess
import unittest


class MbswplspreprocessTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/deprecated/mbswplspreprocess'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'MBIO Error returned from function', e.output)
      self.assertIn(b'mb_datalist_open', e.output)
      self.assertIn(b'Unable to open data list file: datalist.mb-1', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('Preprocess SWATHplus SXP formatted files', output)
    self.assertIn('-Obasename', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('Preprocess SWATHplus SXP formatted files', output)
    self.assertIn('-Obasename', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('copy_rawamp:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
