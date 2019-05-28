#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbprocess command line app."""

import os
import subprocess
import unittest


class MbprocessTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbprocess'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Program <mbprocess> requires an input data', e.output)
      self.assertIn(b'specified with the -I option.', e.output)
      self.assertIn(b'The default input file is "datalist.mb-1".', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('processing swath sonar bathymetry', output)
    self.assertIn('mbedit edit save files', output)
    self.assertIn('usage:', output)
    self.assertIn('-S', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('processing swath sonar bathymetry', output)
    self.assertIn('mbedit edit save files', output)
    self.assertIn('usage:', output)
    self.assertIn('-S', output)
    # No dbg2 info.

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
