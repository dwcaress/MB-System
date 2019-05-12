#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbnavlist command line app."""

import glob
import os
import subprocess
import unittest


class MbnavlistTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbnavlist'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to open data list file: datalist.mb-1', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    self.assertIn(b'mbnavlist prints', output)

  def testDoesNotExist(self):
    filename = '/does/not/exist.mb21'
    cmd = [self.cmd, '-I' + filename]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to open file', e.output)
      self.assertIn(filename.encode(), e.output)
    self.assertTrue(raised)

  def CheckNavList(self, src_filename, expected_filename):
      cmd = [self.cmd, '-I' + src_filename]
      output = subprocess.check_output(cmd)
      with open(expected_filename, 'rb') as expected_file:
        expected = expected_file.read()
      self.assertEqual(expected, output)

  def testJsonOutputStyle(self):
    for expected_filename in glob.glob('testdata/mb*/*.mbnavlist.txt'):
      tmp_filename = os.path.splitext(expected_filename)[0]
      src_filename = os.path.splitext(tmp_filename)[0]
      self.CheckNavList(src_filename, expected_filename)


if __name__ == '__main__':
  unittest.main()
