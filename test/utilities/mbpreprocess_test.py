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

  def testKongsbergHeadingWithoutPlatform(self):
    # Kongsberg EM122 .all data (format 58) preprocessed without a platform
    # file must keep the recorded heading in the format 59 output and in the
    # .fbt file. mbsys_simrad3_preprocess() used to add uninitialized platform
    # mount offsets, giving heading 0.000 in the .mb59 file and -21474836 in
    # the .fbt file instead of about 8.3 degrees. The test data are two pings
    # of a Sea Acceptance Test line of RV METEOR, 19 December 2011.
    # Uninitialized stack values change with the stack layout, and the old
    # code gave the correct heading by chance in some runs, so mbpreprocess
    # is run with a range of environment sizes, each of which must pass.
    cmd_dir = os.path.dirname(os.path.abspath(self.cmd))
    testdata = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testdata', 'mb58')
    src_name = '0008_20111219_210356_METEOR_EM122.snipped.all'
    env = dict(os.environ)
    env['PATH'] = cmd_dir + os.pathsep + env.get('PATH', '')

    def headings(tmpdir, filename, iformat):
      cmd = [os.path.join(cmd_dir, 'mblist'), '-I' + filename, '-F%d' % iformat, '-Q', '-OH']
      output = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, cwd=tmpdir, env=env).decode()
      return [float(value) for value in output.split()]

    with tempfile.TemporaryDirectory() as tmpdir:
      shutil.copy(os.path.join(testdata, src_name), tmpdir)
      ofile = os.path.splitext(src_name)[0] + '.mb59'
      raw = headings(tmpdir, src_name, 58)
      self.assertEqual(2, len(raw))
      for value in raw:
        self.assertAlmostEqual(8.27, value, delta=0.05)

      cmd = [os.path.abspath(self.cmd), '--input=' + src_name, '--format=58']
      for pad in range(0, 256, 8):
        run_env = dict(env)
        run_env['MB_TEST_STACK_PAD'] = 'x' * pad
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmpdir, env=run_env)
        for filename, iformat in ((ofile, 59), (ofile + '.fbt', 71)):
          preprocessed = headings(tmpdir, filename, iformat)
          self.assertEqual(len(raw), len(preprocessed), filename)
          for raw_value, value in zip(raw, preprocessed):
            self.assertAlmostEqual(raw_value, value, delta=0.1,
                                   msg='%s (environment padding %d)' % (filename, pad))


if __name__ == '__main__':
  unittest.main()
