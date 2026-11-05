#!/usr/bin/env python3

# See README file for copying and redistribution conditions.

"""Tests for mbsvpextract command line app."""

import glob
import json
import os
import shutil
import stat
import subprocess
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
WOA = os.path.join(HERE, 'testdata/woa')
REQUIRED = ['--woa-version=23', '-Pdecav', '-R01']

SITES = """## Site File Version 2.00
## Number of sites: 3
 -11.5000000,  37.0000000, -3612.394, 4, 1,Site 0
 -11:15.0,     36:30.0,        0.000, 4, 1,Site, 1
 -10.2500000,  38.2500000,     0.000, 4, 1,Site 0
"""

ROUTE = """## Route File Version 2.00
##   WAYPOINT_NONE         0  Defines topography between waypoints
> ## STARTROUTE 4,0,1,1,Route:1
-12.5,36.5,-3000.0,1,90.0,0.0,0.0,0.1 ## WAYPOINT
-12.0,36.5,-3000.0,0,90.0,0.0,0.0,0.1
-11.5,36.5,0.0,3,90.0,0.0,0.0,0.1 ## WAYPOINT STARTLINE
-10.5,37.5,0.0,4,90.0,0.0,0.0,0.1 ## WAYPOINT ENDLINE
> ## ENDROUTE
"""


class MbsvpextractUsageTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/svptools/mbsvpextract'

  def usage_error(self, args, expected):
    result = subprocess.run([self.cmd] + args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(9, result.returncode)
    self.assertIn(expected, result.stdout)

  def testNoArgs(self):
    result = subprocess.run([self.cmd], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.assertEqual(9, result.returncode)
    for text in (b'--input {-I}, --sites or --route is required', b'--svpfolder {-S} is required',
                 b'--outputfolder {-O} is required', b'--woa-version (18 or 23) is required',
                 b'--period {-P} (such as decav, A5B7 or B5C2) is required', b'--resolution {-R} (01 or 04) is required'):
      self.assertIn(text, result.stdout)

  def testHelp(self):
    output = subprocess.check_output([self.cmd, '-H'], stderr=subprocess.STDOUT).decode()
    self.assertIn('Version', output)
    self.assertIn('waypoints of a route file', output)
    self.assertIn('https://zenodo.org/communities/mb-system_woa_svp_datasets/', output)
    self.assertIn('usage:', output)
    for option in ('--input', '--sites', '--route', '--svpfolder', '--outputfolder', '--woa-version', '--period', '--resolution',
                   '--formula', '--strict-nearest', '--overwrite', '--verbose'):
      self.assertIn(option, output)

  def testBadOptions(self):
    base = ['-Ia', '-Sb', '-Oc']
    self.usage_error(base + ['--woa-version=19', '-Pdecav', '-R01'], b'--woa-version must be 18 or 23')
    self.usage_error(base + ['--woa-version=23', '-Pdecav', '-R02'], b'--resolution must be 01 or 04')
    self.usage_error(base + ['--woa-version=23', '-P../x', '-R01'], b'--period holds letters and digits only')
    self.usage_error(base + REQUIRED + ['--formula=x'], b'--formula must be')
    self.usage_error(base + REQUIRED + ['stray'], b'Unexpected argument')


class MbsvpextractRunTest(unittest.TestCase):

  def setUp(self):
    self.cmd = os.path.abspath('../../src/svptools/mbsvpextract')
    self.dir = tempfile.mkdtemp(prefix='mbsvpextract_test')
    self.addCleanup(shutil.rmtree, self.dir)
    self.output = os.path.join(self.dir, 'out')
    self.sites = self.write('sites.ste', SITES)
    self.route = self.write('route.rte', ROUTE)

  def write(self, name, text):
    path = os.path.join(self.dir, name)
    with open(path, 'w') as f:
      f.write(text)
    return path

  def run_tool(self, *args, code=0, required=REQUIRED, woa=WOA):
    result = subprocess.run([self.cmd, '-S', woa, '-O', self.output] + required + list(args),
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    self.assertEqual(code, result.returncode, result.stdout)
    return result.stdout

  def summary(self):
    names = glob.glob(self.output + '/mbsvpextract-*.json')
    with open(max(names, key=os.path.getmtime)) as f:
      return json.load(f)

  def read(self, name):
    with open(os.path.join(self.output, name)) as f:
      return f.read().splitlines()

  def testSites(self):
    self.run_tool('-I', self.sites)
    self.assertEqual(['Site_0.svp', 'Site_0_2.svp', 'Site_1.svp'],
                     sorted(n for n in os.listdir(self.output) if n.endswith('.svp')))
    lines = self.read('Site_0.svp')
    self.assertEqual('# mbsvpextract; formula=chen; period=decav; annual average; endpoint=11000m; site=Site 0', lines[0])
    self.assertTrue(lines[1].startswith('# MBSVPEXTRACT_LOCATION {'))
    meta = json.loads(lines[1].split(' ', 2)[2])
    self.assertEqual({'version': 1, 'Longitude_site': -11.5, 'Latitude_site': 37.0, 'WOAdataset': 'WOA23',
                      'WOA_resolution': '01', 'WOA_period': 'decav', 'Longitude_WOA': -11.5, 'Latitude_WOA': 36.5}, meta)
    self.assertTrue(lines[2].startswith('0.000000 '))
    self.assertTrue(lines[-1].startswith('11000.000000 '))
    # a usual file mode, not owner only
    self.assertTrue(stat.S_IMODE(os.stat(os.path.join(self.output, 'Site_0.svp')).st_mode) & 0o044)
    report = self.summary()
    self.assertEqual('sites', report['input_type'])
    self.assertEqual(os.path.realpath(self.sites), report['input_file'])
    self.assertEqual('WOA23', report['edition'])
    self.assertEqual('decav', report['period'])
    self.assertFalse(report['interrupted'])
    self.assertEqual(['success'] * 3, [row['status'] for row in report['sites']])
    self.assertEqual('Site, 1', report['sites'][1]['name'])
    self.assertEqual(-11.25, report['sites'][1]['longitude'])
    self.assertEqual(36.5, report['sites'][1]['latitude'])
    self.assertEqual(3, report['sites'][0]['line'])
    self.assertEqual(1, report['sites'][0]['candidate_rank'])

  def testRoute(self):
    self.run_tool('-I', self.route)
    self.assertEqual(['Route_1_WP000.svp', 'Route_1_WP001.svp', 'Route_1_WP002.svp'],
                     sorted(n for n in os.listdir(self.output) if n.endswith('.svp')))
    meta = json.loads(self.read('Route_1_WP002.svp')[1].split(' ', 2)[2])
    self.assertEqual(-10.5, meta['Longitude_site'])
    self.assertEqual(37.5, meta['Latitude_site'])
    self.assertEqual('Route:1', meta['Route'])
    self.assertEqual(2, meta['Waypoint'])
    report = self.summary()
    self.assertEqual('route', report['input_type'])
    self.assertEqual(['Route:1 WP000', 'Route:1 WP001', 'Route:1 WP002'], [row['name'] for row in report['sites']])
    self.assertEqual([4, 6, 7], [row['line'] for row in report['sites']])
    self.assertEqual([1, 3, 4], [row['waypoint_type'] for row in report['sites']])

  def testRouteAndSitesGiveTheSameProfile(self):
    self.run_tool('--route', self.route)
    route_lines = self.read('Route_1_WP001.svp')
    site = self.write('one.ste', '## Site File Version 2.00\n-11.5,36.5,0,0,0,One\n')
    self.run_tool('--sites', site)
    self.assertEqual(route_lines[2:], self.read('One.svp')[2:])

  def testForcedTypeWins(self):
    path = self.write('plain.txt', ROUTE.replace('## Route File Version 2.00\n', ''))
    self.run_tool('-I', path, code=1)
    self.run_tool('--route', path)
    self.assertTrue(os.path.exists(os.path.join(self.output, 'Route_1_WP000.svp')))

  def testFormulaBoth(self):
    self.run_tool('-I', self.route, '--formula=both')
    chen = self.read('Route_1_WP000_chen.svp')
    leroy = self.read('Route_1_WP000_leroy.svp')
    self.assertIn('formula=chen', chen[0])
    self.assertIn('formula=leroy', leroy[0])
    self.assertNotEqual(chen[2], leroy[2])
    self.assertEqual(2, len(self.summary()['sites'][0]['outputs']))

  def testFormulaLeroy(self):
    self.run_tool('-I', self.route, '--formula=leroy')
    self.assertIn('formula=leroy', self.read('Route_1_WP000.svp')[0])

  def testUserChoosesTheGrid(self):
    # the same bytes under another period name: the requested name is what is read and recorded
    grids = os.path.join(self.dir, 'grids')
    os.mkdir(grids)
    shutil.copy(os.path.join(WOA, 'woa18_decav_svp00_01.nc'), os.path.join(grids, 'woa18_A5B7_svp00_01.nc'))
    self.run_tool('-I', self.route, required=['--woa-version=18', '-P', 'A5B7', '-R', '01'], woa=grids)
    lines = self.read('Route_1_WP000.svp')
    self.assertIn('period=A5B7', lines[0])
    meta = json.loads(lines[1].split(' ', 2)[2])
    self.assertEqual('WOA18', meta['WOAdataset'])
    self.assertEqual('A5B7', meta['WOA_period'])
    self.assertTrue(self.summary()['dataset'].endswith('woa18_A5B7_svp00_01.nc'))

  def testMissingDatasetIsNotReplaced(self):
    for required in (['--woa-version=18', '-PB5C2', '-R01'], ['--woa-version=23', '-PA5B7', '-R01'], ['--woa-version=23', '-Pdecav', '-R04']):
      output = self.run_tool('-I', self.route, required=required, code=1)
      self.assertIn('Required dataset not found', output)
      self.assertIn('https://zenodo.org/communities/mb-system_woa_svp_datasets/', output)
    self.assertFalse(os.path.exists(self.output))

  def testOverwrite(self):
    self.run_tool('-I', self.route)
    path = os.path.join(self.output, 'Route_1_WP000.svp')
    with open(path, 'w') as f:
      f.write('kept')
    output = self.run_tool('-I', self.route, code=1)
    self.assertIn('Output exists; use --overwrite', output)
    with open(path) as f:
      self.assertEqual('kept', f.read())
    self.assertEqual(['failed'] * 3, [row['status'] for row in self.summary()['sites']])
    self.run_tool('-I', self.route, '--overwrite')
    with open(path) as f:
      self.assertNotEqual('kept', f.read())

  def testPositionOutsideGridFailsAndNextGoesOn(self):
    path = self.write('two.ste', '## Site File Version 2.00\n50.0,37.0,0,0,0,Far\n-11.5,37.0,0,0,0,Near\n')
    output = self.run_tool('-I', path, code=1)
    self.assertIn('Site outside cropped longitude coverage', output)
    report = self.summary()
    self.assertEqual(['failed', 'success'], [row['status'] for row in report['sites']])
    self.assertIn('cropped', report['sites'][0]['error'])
    self.assertFalse(os.path.exists(os.path.join(self.output, 'Far.svp')))

  def testInputErrors(self):
    self.assertIn('Input does not exist', self.run_tool('-I', os.path.join(self.dir, 'none.ste'), code=1))
    self.assertIn('--sites or --route', self.run_tool('-I', self.write('x.txt', '1 2\n'), code=1))
    self.assertIn(':2: Latitude outside', self.run_tool('-I', self.write('y.ste', '## Site File Version 2.00\n1,95,0,0,0,a\n'), code=1))
    self.assertIn('No lines marked ## WAYPOINT', self.run_tool('-I', self.write('z.rte', '## Route File Version 2.00\n1,2,3,0\n'), code=1))


if __name__ == '__main__':
  unittest.main()
