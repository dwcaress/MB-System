#!/usr/bin/env python3

"""Tests for mbphotomosaic command line app."""

import subprocess
import unittest


class MbphotomosaicTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbphotomosaic'

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbphotomosaic', output)
    self.assertIn('makes a mosaic of navigated downlooking photographs',
                   output)
    self.assertIn('usage: mbphotomosaic', output)
    self.assertIn('--threads=nthreads', output)


if __name__ == '__main__':
  unittest.main()
