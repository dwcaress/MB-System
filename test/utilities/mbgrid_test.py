#!/usr/bin/env python3

# Copyright 2019 Google Inc. All Rights Reserved.
#
# See README file for copying and redistribution conditions.

"""Tests for mbgrid command line app."""

import os
import shutil
import subprocess
import tempfile
import unittest


def read_ascii_grid(path):
  """Reads an mbgrid -G1 ascii grid, returning the list of values."""
  with open(path) as f:
    lines = f.read().split('\n')
  nx, ny = (int(v) for v in lines[2].split())
  values = [float(v) for line in lines[4:] for v in line.split()]
  assert len(values) == nx * ny
  return values


def depth_errors(path, truth):
  """Absolute errors of the grid nodes that have data, and the node count."""
  values = read_ascii_grid(path)
  errors = sorted(abs(v - truth) for v in values if v < 99990.0)
  return errors, len(values)


class MbGridTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/utilities/mbgrid'
    self.synthetic = os.path.abspath('mbcube_synthetic')

  def testNoArgs(self):
    cmd = [self.cmd]
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid:\n--------------------------")
    print(output)
    self.assertIn('Unable to open data list file:', output)
    self.assertIn('datalist.mb-1', output)

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid -h:\n--------------------------")
    print(output)
    self.assertIn('Version', output)
    self.assertIn('used to grid bathymetry, amplitude', output)
    self.assertIn('curvature algorithm', output)
    self.assertIn('usage:', output)
    self.assertIn('-Xextend', output)
    self.assertIn('CUBE', output)
    self.assertIn('--cube-order=exclusive|special|1a|1b|2', output)
    self.assertIn('--cube-depth-reference=sonar|surface', output)
    self.assertIn('--cube-model-c=fraction', output)
    self.assertIn('--cube-prediction=median[/factor]|gridfile', output)

  def testHelpVerbose2(self):
    cmd = [self.cmd, '-h', '-V', '-V']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    print("\nOutput from mbgrid -h -V -V:\n--------------------------")
    print(output)
    self.assertIn('Version', output)
    self.assertIn('used to grid bathymetry, amplitude', output)
    self.assertIn('curvature algorithm', output)
    self.assertIn('usage:', output)
    self.assertIn('-Xextend', output)
    self.assertIn('dbg2', output)
    self.assertIn('lonflip', output)
    self.assertIn('minormax_weighted_mean_threshold:', output)

  def testCubeInvalidOption(self):
    cmd = [self.cmd, '-F9', '--cube-order=4']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertNotEqual(0, result.returncode)
    self.assertIn('Invalid CUBE option: --cube-order=4', output)

  def testCubeOrderOfOtherEdition(self):
    # Order 3 exists only in edition 4 (and the CCOM table), Order 1a only
    # in editions 5 and 6, Exclusive Order only in edition 6
    bounds = ['-R-124.51/-124.49/40.79/40.81', '-E10/10/meters']
    for options, message in ((['--cube-order=3'], 'IHO S-44 edition 6 has no order 3'),
                             (['--cube-iho-edition=4', '--cube-order=1a'], 'IHO S-44 edition 4 has no order 1a'),
                             (['--cube-iho-edition=5', '--cube-order=exclusive'], 'IHO S-44 edition 5 has no order exclusive'),
                             (['--cube-iho-ccom', '--cube-order=1b'], 'errmod_iho table (--cube-iho-ccom) has no order 1b'),
                             (['--cube-iho-ccom', '--cube-iho-edition=4'], 'cannot be combined'),
                             (['--cube-iho-edition=3'], 'Invalid CUBE option: --cube-iho-edition=3')):
      cmd = [self.cmd, '-F9'] + options + bounds
      result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
      output = result.stdout.decode()
      self.assertNotEqual(0, result.returncode, options)
      self.assertIn(message, output)

  def testCubeIhoEdition(self):
    # the selected edition and order are reported before the data are read
    bounds = ['-R-124.51/-124.49/40.79/40.81', '-E10/10/meters']
    for options, message in ((['--cube-iho-edition=4', '--cube-order=3'],
                              'IHO S-44 Ed. 4 Order 3 (TVU a 1.000 m, b 0.0230; THU 150.0 m + 5% of depth)'),
                             (['--cube-iho-edition=5', '--cube-order=2'],
                              'IHO S-44 Ed. 5 Order 2 (TVU a 1.000 m, b 0.0230; THU 20.0 m + 10% of depth)'),
                             (['--cube-order=2', '--cube-iho-edition=4'],
                              'IHO S-44 Ed. 4 Order 2 (TVU a 1.000 m, b 0.0230; THU 20.0 m + 5% of depth)'),
                             (['--cube-iho-ccom', '--cube-order=special'],
                              'CCOM errmod_iho Special Order (TVU a 0.200 m, b 0.0075; THU 2.0 m + 0% of depth)')):
      cmd = [self.cmd, '-F9'] + options + bounds
      result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
      self.assertIn('Sounding uncertainty:   ' + message, result.stdout.decode())

  def testCubeRejectsAmplitude(self):
    cmd = [self.cmd, '-F9', '-A3', '-R-124.51/-124.49/40.79/40.81', '-E10/10/meters']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertNotEqual(0, result.returncode)
    self.assertIn('only available for bathymetry and topography', output)

  def testCubeRejectsTimeDifference(self):
    cmd = [self.cmd, '-F9', '-U10', '-R-124.51/-124.49/40.79/40.81', '-E10/10/meters']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertNotEqual(0, result.returncode)
    self.assertIn('cannot be used with CUBE gridding', output)

  def runCube(self, tmp, seafloor, depth, sensordepth, root, options):
    """Writes synthetic swath data and grids it as -G1 ascii grids."""
    swath = os.path.join(tmp, root + '.mb71')
    datalist = os.path.join(tmp, root + '_datalist.mb-1')
    subprocess.check_output([self.synthetic, swath, seafloor, str(depth), str(sensordepth)])
    with open(datalist, 'w') as f:
      f.write(swath + ' 71\n')
    cmd = [os.path.abspath(self.cmd), '-I' + datalist, '-O' + os.path.join(tmp, root), '-A1', '-G1'] + options
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, cwd=tmp).decode()
    print('\nOutput from ' + ' '.join(cmd) + ':\n--------------------------')
    print(output)
    return output

  def testCubeFlatSeafloor(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      output = self.runCube(tmp, 'flat', 100, 0, 'flat', ['-F9', '-E2/2/meters', '-M'])
      self.assertIn('CUBE (Combined Uncertainty and Bathymetric Estimator)', output)
      self.assertIn('Sounding uncertainty:   IHO S-44 Ed. 6 Order 2', output)
      errors, nnodes = depth_errors(os.path.join(tmp, 'flat.asc'), 100.0)
      self.assertGreater(len(errors), 0.85 * nnodes)
      self.assertLess(errors[-1], 0.25)
      for suffix in ('_num', '_sd', '_nhyp', '_strength'):
        self.assertTrue(os.path.exists(os.path.join(tmp, 'flat' + suffix + '.asc')), suffix)
      nhyp = read_ascii_grid(os.path.join(tmp, 'flat_nhyp.asc'))
      strength = read_ascii_grid(os.path.join(tmp, 'flat_strength.asc'))
      depth = read_ascii_grid(os.path.join(tmp, 'flat.asc'))
      for k, z in enumerate(depth):
        if z < 99990.0:
          self.assertGreaterEqual(nhyp[k], 1.0)
          self.assertGreaterEqual(strength[k], 0.0)
          self.assertLessEqual(strength[k], 5.0)

  def testCubeRejectsSpikes(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      self.runCube(tmp, 'spikes', 100, 0, 'cube', ['-F9', '-E2/2/meters'])
      self.runCube(tmp, 'spikes', 100, 0, 'median', ['-F2', '-E2/2/meters'])
      cube_errors, _ = depth_errors(os.path.join(tmp, 'cube.asc'), 100.0)
      median_errors, _ = depth_errors(os.path.join(tmp, 'median.asc'), 100.0)
      # 99th percentile of the node errors
      self.assertLess(cube_errors[int(0.99 * len(cube_errors))], 0.3)
      self.assertGreater(median_errors[int(0.99 * len(median_errors))], 1.0)

  def testCubePredictedRuleNeedsPrediction(self):
    cmd = [self.cmd, '-F9', '--cube-extractor=predicted', '-R-124.51/-124.49/40.79/40.81', '-E10/10/meters']
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode()
    self.assertNotEqual(0, result.returncode)
    self.assertIn('needs a predicted surface', output)

  def testCubeMedianPrediction(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      # sparse data (a few soundings per 2 m node): the median is taken on a
      # grid four times coarser, so single spikes do not become predictions
      output = self.runCube(tmp, 'spikes', 100, 0, 'pred',
                            ['-F9', '-E2/2/meters', '--cube-prediction=median/4', '--cube-extractor=predicted'])
      self.assertIn('Predicted surface:      median of the soundings nearest to each node', output)
      self.assertIn('4 times the node spacing', output)
      self.assertIn('data points used for the median predicted surface', output)
      self.assertIn('Predicted surface set at', output)
      errors, _ = depth_errors(os.path.join(tmp, 'pred.asc'), 100.0)
      self.assertLess(errors[int(0.99 * len(errors))], 0.3)

  def testCubeGridTitle(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    if shutil.which('gmt') is None:
      self.skipTest('gmt not found')
    with tempfile.TemporaryDirectory() as tmp:
      self.runCube(tmp, 'flat', 100, 0, 'title', ['-F9', '-E4/4/meters', '-G3', '--cube-iho-edition=5',
                                                  '--cube-order=1a'])
      info = subprocess.check_output(['gmt', 'grdinfo', os.path.join(tmp, 'title.grd')]).decode()
      self.assertIn('Bathymetry Grid (CUBE, IHO S-44 Ed. 5 Order 1a)', info)

  def testCubeDepthBelowSonar(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      # AUV 50 m above a 3000 m seafloor with spikes: uncertainties scaled by
      # the depth below the sonar reject the spikes, uncertainties scaled by
      # the full water depth let them bias the surface
      options = ['-F9', '-E4/4/meters', '--cube-order=special']
      self.runCube(tmp, 'spikes', 3000, 2950, 'sonar', options)
      self.runCube(tmp, 'spikes', 3000, 2950, 'surface', options + ['--cube-depth-reference=surface'])
      sonar_errors, _ = depth_errors(os.path.join(tmp, 'sonar.asc'), 3000.0)
      surface_errors, _ = depth_errors(os.path.join(tmp, 'surface.asc'), 3000.0)
      self.assertLess(sonar_errors[-1], 0.3)
      self.assertGreater(surface_errors[-1], 1.0)

  def testCubePredictionGridSign(self):
    if not os.path.exists(self.synthetic):
      self.skipTest('mbcube_synthetic not built')
    with tempfile.TemporaryDirectory() as tmp:
      # a grid of heights (-A2) is the predicted surface of a topography
      # grid, but not of a bathymetry grid (-A1)
      swath = os.path.join(tmp, 'flat.mb71')
      datalist = os.path.join(tmp, 'flat_datalist.mb-1')
      subprocess.check_output([self.synthetic, swath, 'flat', '100', '0'])
      with open(datalist, 'w') as f:
        f.write(swath + ' 71\n')
      cmd = [os.path.abspath(self.cmd), '-I' + datalist]
      subprocess.check_output(cmd + ['-O' + os.path.join(tmp, 'height'), '-A2', '-F2', '-E4/4/meters'],
                              stderr=subprocess.STDOUT, cwd=tmp)
      cube = cmd + ['-F9', '-E2/2/meters', '-G1', '--cube-prediction=' + os.path.join(tmp, 'height.grd')]
      result = subprocess.run(cube + ['-A1', '-O' + os.path.join(tmp, 'bathymetry')], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, cwd=tmp)
      self.assertEqual(12, result.returncode)  # MB_ERROR_BAD_PARAMETER
      self.assertIn('with -A1 its values must be depths (positive down)', result.stdout.decode())
      result = subprocess.run(cube + ['-A2', '-O' + os.path.join(tmp, 'topography')], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, cwd=tmp)
      self.assertEqual(0, result.returncode)
      self.assertIn('Predicted surface set at', result.stdout.decode())


if __name__ == '__main__':
  unittest.main()
