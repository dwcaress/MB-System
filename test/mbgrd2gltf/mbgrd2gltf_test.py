#!/usr/bin/env python3

"""Tests for mbgrd2gltf command line app."""

import subprocess
import unittest


class Mbgrd2gltfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/mbgrd2gltf/mbgrd2gltf'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('MBgrd2gltf', output)
    self.assertIn('converts a GMT GRD format bathymetry grid file', output)
    self.assertIn('usage: mbgrd2gltf', output)


if __name__ == '__main__':
  unittest.main()
