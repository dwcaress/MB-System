#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbinfo command line app."""

import glob
import json
import os
import subprocess
import unittest

class MbinfoTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbinfo'

  def CheckInfoDefault(self, src_filename, expected_filename):
    cmd = [self.cmd, '-I' + src_filename]
    testoutput = subprocess.check_output(cmd)
    self.assertIn(b'"Limits:"', testoutput)
    expected = open(expected_filename)
    expectedoutput = expected.read()
    self.assertEqual(testoutput, expectedoutput)

  def CheckInfoJson(self, src_filename, expected_filename):
    cmd = [self.cmd, '-X1', '-I' + src_filename]
    output = subprocess.check_output(cmd)
    self.assertIn(b'"limits"', output)

    summary = json.loads(output, strict=False)

    with open(expected_filename) as src:
      expected = json.load(src, strict=False)
    self.assertEqual(expected, summary)

  def testNoArgs(self):
    # Report a failure by calling exit(3).  There is no real failure.
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'initialization failed', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd)
    self.assertIn(b'basic statistics', output)

  def testDefaultOutputStyle(self):
    cmd = [self.cmd, '-Itestdata/mb21/TN136HS.309.snipped.mb21']
    output = subprocess.check_output(cmd)
    self.assertIn(b'MBIO Data Format ID:  21', output)

  def testJsonOutputStyle(self):
    for expected_filename in glob.glob('testdata/mb*/*.json'):
      src_filename = os.path.splitext(expected_filename)[0]
      self.CheckInfoJson(src_filename, expected_filename)


if __name__ == '__main__':
  unittest.main()
