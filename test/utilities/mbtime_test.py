#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbtime command line app."""

import os
import subprocess
import unittest


class MbtimeTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbtime'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('1970/01/01/00/00/00.000000', output.strip())

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('converts time values between epoch seconds', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tyear/month/day/hour/minute/second', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('converts time values between epoch seconds', output)
    self.assertIn('usage:', output)
    self.assertIn('-Tyear/month/day/hour/minute/second', output)
    self.assertIn('dbg2', output)
    self.assertIn('time_d:', output)

  def testSec0(self):
    cmd = [self.cmd, '-M', '0']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('1970/01/01/00/00/00.000000', output.strip())

  def testSec1(self):
    cmd = [self.cmd, '-M', '1.2345678']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('1970/01/01/00/00/01.234567', output.strip())

  def testSecInManPage(self):
    cmd = [self.cmd, '-M1212777434.0']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('2008/06/06/18/37/14.000000', output.strip())

  # Negative values do not work for input seconds.  e.g. -M -2000.1234

  def testDateEpoch(self):
    cmd = [self.cmd, '-T', '1970/01/01/00/00/00.000000']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('0.000000', output.strip())

  def testDateInManPage(self):
    cmd = [self.cmd, '-T2008/06/06/18/37/14.0']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('1212777434.000000', output.strip())

  def testDateNegative(self):
    cmd = [self.cmd, '-T', '1969/12/31/00/00/00.000000']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('-86400.000000', output.strip())

  # https://en.wikipedia.org/wiki/Year_2038_problem

  def testYear2028Max32(self):
    cmd = [self.cmd, '-M2147483647']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('2038/01/19/03/14/07.000000', output.strip())

  def testYear2028DoNotOverflow(self):
    cmd = [self.cmd, '-M2147483648']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('2038/01/19/03/14/08.000000', output.strip())

  def testYear2028Max32Date(self):
    cmd = [self.cmd, '-T2038/01/19/03/14/07.000000']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('2147483647.000000', output.strip())

  def testYear2028DoNotOverflowDate(self):
    cmd = [self.cmd, '-T2038/01/19/03/14/08.00000']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertEqual('2147483648.000000', output.strip())

if __name__ == '__main__':
  unittest.main()
