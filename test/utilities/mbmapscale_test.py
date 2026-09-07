#!/usr/bin/env python3

"""Tests for mbmapscale command line app."""

import subprocess
import unittest


class MbmapscaleTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbmapscale'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('mbmapscale outputs the scaling', output)
    self.assertIn('--help', output)
    self.assertIn('--latitude=latitude', output)

  def testNoArgsUsesDefaultLatitude(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Meters per degree longitude:', output)
    self.assertIn('Meters per degree latitude:', output)


if __name__ == '__main__':
  unittest.main()
