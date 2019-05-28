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
      self.assertIn(b'MBIO Error allocating histogram arrays:', e.output)
      self.assertIn(b'Illegal format identifier, initialization', e.output)
      self.assertIn(b'Program <MBHISTOGRAM> Terminated', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'Version', e.output)
      self.assertIn(b'generates a histogram', e.output)
      self.assertIn(b'usage:', e.output)
      self.assertIn(b'-Nnbins', e.output)
    self.assertTrue(raised)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'Version', e.output)
      self.assertIn(b'generates a histogram', e.output)
      self.assertIn(b'usage:', e.output)
      self.assertIn(b'-Nnbins', e.output)
      self.assertIn(b'dbg2', e.output)
      self.assertIn(b'lonflip:', e.output)
      self.assertIn(b'gaussian:', e.output)
    self.assertTrue(raised)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
