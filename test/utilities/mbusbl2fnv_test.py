#!/usr/bin/env python3

"""Tests for mbusbl2fnv command line app."""

import subprocess
import unittest


class Mbusbl2fnvTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbusbl2fnv'

  def testHelp(self):
    cmd = [self.cmd, '-H']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbusbl2fnv', output)
    self.assertIn('Usage:', output)
    self.assertIn('-I usbl.csv', output)

  def testNoArgsEmitsFnvHeader(self):
    # With no input file it reads stdin; closing stdin immediately still
    # lets it print the fnv column header before exiting.
    cmd = [self.cmd]
    output = subprocess.check_output(
        cmd, stdin=subprocess.DEVNULL, stderr=subprocess.STDOUT).decode()
    self.assertIn('<longitude (deg)>', output)
    self.assertIn('<latitude (deg)>', output)


if __name__ == '__main__':
  unittest.main()
