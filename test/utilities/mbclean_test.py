#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbclean command line app."""

import os
import struct
import subprocess
import tempfile
import unittest


def read_esf_filter_edits(path):
  """Returns the (time_d, beam) of the filter edits of an edit save file."""
  with open(path, 'rb') as f:
    data = f.read()
  offset = 1024 if data[:10] == b'ESFVERSION' else 0
  edits = set()
  while offset + 16 <= len(data):
    time_d, beam, action = struct.unpack('>dii', data[offset:offset + 16])
    offset += 16
    if action == 4:  # MBP_EDIT_FILTER
      edits.add((round(time_d, 6), beam % 100000000))
  return edits


class MbCleanTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbclean'
    self.mblist = os.path.abspath('../../src/utilities/mblist')
    self.synthetic = os.path.abspath('mbcube_synthetic')
    self.mbgrid = os.path.abspath('../../src/utilities/mbgrid')

  def testNoArgs(self):
    cmd = [self.cmd]
    raised = False
    try:
      subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raised = True
      self.assertEqual(2, e.returncode)
      self.assertIn(b'Unable to open data list file:', e.output)
      self.assertIn(b'datalist.mb-1', e.output)
    self.assertTrue(raised)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('-Sspike_slope', output)
    self.assertIn('', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('identifies and flags artifacts', output)
    self.assertIn('-Sspike_slope', output)
    self.assertIn('lonflip', output)

  def testHelpCube(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('--cube-spacing=dx[/dy]', output)
    self.assertIn('--cube-flag=hypothesis|outlier|both', output)
    self.assertIn('--cube-iho-edition=4|5|6', output)
    self.assertIn('--cube-prediction=median[/factor]|gridfile', output)
    self.assertIn('--cube-prediction-topography', output)

  def testCubeCannotBeCombined(self):
    cmd = [self.cmd, '--cube-spacing=2', '-C3']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(9, result.returncode)  # MB_ERROR_BAD_USAGE
    self.assertIn('cannot be combined', result.stdout.decode())

  def testCubeInvalidOptions(self):
    for option in ('--cube-order=4', '--cube-flag=all', '--cube-spacing=2/x'):
      cmd = [self.cmd, '--cube-spacing=2', option]
      result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
      self.assertEqual(9, result.returncode, option)
      self.assertIn('Invalid CUBE option: ' + option, result.stdout.decode())

  def testCubeBayesFactorTooLarge(self):
    # from exp(h^2 / 2) on every sounding is an outlier (threshold below zero)
    cmd = [self.cmd, '--cube-spacing=2', '--cube-est-offset=1', '--cube-bayes-factor=2']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(12, result.returncode)  # MB_ERROR_BAD_PARAMETER
    self.assertIn('(--cube-bayes-factor) must be below exp(h^2 / 2)', result.stdout.decode())

  def testCubePredictionTopographyNeedsGrid(self):
    cmd = [self.cmd, '--cube-spacing=2', '--cube-prediction-topography']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(12, result.returncode)
    self.assertIn('needs a predicted surface grid', result.stdout.decode())

  def runCubeSpikes(self, tmp, options):
    """Flags synthetic spikes (2% of the soundings, 5 to 20 m off a flat
    seafloor at 100 m), returning the output, the detected and the falsely
    flagged fractions."""
    swath = os.path.join(tmp, 'spikes.mb71')
    if not os.path.exists(swath):
      subprocess.check_output([self.synthetic, swath, 'spikes', '100', '0'])
    datalist = os.path.join(tmp, 'datalist.mb-1')
    with open(datalist, 'w') as f:
      f.write(swath + ' 71\n')
    cmd = [os.path.abspath(self.cmd), '-I' + datalist, '--cube-spacing=2'] + options
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmp).decode()
    print('\nOutput from ' + ' '.join(cmd) + ':\n--------------------------')
    print(output)
    listing = subprocess.check_output([self.mblist, '-I' + swath, '-F71', '-MA', '-OM#z'], cwd=tmp).decode()
    spikes = set()
    good = set()
    for line in listing.splitlines():
      time_d, beam, depth = line.split()
      key = (round(float(time_d), 6), int(beam))
      (spikes if abs(float(depth) - 100.0) > 1.0 else good).add(key)
    flagged = read_esf_filter_edits(swath + '.esf')
    self.assertGreater(len(spikes), 1000)
    return output, len(flagged & spikes) / len(spikes), len(flagged & good) / len(good)

  def testCubeFlagsSpikes(self):
    if not os.path.exists(self.synthetic) or not os.path.exists(self.mblist):
      self.skipTest('mbcube_synthetic or mblist not built')
    with tempfile.TemporaryDirectory() as tmp:
      output, detected, false = self.runCubeSpikes(tmp, ['--cube-order=special'])
      self.assertIn('CUBE flagging:', output)
      self.assertIn('IHO S-44 Ed. 6 Special Order', output)
      self.assertIn('2.501 normalized errors', output)
      self.assertGreater(detected, 0.98)
      self.assertLess(false, 0.001)
      with open(os.path.join(tmp, 'spikes.mb71.par')) as f:
        self.assertIn('EDITSAVEFILE', f.read())
      # a second run finds the flagged soundings already flagged
      output, detected_again, false_again = self.runCubeSpikes(tmp, ['--cube-order=special'])
      self.assertIn('\n0 total beams flagged', output)
      self.assertEqual(detected, detected_again)
      self.assertEqual(false, false_again)

  def testCubeFlagsSpikesWithMedianPrediction(self):
    if not os.path.exists(self.synthetic) or not os.path.exists(self.mblist):
      self.skipTest('mbcube_synthetic or mblist not built')
    with tempfile.TemporaryDirectory() as tmp:
      output, detected, false = self.runCubeSpikes(
          tmp, ['--cube-model=constant', '--cube-sigma-v=0.05', '--cube-sigma-h=0.5',
                '--cube-prediction=median/2', '--cube-extractor=predicted'])
      self.assertIn('data points used for the median predicted surface', output)
      self.assertGreater(detected, 0.98)
      self.assertLess(false, 0.002)

  def testCubeHypothesisOnly(self):
    if not os.path.exists(self.synthetic) or not os.path.exists(self.mblist):
      self.skipTest('mbcube_synthetic or mblist not built')
    with tempfile.TemporaryDirectory() as tmp:
      output, detected, false = self.runCubeSpikes(tmp, ['--cube-order=special', '--cube-flag=hypothesis'])
      self.assertIn('\n0 total soundings exceeding the outlier threshold', output)
      self.assertLess(detected, 0.5)
      self.assertLess(false, 0.001)

  def testCubeSingleFile(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      swath = os.path.join(tmp, 'spikes.mb71')
      subprocess.check_output([self.synthetic, swath, 'spikes', '100', '0'])
      cmd = [os.path.abspath(self.cmd), '-I' + swath, '--cube-spacing=2', '--cube-order=special']
      output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmp).decode()
      self.assertIn('Grid dimensions:        234 x 151', output)
      self.assertIn('1 total swath data files processed', output)
      self.assertGreater(len(read_esf_filter_edits(swath + '.esf')), 1200)

  def testCubePredictionGridBlunders(self):
    if not os.path.exists(self.synthetic) or not os.path.exists(self.mbgrid):
      self.skipTest('mbcube_synthetic or mbgrid not built')
    with tempfile.TemporaryDirectory() as tmp:
      # a predicted surface 30 m too shallow: the blunder test rejects the
      # soundings deeper than 70 m + max(10 m, 25%, 3 sigma), so only the
      # shallow spikes reach the nodes and the seafloor soundings compared
      # with them are blunders
      shallow = os.path.join(tmp, 'shallow.mb71')
      subprocess.check_output([self.synthetic, shallow, 'flat', '70', '0'])
      shallow_list = os.path.join(tmp, 'shallow.mb-1')
      with open(shallow_list, 'w') as f:
        f.write(shallow + ' 71\n')
      subprocess.check_output([self.mbgrid, '-I' + shallow_list, '-O' + os.path.join(tmp, 'pred'), '-A1', '-F2',
                               '-E4/4/meters'], stderr=subprocess.STDOUT, cwd=tmp)
      swath = os.path.join(tmp, 'spikes.mb71')
      subprocess.check_output([self.synthetic, swath, 'spikes', '100', '0'])
      cmd = [os.path.abspath(self.cmd), '-I' + swath, '--cube-spacing=2', '--cube-order=special',
             '--cube-prediction=' + os.path.join(tmp, 'pred.grd')]
      output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmp).decode()
      self.assertIn('Predicted surface set at', output)
      blunders = [int(line.split()[0]) for line in output.splitlines()
                  if line.endswith('total soundings rejected by the blunder test')]
      self.assertEqual(1, len(blunders))
      self.assertGreater(blunders[0], 0)

  def testCubePredictionGridSign(self):
    if not os.path.exists(self.synthetic) or not os.path.exists(self.mbgrid):
      self.skipTest('mbcube_synthetic or mbgrid not built')
    with tempfile.TemporaryDirectory() as tmp:
      # the 100 m seafloor of the spike data as a grid of depths (-A1) and
      # as a grid of heights (-A2)
      flat = os.path.join(tmp, 'flat.mb71')
      subprocess.check_output([self.synthetic, flat, 'flat', '100', '0'])
      flat_list = os.path.join(tmp, 'flat.mb-1')
      with open(flat_list, 'w') as f:
        f.write(flat + ' 71\n')
      for name, datatype in (('depth', '-A1'), ('height', '-A2')):
        subprocess.check_output([self.mbgrid, '-I' + flat_list, '-O' + os.path.join(tmp, name), datatype, '-F2',
                                 '-E4/4/meters'], stderr=subprocess.STDOUT, cwd=tmp)
      edits = {}
      for name, options in (('depth', []), ('height', ['--cube-prediction-topography'])):
        swath = os.path.join(tmp, 'spikes_' + name + '.mb71')
        subprocess.check_output([self.synthetic, swath, 'spikes', '100', '0'])
        cmd = [os.path.abspath(self.cmd), '-I' + swath, '--cube-spacing=2', '--cube-order=special',
               '--cube-prediction=' + os.path.join(tmp, name + '.grd')]
        if name == 'height':
          # read as depths, the heights are opposite to the soundings:
          # stopped after the first read, before anything is flagged
          result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=tmp)
          self.assertEqual(12, result.returncode)
          self.assertIn('has the opposite sign to', result.stdout.decode())
          self.assertFalse(os.path.exists(swath + '.esf'))
        output = subprocess.check_output(cmd + options, stderr=subprocess.STDOUT, cwd=tmp).decode()
        if name == 'height':
          self.assertIn('(heights, positive up)', output)
        edits[name] = read_esf_filter_edits(swath + '.esf')
      # the -A1 and -A2 median grids differ by up to a few decimeters
      # (mbgrid takes one of the two middle values of an even count)
      self.assertGreater(len(edits['depth']), 1200)
      self.assertGreater(len(edits['depth'] & edits['height']), 0.98 * len(edits['depth']))
      self.assertLess(abs(len(edits['height']) - len(edits['depth'])), 0.02 * len(edits['depth']))


if __name__ == '__main__':
  unittest.main()
