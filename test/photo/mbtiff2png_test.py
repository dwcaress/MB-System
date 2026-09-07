#!/usr/bin/env python3

"""Tests for mbtiff2png command line app."""

import subprocess
import unittest


class Mbtiff2pngTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbtiff2png'

  def testNoInputFailsCleanly(self):
    # mbtiff2png does not special-case --help/-h before checking for a
    # required input file, so the smoke-test contract here is simply that
    # a missing input is reported cleanly rather than crashing.
    cmd = [self.cmd, '--help']
    proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT)
    self.assertNotEqual(proc.returncode, 0)
    output = proc.stdout.decode()
    self.assertIn('Program <mbtiff2png>', output)
    self.assertIn('Input Tiff image file not specified', output)


if __name__ == '__main__':
  unittest.main()
