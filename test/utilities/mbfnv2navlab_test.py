#!/usr/bin/env python3

"""Tests for mbfnv2navlab command line app."""

import subprocess
import unittest


class Mbfnv2navlabTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbfnv2navlab'

  def testHelp(self):
    cmd = [self.cmd, '-H']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbfnv2navlab', output)
    self.assertIn('Usage:', output)
    self.assertIn('-I input.fnv', output)


if __name__ == '__main__':
  unittest.main()
