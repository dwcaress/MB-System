#!/usr/bin/env python3

"""Tests for mbphotogrammetry command line app."""

import subprocess
import unittest


class MbphotogrammetryTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbphotogrammetry'

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbphotogrammetry', output)
    self.assertIn(
        'generates bathymetry from stereo pairs of photographs', output)
    self.assertIn('usage: mbphotogrammetry', output)
    self.assertIn('--threads=nthreads', output)


if __name__ == '__main__':
  unittest.main()
