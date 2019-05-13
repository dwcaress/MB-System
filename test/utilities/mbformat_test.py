#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbformat command line app."""

import subprocess
import unittest


class MbFormatTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbformat'

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd).decode()
    self.assertIn('Supported MBIO Formats:', output)
    self.assertIn('MBIO Data Format ID:  11', output)
    self.assertIn('MBIO Data Format ID:  261', output)
    self.assertIn('Format name:          MBF_KEMKMALL', output)
    self.assertIn('Informal Description: Kongsberg', output)

  def testHtml(self):
    cmd = [self.cmd, '-W']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('<TITLE>MB-System Supported Data Formats</TITLE>', output)
    self.assertIn('<LI>MBIO Data Format ID:  11 </LI>', output)
    self.assertIn('<HTML>', output)
    self.assertIn('<BODY', output)
    self.assertIn('<UL>', output)

  def testVerboseOnly(self):
    cmd = [self.cmd, '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('MB-system Version', output)

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
    cmd = [self.cmd, '-H']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies the swath data formats', output)
    self.assertIn('usage:', output)

  def testFormat(self):
    cmd = [self.cmd, '-F51']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('MBF_EMOLDRAW', output)
    self.assertIn('EM121', output)
    self.assertNotIn('MBF_EM12IFRM', output)

  def testInputFile(self):
    cmd = [self.cmd, '-Itestdata/mb21/TN136HS.309.snipped.mb21']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('MBF_HSATLRAW', output)
    self.assertIn('Hydrosweep', output)
    self.assertNotIn('MBF_EM12IFRM', output)

  def testInputFileWithFileRoot(self):
    cmd = [self.cmd, '-K', '-Itestdata/mb173/NBP0209.snipped.a77']
    output = subprocess.check_output(cmd).decode().rstrip()
    self.assertEqual('testdata/mb173/NBP0209.snipped 173', output)

  def testInputFileIgnoreFormat(self):
    cmd = [self.cmd, '-F173', '-Itestdata/mb21/TN136HS.309.snipped.mb21']
    output = subprocess.check_output(cmd).decode()
    self.assertIn('MBF_HSATLRAW', output)
    self.assertIn('Hydrosweep', output)
    self.assertNotIn('MBF_EM12IFRM', output)

  def testInputFileWithOnlyIdNumber(self):
    cmd = [self.cmd, '-L', '-Itestdata/mb173/NBP0209.snipped.a77']
    output = subprocess.check_output(cmd).decode().rstrip()
    format_id = int(output)
    expected = 173
    self.assertEqual(expected, format_id)

  def testDoesNotExist(self):
    cmd = [self.cmd, '-I/does/not/exist']
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'unable to infer format from filename', e.output)
    self.assertTrue(raised)

if __name__ == '__main__':
  unittest.main()
