#!/usr/bin/env python3

"""Tests for mbimagelist command line app."""

import subprocess
import unittest


class MbimagelistTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbimagelist'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Program mbimagelist', output)
    self.assertIn('parses recursive imagelist files', output)
    self.assertIn('usage: mbimagelist', output)


if __name__ == '__main__':
  unittest.main()
