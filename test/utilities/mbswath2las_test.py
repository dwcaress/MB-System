#!/usr/bin/env python3

"""Tests for mbswath2las command line app."""

import subprocess
import unittest


class Mbswath2lasTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbswath2las'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbswath2las', output)
    self.assertIn('MBswath2las exports swath bathymetry data', output)
    self.assertIn('usage: mbswath2las', output)

  def testNoDatalistFailsCleanly(self):
    # With no datalist present in the cwd, this should fail with a clear
    # message rather than crashing.
    cmd = [self.cmd]
    proc = subprocess.run(cmd, stdin=subprocess.DEVNULL,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertNotEqual(proc.returncode, 0)
    output = proc.stdout.decode()
    self.assertIn('Unable to open data list file', output)


if __name__ == '__main__':
  unittest.main()
