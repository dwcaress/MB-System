#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbgetesf command line app."""

import os
import subprocess
import unittest


class MbgetesfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbgetesf'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'MBIO Error returned from function', e.output)
      self.assertIn(b'Illegal format identifier', e.output)
      self.assertIn(b'Multibeam File <stdin> not initialized', e.output)
      self.assertIn(b'Program <mbgetesf> Terminated', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('reads a multibeam data file and writes', output)
    self.assertIn('used to apply the edit events to another file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Oesffile', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('reads a multibeam data file and writes', output)
    self.assertIn('used to apply the edit events to another file', output)
    self.assertIn('usage:', output)
    self.assertIn('-Oesffile', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('kluge:', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
