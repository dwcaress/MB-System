#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbareaclean command line app."""

import os
import shutil
import subprocess
import tempfile
import unittest


class MbAreaCleanTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbareaclean'

  def testNoArgs(self):
    cmd = [self.cmd]
    subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('usage:', output)
    self.assertIn('minbeam[/maxbeam]', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('usage:', output)
    self.assertIn('minbeam[/maxbeam]', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('median_filter', output)

  def testKeepsExistingEdits(self):
    # mbareaclean must keep the edits already in the edit save file, also
    # for soundings it does not evaluate (here: outside the --bounds area).
    # It used to rewrite the file with only the edits of the soundings
    # stored in its grid, deleting all the others.
    cmd_dir = os.path.dirname(os.path.abspath(self.cmd))
    testdata = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testdata', 'mb54')
    swath = 'TN136HS.309.snipped.mb54'
    env = dict(os.environ)
    env['PATH'] = cmd_dir + os.pathsep + env.get('PATH', '')

    def edits(tmpdir):
      output = subprocess.check_output(
          [os.path.join(cmd_dir, 'mbdumpesf'), '--input=' + swath + '.esf'],
          stderr=subprocess.DEVNULL, cwd=tmpdir, env=env).decode()
      # keep time, beam and action, not the record index
      return [line.split('time:', 1)[1] for line in output.splitlines() if line.startswith('EDITS READ:')]

    with tempfile.TemporaryDirectory() as tmpdir:
      shutil.copy(os.path.join(testdata, swath), tmpdir)
      subprocess.check_output(
          [os.path.join(cmd_dir, 'mbclean'), '--input=' + swath, '--format=54', '--depth-range=455/470'],
          stderr=subprocess.STDOUT, cwd=tmpdir, env=env)
      before = edits(tmpdir)
      self.assertGreater(len(before), 0)

      with open(os.path.join(tmpdir, 'datalist.mb-1'), 'w') as datalist:
        datalist.write(swath + ' 54\n')
      subprocess.check_output(
          [os.path.abspath(self.cmd), '--input=datalist.mb-1', '--bin-size=20', '--median-filter=0.5/2',
           '--flag-bad', '--bounds=-124.51/-124.5015/40.83/40.85'],
          stderr=subprocess.STDOUT, cwd=tmpdir, env=env)
      after = edits(tmpdir)
      for edit in before:
        self.assertIn(edit, after)

  # TODO(schwehr): Add more tests of actual usage.


if __name__ == '__main__':
  unittest.main()
