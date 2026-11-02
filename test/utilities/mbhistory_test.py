#!/usr/bin/env python3

# See README file for copying and redistribution conditions.

"""Tests for mbhistory command line app.

The tests make synthetic edit save (*.esf) files next to an empty
placeholder swath file, so no sonar data are needed.
"""

import glob
import os
import shutil
import struct
import subprocess
import tempfile
import unittest
import zlib

MB_PATH_MAXLINE = 1024
ACTION_FLAG = 1
ACTION_UNFLAG = 2


def write_esf(path, edits):
  """Write a version 3 edit save file (big endian) with (time, beam, action) edits."""
  header = b'ESFVERSION03\nESF Mode: 0\nProgram: mbhistory_test\n'
  with open(path, 'wb') as esf:
    esf.write(header.ljust(MB_PATH_MAXLINE, b'\0'))
    for time_d, beam, action in edits:
      esf.write(struct.pack('>dii', time_d, beam, action))


def crc32(path):
  with open(path, 'rb') as f:
    return zlib.crc32(f.read()) & 0xffffffff


class MbhistoryTest(unittest.TestCase):

  def setUp(self):
    self.cmd = os.path.abspath('../../src/utilities/mbhistory')
    self.tmpdir = tempfile.mkdtemp(prefix='mbhistory_test_')
    self.swath = os.path.join(self.tmpdir, 'line.mb88')
    # placeholder swath file; datalists skip empty files
    with open(self.swath, 'w') as f:
      f.write('placeholder\n')
    self.esf = self.swath + '.esf'
    self.history = os.path.join(self.tmpdir, 'mbhistory')

  def tearDown(self):
    shutil.rmtree(self.tmpdir)

  def run_mbhistory(self, *args):
    cmd = [self.cmd, '--input=' + self.swath] + list(args)
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return result.returncode, result.stdout.decode()

  def manifest(self):
    with open(os.path.join(self.history, 'line.mb88.esf.manifest')) as f:
      return [line.rstrip('\n').split('\t') for line in f if not line.startswith('#')]

  def make_versions(self):
    """Make versions 1-3 with 1, 2 and 3 edits; the current file has 4 edits."""
    edits = []
    for i in range(3):
      edits.append((1761000000.0 + i, 10 + i, ACTION_FLAG))
      write_esf(self.esf, edits)
      code, output = self.run_mbhistory('--tag=t%d' % (i + 1))
      self.assertEqual(0, code, output)
    edits.append((1761000003.0, 20, ACTION_FLAG))
    write_esf(self.esf, edits)

  def testNoArgs(self):
    result = subprocess.run([self.cmd], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(9, result.returncode)
    output = result.stdout.decode()
    self.assertIn('No input swath file or datalist specified', output)
    self.assertIn('usage:', output)

  def testHelp(self):
    output = subprocess.check_output([self.cmd, '-h'], stderr=subprocess.STDOUT).decode()
    self.assertIn('restores the versions', output)
    self.assertIn('usage:', output)
    self.assertIn('--restore-date', output)
    self.assertIn('--prune', output)

  def testBadUsage(self):
    code, output = self.run_mbhistory('--list', '--prune', '--keep=2')
    self.assertEqual(9, code)
    self.assertIn('Only one of', output)
    code, output = self.run_mbhistory('--prune')
    self.assertEqual(9, code)
    self.assertIn('--prune needs --keep', output)
    code, output = self.run_mbhistory('--restore-date=yesterday')
    self.assertEqual(9, code)
    self.assertIn('Bad date', output)

  def testListNoHistory(self):
    code, output = self.run_mbhistory()
    self.assertEqual(0, code)
    self.assertIn('no history versions', output)

  def testTagAndList(self):
    self.make_versions()
    snapshots = sorted(glob.glob(os.path.join(self.history, 'line.mb88.esf.v*')))
    self.assertEqual(3, len(snapshots))
    self.assertRegex(os.path.basename(snapshots[0]), r'^line\.mb88\.esf\.v0001_\d{8}T\d{6}$')

    entries = self.manifest()
    self.assertEqual(['1', '2', '3'], [e[0] for e in entries])
    self.assertEqual('mbhistory', entries[0][3])
    self.assertEqual('%8.8x' % crc32(snapshots[2]), entries[2][7])

    code, output = self.run_mbhistory('--list')
    self.assertEqual(0, code)
    self.assertIn('[t1]', output)
    self.assertNotIn('[t1] tag t1', output)
    self.assertRegex(output, r'now .*\(current\)\s+4')

    # tagging an unchanged file reuses the newest version
    write_esf(self.esf, [(1761000000.0 + i, 10 + i, ACTION_FLAG) for i in range(3)])
    code, output = self.run_mbhistory('--tag=again')
    self.assertIn('tagged version 3 as again', output)

    # a tag name is not reused unless forced
    code, output = self.run_mbhistory('--tag=t1')
    self.assertNotEqual(0, code)
    self.assertIn('already set', output)

  def testShowAndDiff(self):
    self.make_versions()
    code, output = self.run_mbhistory('--show=2')
    self.assertEqual(0, code)
    self.assertIn('Edit events:    2 (flag 2', output)
    self.assertIn('ESFVERSION03', output)

    code, output = self.run_mbhistory('--diff=1')
    self.assertEqual(0, code)
    self.assertIn('from v1 to now', output)
    self.assertIn('3 beams changed: 3 now flagged', output)

    code, output = self.run_mbhistory('--diff=now:1')
    self.assertIn('3 edits removed', output)

    code, output = self.run_mbhistory('--diff=2:2')
    self.assertIn('none', output)

    code, output = self.run_mbhistory('--diff=9')
    self.assertNotEqual(0, code)
    self.assertIn('version 9 not found', output)

  def testDiffUsesFinalEdit(self):
    # flag then unflag the same beam: the final edit is an unflag
    write_esf(self.esf, [(1761000000.0, 5, ACTION_FLAG)])
    self.run_mbhistory('--tag=a')
    write_esf(self.esf, [(1761000000.0, 5, ACTION_FLAG), (1761000000.0, 5, ACTION_UNFLAG)])
    code, output = self.run_mbhistory('--diff=1')
    self.assertIn('1 beams changed: 0 now flagged, 1 now unflagged', output)

  def testRestoreKeepsLaterVersions(self):
    self.make_versions()
    before = crc32(self.esf)

    # without --yes a restore is a dry run
    code, output = self.run_mbhistory('--restore=1')
    self.assertEqual(0, code)
    self.assertIn('would restore version 1', output)
    self.assertIn('Add --yes to apply', output)
    self.assertEqual(before, crc32(self.esf))

    code, output = self.run_mbhistory('--restore=1', '--yes')
    self.assertEqual(0, code, output)
    self.assertIn('current edits saved as version 4', output)
    self.assertIn('restored version 1', output)
    self.assertFalse(os.path.exists(self.swath + '.lck'))

    entries = self.manifest()
    self.assertEqual(['1', '2', '3', '4'], [e[0] for e in entries])
    self.assertEqual('restore (v4 -> v1)', entries[3][9])
    self.assertEqual('%8.8x' % before, entries[3][7])
    self.assertEqual(entries[0][7], '%8.8x' % crc32(self.esf))

    code, output = self.run_mbhistory('--list')
    self.assertIn('restored from v1', output)

    # the restore can be undone
    code, output = self.run_mbhistory('--restore=4', '--yes')
    self.assertEqual(0, code, output)
    self.assertEqual(before, crc32(self.esf))
    self.assertEqual(5, len(self.manifest()))

    # restoring the current state changes nothing
    code, output = self.run_mbhistory('--restore=4', '--yes')
    self.assertIn('already match version 4', output)
    self.assertEqual(5, len(self.manifest()))

  def testRestoreLockedFile(self):
    self.make_versions()
    with open(self.swath + '.lck', 'w') as lock:
      lock.write('Locking Program: mbeditviz\n')
    code, output = self.run_mbhistory('--restore=1', '--yes')
    self.assertNotEqual(0, code)
    self.assertIn('file is locked', output)
    self.assertEqual(3, len(self.manifest()))

  def testRestoreTagAndDate(self):
    self.make_versions()
    code, output = self.run_mbhistory('--restore-tag=t2', '--yes')
    self.assertEqual(0, code, output)
    self.assertIn('restored version 2', output)

    code, output = self.run_mbhistory('--restore-tag=nosuchtag', '--yes')
    self.assertIn('tag nosuchtag not set', output)

    # every snapshot was taken after 2000, so the first one holds that state
    code, output = self.run_mbhistory('--restore-date=2000-01-01', '--yes')
    self.assertIn('restored version 1', output)

    # no snapshot after a future date: the current file is that state
    code, output = self.run_mbhistory('--restore-date=2999-12-31T23:59')
    self.assertIn('no change since that date', output)

  def testRestoreDateBeforeHistoryWarns(self):
    # a history without a "created" version does not know the edits made
    # before its oldest version
    self.make_versions()
    code, output = self.run_mbhistory('--restore-date=2000-01-01')
    self.assertEqual(0, code, output)
    self.assertIn('would restore version 1', output)
    self.assertIn('edits made before', output)
    self.assertIn('version 1 is the oldest known state', output)

  def testCreatedVersionWithMbclean(self):
    # mbclean creates the esf file: version 1 holds no edits and has the note
    # "created", so restoring a date before the first edits removes them all.
    # Snapshots are saved before each run: runs 2 and 3 save versions 2 and 3.
    # Run 3 adds no edits, so run 4 finds the same edits as version 3 (only
    # the esf header was rewritten) and saves no new version.
    mbclean = os.path.abspath('../../src/utilities/mbclean')
    testdata = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'testdata', 'mb54')
    swath = os.path.join(self.tmpdir, 'TN136HS.309.snipped.mb54')
    shutil.copy(os.path.join(testdata, 'TN136HS.309.snipped.mb54'), swath)
    for depth_range in ('0/460', '0/455', '0/455', '0/455'):
      subprocess.run([mbclean, '-I' + swath, '-F54', '--depth-range=' + depth_range], stdout=subprocess.DEVNULL,
                     stderr=subprocess.DEVNULL, cwd=self.tmpdir, check=True)
    with open(os.path.join(self.history, 'TN136HS.309.snipped.mb54.esf.manifest')) as f:
      entries = [line.rstrip('\n').split('\t') for line in f if not line.startswith('#')]
    self.assertEqual(3, len(entries))
    self.assertEqual('created', entries[0][9])
    self.assertEqual('mbclean', entries[0][3])
    self.assertEqual(MB_PATH_MAXLINE, os.path.getsize(os.path.join(self.history, entries[0][8])))

    cmd = [self.cmd, '--input=' + swath, '--restore-date=2000-01-01', '--yes']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertEqual(0, result.returncode, output)
    self.assertIn('restored version 1', output)
    self.assertNotIn('were not recorded', output)
    self.assertEqual(MB_PATH_MAXLINE, os.path.getsize(swath + '.esf'))

  def testDatalist(self):
    self.make_versions()
    swath2 = os.path.join(self.tmpdir, 'line2.mb88')
    with open(swath2, 'w') as f:
      f.write('placeholder\n')
    datalist = os.path.join(self.tmpdir, 'datalist.mb-1')
    with open(datalist, 'w') as f:
      f.write('line.mb88 88\nline2.mb88 88\n')
    result = subprocess.run([self.cmd, '--input=' + datalist, '--list'], stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertEqual(0, result.returncode, output)
    self.assertIn('line.mb88.esf', output)
    self.assertIn('line2.mb88.esf, not present', output)

    # an empty datalist is reported
    empty = os.path.join(self.tmpdir, 'empty.mb-1')
    open(empty, 'w').close()
    result = subprocess.run([self.cmd, '--input=' + empty], stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    self.assertIn('No swath files found', result.stdout.decode())

  def testPrune(self):
    self.make_versions()
    code, output = self.run_mbhistory('--prune', '--keep=1')
    self.assertIn('2 tagged versions kept', output)
    self.assertIn('nothing to prune', output)

    code, output = self.run_mbhistory('--prune', '--keep=1', '--force')
    self.assertIn('would remove version 1', output)
    self.assertIn('would remove version 2', output)
    self.assertEqual(3, len(self.manifest()))

    code, output = self.run_mbhistory('--prune', '--keep=1', '--force', '--yes')
    self.assertEqual(0, code, output)
    self.assertEqual(['3'], [e[0] for e in self.manifest()])
    self.assertEqual(1, len(glob.glob(os.path.join(self.history, 'line.mb88.esf.v*'))))
    with open(os.path.join(self.history, 'tags.txt')) as f:
      tags = f.read()
    self.assertNotIn('t1\t', tags)
    self.assertIn('t3\t', tags)


if __name__ == '__main__':
  unittest.main()
