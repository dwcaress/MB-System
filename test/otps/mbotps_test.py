#!/usr/bin/env python3

"""Tests for mbotps command line app."""

import subprocess
import unittest


class MbotpsTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/otps/mbotps'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbotps', output)
    self.assertIn('MBotps predicts tides', output)
    self.assertIn('usage: mbotps', output)
    self.assertIn('--input=datalist', output)


if __name__ == '__main__':
  unittest.main()
