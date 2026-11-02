#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mblist command line app."""

import os
import subprocess
import unittest


class MblistTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mblist'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to open data list file:', e.output)
      self.assertIn(b'datalist.mb-1', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('prints the specified contents of a swath data', output)
    self.assertIn('usage:', output)
    self.assertIn('-Zsegment', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('prints the specified contents of a swath data', output)
    self.assertIn('usage:', output)
    self.assertIn('-Zsegment', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('projection_pars:', output)

  def testInvalidBeamRange(self):
    # An unparsable beam range used to leave the beam range uninitialized,
    # so mblist could loop over a random number of beams for every ping.
    for arg in ('-MC', '-M5', '-MX', '--beam-range=C', '--beam-range=X'):
      raised = False
      try:
        subprocess.check_output([self.cmd, arg], stderr=subprocess.STDOUT)
      except subprocess.CalledProcessError as e:
        raised = True
        self.assertEqual(9, e.returncode, arg)
        self.assertIn(b'Invalid beam range:', e.output, arg)
        self.assertIn(b'usage:', e.output, arg)
      self.assertTrue(raised, arg)

  def testValidBeamRange(self):
    # Valid beam ranges are accepted; mblist then stops because there is
    # no input file.
    for arg in ('-M10/20', '-MA', '-MX20', '--beam-range=10/20'):
      raised = False
      try:
        subprocess.check_output([self.cmd, arg], stderr=subprocess.STDOUT)
      except subprocess.CalledProcessError as e:
        raised = True
        self.assertEqual(2, e.returncode, arg)
        self.assertNotIn(b'Invalid beam range:', e.output, arg)
        self.assertIn(b'Unable to open data list file:', e.output, arg)
      self.assertTrue(raised, arg)

  def testInvalidPixelRange(self):
    # Same as testInvalidBeamRange for the sidescan pixel range.
    for arg in ('-NC', '-N5', '--pixel-range=C'):
      raised = False
      try:
        subprocess.check_output([self.cmd, arg], stderr=subprocess.STDOUT)
      except subprocess.CalledProcessError as e:
        raised = True
        self.assertEqual(9, e.returncode, arg)
        self.assertIn(b'Invalid pixel range:', e.output, arg)
        self.assertIn(b'usage:', e.output, arg)
      self.assertTrue(raised, arg)

  def testValidPixelRange(self):
    for arg in ('-N10/20', '-NA', '--pixel-range=10/20'):
      raised = False
      try:
        subprocess.check_output([self.cmd, arg], stderr=subprocess.STDOUT)
      except subprocess.CalledProcessError as e:
        raised = True
        self.assertEqual(2, e.returncode, arg)
        self.assertNotIn(b'Invalid pixel range:', e.output, arg)
        self.assertIn(b'Unable to open data list file:', e.output, arg)
      self.assertTrue(raised, arg)

  # TODO(schwehr): Add tests of actual usage.


if __name__ == '__main__':
  unittest.main()
