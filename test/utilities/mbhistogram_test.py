#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbhistogram command line app."""

import os
import subprocess
import unittest


class MbhistogramTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbhistogram'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      output = e.output.decode()
      self.assertIn('MBIO Error allocating histogram arrays:', output)
      self.assertIn('Illegal format identifier, initialization', output)
      self.assertIn('Program <MBHISTOGRAM> Terminated', output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('generates a histogram', output)
    self.assertIn('usage:', output)
    self.assertIn('-Nnbins', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('generates a histogram', output)
    self.assertIn('usage:', output)
    self.assertIn('-Nnbins', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
