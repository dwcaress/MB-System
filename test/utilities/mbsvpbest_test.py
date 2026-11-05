#!/usr/bin/env python3

# See README file for copying and redistribution conditions.

"""Tests for mbsvpbest command line app."""

import glob
import json
import os
import shutil
import stat
import subprocess
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SWATH = os.path.join(HERE, 'testdata/mb58/0008_20111219_210356_METEOR_EM122.snipped.all')
WOA = os.path.join(HERE, 'testdata/woa')


class MbsvpbestUsageTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/svptools/mbsvpbest'

  def usage_error(self, args, expected):
    raised = False
    try:
      subprocess.check_output([self.cmd] + args, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(9, e.returncode)
      self.assertIn(expected, e.output)
    self.assertTrue(raised)

  def testNoArgs(self):
    self.usage_error([], b'--svpfolder {-S} is required')

  def testHelp(self):
    output = subprocess.check_output([self.cmd, '-H'], stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('not the true water column', output)
    self.assertIn('usage:', output)
    for option in ('--svpfolder', '--apply-best', '--enddate', '--input', '--minimum-beams', '--nearest',
                   '--outputfolder', '--period', '--plots', '--resolution', '--selection', '--verbose'):
      self.assertIn(option, output)

  def testBadOptions(self):
    self.usage_error(['-Sx', '--minimum-beams=2'], b'--minimum-beams must be >=3')
    self.usage_error(['-Sx', '-R02'], b'--resolution must be 01 or 04')
    self.usage_error(['-Sx', '--selection=other'], b'--selection must be')
    self.usage_error(['-Sx', 'stray'], b'Unexpected argument')


class MbsvpbestRunTest(unittest.TestCase):

  def setUp(self):
    self.cmd = os.path.abspath('../../src/svptools/mbsvpbest')
    self.dir = tempfile.mkdtemp(prefix='mbsvpbest_test')
    self.addCleanup(shutil.rmtree, self.dir)
    self.swath = os.path.join(self.dir, os.path.basename(SWATH))
    shutil.copy(SWATH, self.swath)
    self.output = os.path.join(self.dir, 'out')
    self.env = dict(os.environ)
    self.env['PATH'] = os.path.abspath('../../src/utilities') + os.pathsep + self.env['PATH']

  def run_tool(self, *args, code=0):
    result = subprocess.run([self.cmd, '-S', WOA, '-O', self.output] + list(args), env=self.env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    self.assertEqual(code, result.returncode, result.stdout)
    return result.stdout

  def ranking(self):
    names = glob.glob(self.output + '/run-*/ranking.json')
    self.assertEqual(1, len(names))
    with open(names[0]) as f:
      return json.load(f)

  def names(self, report):
    return [os.path.basename(r['profile']) for r in report['ranking']]

  def testCombinedRestoresFiles(self):
    before = sorted(os.listdir(self.dir))
    output = self.run_tool('-I', self.swath, '-P', 'decav', '-R', '01')
    self.assertIn('Original parameter files restored.', output)
    report = self.ranking()
    self.assertFalse(report['applied'])
    self.assertEqual(report['inputs'], [os.path.realpath(self.swath)])
    self.assertEqual(sorted(self.names(report)), sorted(
        ['internal_0000_0000.svp'] +
        [f'woa18_decav_svp{t}_01_{f}.svp' for t in ('00', '12', '16') for f in ('chen', 'leroy')] +
        [f'woa23_decav_svp00_01_{f}.svp' for f in ('chen', 'leroy')]))
    rmse = [r['rmse'] for r in report['ranking']]
    self.assertEqual(rmse, sorted(rmse))
    for record in report['ranking']:
      self.assertEqual(789, record['soundings'])
      self.assertEqual(2, record['pings'])
      self.assertEqual(0, record['rejected_pings'])
    # season and month of WOA23 are missing from the test folder
    self.assertEqual(2, len(report['profile_notes']))
    # nothing is left next to the data, and no trial output in the run directory
    self.assertEqual(before + ['out'], sorted(os.listdir(self.dir)))
    run = os.path.dirname(glob.glob(self.output + '/run-*/ranking.json')[0])
    self.assertEqual([], glob.glob(run + '/processed_*'))
    self.assertFalse(os.path.exists(run + '/backup'))
    with open(run + '/ranking.csv') as f:
      self.assertEqual('profile,rmse,mae,mse,me,soundings,pings,rejected_pings\n', f.readline())

  def testExistingSidecarsAreRestored(self):
    par = self.swath + '.par'
    resf = self.swath + '.resf'
    for name, text in ((par, b'## not much\nSVPMODE 0\n'), (resf, b'old')):
      with open(name, 'wb') as f:
        f.write(text)
      os.utime(name, (1000000000, 1000000000))
    self.run_tool('-I', self.swath, '-P', 'decav', '-R', '01')
    for name, text in ((par, b'## not much\nSVPMODE 0\n'), (resf, b'old')):
      with open(name, 'rb') as f:
        self.assertEqual(text, f.read())
      self.assertEqual(1000000000, os.stat(name)[stat.ST_MTIME])

  def testValuesBySpacesAndCommas(self):
    self.run_tool('-I', self.swath, '-P', 'decav', 'none', '-R', '01,01')
    self.assertEqual(9, len(self.ranking()['ranking']))

  def testNearest(self):
    self.run_tool('-I', self.swath, '-P', 'decav', '-R', '01', '-N')
    run = os.path.dirname(glob.glob(self.output + '/run-*/ranking.json')[0])
    with open(run + '/woa18_decav_svp00_01_chen.svp') as f:
      text = f.read()
    self.assertIn('"method":"nearest"', text)
    self.assertIn('"cells_used":1,', text)
    self.assertIn('physics_from_depth', text)

  def testNoCandidateOfGrids(self):
    # the internal profile is still a candidate
    output = self.run_tool('-I', self.swath, '-P', 'none', '-R', '01')
    self.assertIn('Profile note: No WOA18/WOA23', output)
    self.assertEqual(['internal_0000_0000.svp'], self.names(self.ranking()))

  def testApplyBest(self):
    output = self.run_tool('-I', self.swath, '-P', 'decav', '-R', '01', '--apply-best')
    self.assertIn('Winner applied to original inputs.', output)
    report = self.ranking()
    self.assertTrue(report['applied'])
    with open(self.swath + '.par') as f:
      self.assertIn(self.names(report)[0], f.read())
    root = self.swath[:-len('.all')]
    self.assertTrue(os.path.exists(root + 'p.mb58'))

  def write_list(self, *lines):
    path = os.path.join(self.dir, 'list.mb-1')
    with open(path, 'w') as f:
      f.write('# comment\n' + ''.join(line + '\n' for line in lines))
    return path

  def testIdenticalInternalProfileRunsOnce(self):
    other = self.swath[:-len('.all')] + 'b.all'
    shutil.copy(self.swath, other)
    path = self.write_list(os.path.basename(self.swath) + ' 58', os.path.basename(other) + ' 58')
    self.run_tool('-I', path, '-P', 'none', '-R', '01')
    report = self.ranking()
    self.assertEqual(['internal_0000_0000.svp'], self.names(report))
    self.assertEqual(2 * 789, report['ranking'][0]['soundings'])
    self.assertTrue(any('same table' in note for note in report['profile_notes']))

  def testPerFile(self):
    other = self.swath[:-len('.all')] + 'b.all'
    shutil.copy(self.swath, other)
    bad = self.swath[:-len('.all')] + 'c.all'
    with open(bad, 'w') as f:
      f.write('this is not swath data\n')
    path = self.write_list(os.path.basename(self.swath) + ' 58', os.path.basename(bad) + ' 58', os.path.basename(other) + ' 58')
    self.run_tool('-I', path, '-P', 'decav', '-R', '01', '--selection', 'per-file', code=1)
    names = glob.glob(self.output + '/per-file-*/summary.json')
    self.assertEqual(1, len(names))
    with open(names[0]) as f:
      summary = json.load(f)
    self.assertEqual('per-file', summary['selection'])
    self.assertFalse(summary['interrupted'])
    self.assertEqual(['success', 'failed', 'success'], [row['status'] for row in summary['files']])
    self.assertIsNone(summary['files'][1]['winner'])
    self.assertIn('rmse', summary['files'][0]['winner'])
    self.assertNotEqual(summary['files'][0]['output_directory'], summary['files'][2]['output_directory'])
    self.assertTrue(os.path.exists(summary['files'][2]['ranking_file']))

  def failing(self, input_path, expected):
    output = self.run_tool('-I', input_path, code=1)
    self.assertIn(expected, output)

  def testInputErrors(self):
    self.failing(os.path.join(self.dir, 'missing.all'), 'Input does not exist')
    self.failing(self.write_list('missing.all 58'), 'Input does not exist')
    self.failing(self.write_list('images.mb-2 -2'), 'Only ordinary/nested -1 datalists')
    self.failing(self.write_list('list.mb-1 -1'), 'Recursive datalist')
    self.failing(self.write_list(), 'Empty datalist')
    other = os.path.join(self.dir, 'unknown.qqq')
    with open(other, 'w') as f:
      f.write('x')
    self.failing(other, 'Cannot tell the swath format')

  def testBadGridFolder(self):
    result = subprocess.run([self.cmd, '-S', os.path.join(self.dir, 'none'), '-I', self.swath, '-O', self.output],
                            env=self.env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    self.assertEqual(1, result.returncode)
    self.assertIn('SVP directory does not exist', result.stdout)


if __name__ == '__main__':
  unittest.main()
