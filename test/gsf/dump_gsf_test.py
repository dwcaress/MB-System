#!/usr/bin/env python3

"""Tests for dump_gsf command line app."""

import subprocess
import unittest


class DumpGsfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/gsf/dump_gsf'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Usage:', output)
    self.assertIn('-f <gsf filename>', output)


if __name__ == '__main__':
  unittest.main()
