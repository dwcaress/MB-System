#!/usr/bin/env python3

"""Tests for mbmakedatalist command line app."""

import subprocess
import unittest


class MbmakedatalistTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbmakedatalist'

  def testHelp(self):
    # Note: mbmakedatalist scans the current directory for swath files by
    # default, so this test deliberately only exercises -h (no side effects)
    # rather than a bare invocation.
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('mbmakedatalist:', output)
    self.assertIn('Macro to generate an MB-System datalist file', output)


if __name__ == '__main__':
  unittest.main()
