#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbconfig command line app."""

import os
import re
import subprocess
import unittest

# e.g. 5.7.5beta9
VERSION_REGEX = r'[0-9]+[.][0-9]+[.][0-9]+((alpha|beta)[0-9]+)?'


class MbconfigTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbconfig'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd).decode().rstrip()
    # regex 5.7.5beta9
    self.assertRegex(output, VERSION_REGEX)

  def testVerboseOnly(self):
    cmd = [self.cmd, '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('MB-System version:', output)
    self.assertRegex(output, VERSION_REGEX)

  def testJunk(self):
    cmd = [self.cmd, '--garbage-opt']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(9, e.returncode)
      self.assertIn(b'usage', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '--verbose', '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('flags', output)
    self.assertIn('levitus database', output)
    self.assertIn('tidal correction', output)

  def testPrefix(self):
    cmd = [self.cmd, '--verbose', '--prefix']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('MB-System install prefix:', output)

  def testCflags(self):
    cmd = [self.cmd, '--verbose', '--cflags']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('MB-System compile flags:', output)
    # TODO(schwehr): -INONE/include is wrong

  def testLibs(self):
    cmd = [self.cmd, '--verbose', '--libs']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('MB-System link flags:', output)
    for lib in ('mbaux', 'mbsapi', 'mbbsio', 'mbview', 'mbgsf', 'mbxgr', 'mbio'):
      self.assertIn(' -l%s.la' % lib, output)
    # TODO(schwehr): -LNONE/include is wrong

  def testVersionId(self):
    cmd = [self.cmd, '--verbose', '--version-id']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().rstrip()
    self.assertIn('MB-System version id:', output)
    # e.g. 50700005
    version_id_regex = r'[0-9]{8}'

    self.assertRegex(output, version_id_regex)

    match = re.search(version_id_regex, output)
    version_id = int(match.group(0))
    self.assertGreater(version_id, 50700000)

  def testVersionMajor(self):
    cmd = [self.cmd, '--version-major']
    output = subprocess.check_output(cmd)
    major = int(output)
    self.assertGreaterEqual(major, 5)

  def testVersionMinor(self):
    cmd = [self.cmd, '--version-minor']
    output = subprocess.check_output(cmd)
    minor = int(output)
    self.assertGreaterEqual(minor, 0)

  def testVersionArchive(self):
    # a.k.a. minor minor
    cmd = [self.cmd, '--version-archive']
    output = subprocess.check_output(cmd)
    archive = int(output)
    self.assertGreaterEqual(archive, 0)

  def testLevitus(self):
    cmd = [self.cmd, '--levitus']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('LevitusAnnual82', output)

  def testOtps(self):
    cmd = [self.cmd, '--otps']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('otps', output.lower())
    # TODO(schwehr): Why if built without otps?


if __name__ == '__main__':
  unittest.main()
