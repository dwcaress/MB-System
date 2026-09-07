#!/usr/bin/env python3

"""Tests for mbgetphotocorrection command line app."""

import subprocess
import unittest


class MbgetphotocorrectionTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbgetphotocorrection'

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbgetphotocorrection', output)
    self.assertIn('usage: mbgetphotocorrection', output)
    self.assertIn('--threads=nthreads', output)


if __name__ == '__main__':
  unittest.main()
