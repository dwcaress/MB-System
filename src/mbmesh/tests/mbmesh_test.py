#!/usr/bin/env python3

"""Smoke test for the mbmesh command line app itself (as opposed to the
algorithm-level tests alongside this file, e.g. test_settings.cpp)."""

import subprocess
import unittest


class MbmeshTest(unittest.TestCase):

  def setUp(self):
    self.cmd = './mbmesh'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('mbmesh generates 3D meshes from swath sonar bathymetry',
                   output)
    self.assertIn('usage: mbmesh', output)

  def testNoInputFailsCleanly(self):
    cmd = [self.cmd]
    proc = subprocess.run(cmd, stdin=subprocess.DEVNULL,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertNotEqual(proc.returncode, 0)
    output = proc.stdout.decode()
    self.assertIn('input datalist path is empty', output)


if __name__ == '__main__':
  unittest.main()
