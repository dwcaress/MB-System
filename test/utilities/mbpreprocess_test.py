#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbpreprocess command line app."""

import os
import subprocess
import unittest


class MbpreprocessTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbpreprocess'

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
      self.assertIn(b'Multibeam File <> not initialized for reading', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('handles preprocessing of swath sonar data', output)
    self.assertIn('--platform-file=platform_file', output)
    self.assertIn('--kluge-fix-wissl-timestamps', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '--help', '--verbose', '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('handles preprocessing of swath sonar data', output)
    self.assertIn('--platform-file=platform_file', output)
    self.assertIn('--kluge-fix-wissl-timestamps', output)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
