#!/usr/bin/env python3

"""Tests for mbnavadjustmerge command line app."""

import subprocess
import unittest


class MbnavadjustmergeTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/mbnavadjust/mbnavadjustmerge'

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbnavadjustmerge', output)
    self.assertIn('merges two existing mbnavadjust projects', output)
    self.assertIn('usage: mbnavadjustmerge --input=project_path', output)


if __name__ == '__main__':
  unittest.main()
