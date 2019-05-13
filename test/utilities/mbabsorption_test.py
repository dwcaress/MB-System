#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbabsorption command line app.

Absorption is always in dB/km.
"""

import os
import re
import subprocess
import unittest


class MbAbsorptionTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbabsorption'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 54.446199
    self.assertAlmostEqual(absorption, expected)

  def testHelp(self):
    cmd = [self.cmd, '-H']
    output = subprocess.check_output(cmd).decode().rstrip()
    self.assertIn('sea water', output)
    self.assertIn('soundspeed', output)

  def testVerbose(self):
    cmd = [self.cmd, '-V']
    output = subprocess.check_output(cmd).decode().rstrip()
    self.assertIn('Input Parameters:', output)
    self.assertIn('200.000000 kHz', output)
    self.assertIn('10.000000 deg C', output)
    self.assertIn('35.000000 per mil', output)
    self.assertIn('0.000000 m', output)
    self.assertIn('8.0', output)  # pH

  def testSoundSpeed(self):
    cmd = [self.cmd, '-C1500.0']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 54.047064
    self.assertAlmostEqual(absorption, expected)

  def testDepth(self):
    cmd = [self.cmd, '-D4000.0']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 32.836398
    self.assertAlmostEqual(absorption, expected)

  def testFrequency(self):
    cmd = [self.cmd, '-F31.23']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 7.354640
    self.assertAlmostEqual(absorption, expected)

  def testPh(self):
    cmd = [self.cmd, '-P7.8']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 54.411284
    self.assertAlmostEqual(absorption, expected)

  def testSalinity(self):
    cmd = [self.cmd, '-S15.000012']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 31.259585
    self.assertAlmostEqual(absorption, expected)

  def testTemperature(self):
    cmd = [self.cmd, '-T0.2']
    output = subprocess.check_output(cmd)
    absorption = float(output)
    expected = 40.702235
    self.assertAlmostEqual(absorption, expected)

# TODO(schwehr): How to handle invalid arguments?

if __name__ == '__main__':
  unittest.main()
