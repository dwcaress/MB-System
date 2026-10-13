#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbpreprocess command line app."""

import os
import shutil
import subprocess
import tempfile
import unittest


class MbpreprocessTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbpreprocess'

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(3, e.returncode)
      self.assertIn(b'MBIO Error returned from function', e.output)
      self.assertIn(b'Illegal format identifier', e.output)
      self.assertIn(b'Multibeam File <> not initialized for reading', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '--help']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('handles preprocessing of swath sonar data', output)
    self.assertIn('--platform-file=platform_file', output)
    self.assertIn('--kluge-fix-wissl-timestamps', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '--help', '--verbose', '--verbose']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('handles preprocessing of swath sonar data', output)
    self.assertIn('--platform-file=platform_file', output)
    self.assertIn('--kluge-fix-wissl-timestamps', output)

  def CheckOldSimradToFormat57(self, src_dir, src_name, iformat, records, good_beams):
    # Old Simrad formats (51, 53, 54) are written by mbpreprocess as format 57;
    # check that the conversion runs and that the output reads back fully
    # (see github issue #1626)
    cmd_dir = os.path.dirname(os.path.abspath(self.cmd))
    testdata = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testdata', src_dir)
    env = dict(os.environ)
    env['PATH'] = cmd_dir + os.pathsep + env.get('PATH', '')
    with tempfile.TemporaryDirectory() as tmpdir:
      shutil.copy(os.path.join(testdata, src_name), tmpdir)
      cmd = [os.path.abspath(self.cmd), '--input=' + src_name, '--format=%d' % iformat]
      subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmpdir, env=env)

      ofile = os.path.splitext(src_name)[0] + '.mb57'
      self.assertTrue(os.path.exists(os.path.join(tmpdir, ofile)))
      cmd = [os.path.join(cmd_dir, 'mbinfo'), '-F57', '-I' + ofile]
      output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmpdir, env=env).decode()
      self.assertRegex(output, r'Number of Records: +%d\n' % records)
      self.assertRegex(output, r'Number of Good Beams: +%d ' % good_beams)

  def testOldSimradFormat51(self):
    self.CheckOldSimradToFormat57('mb51', 'tahoe_98_134.0.em1000.mb51', 51, 2, 120)

  def testOldSimradFormat54(self):
    self.CheckOldSimradToFormat57('mb54', 'TN136HS.309.snipped.mb54', 54, 2, 115)


if __name__ == '__main__':
  unittest.main()
