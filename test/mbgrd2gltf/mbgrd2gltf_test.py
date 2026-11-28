#!/usr/bin/env python3

"""Tests for mbgrd2gltf command line app."""

import subprocess
import unittest


class Mbgrd2gltfTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/mbgrd2gltf/mbgrd2gltf'

  def testHelp(self):
    cmd = [self.cmd, '-h']
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    self.assertIn('MBgrd2gltf', output)
    self.assertIn('converts a GMT GRD format bathymetry grid file', output)
    self.assertIn('usage: mbgrd2gltf', output)

  def testGeoViewpointIsValidated(self):
    # --geoviewpoint takes 6 or 10 numbers, attached, and needs --html.
    cases = (
      (['--html', '--geoviewpoint=1,2,3'], 'takes 6 numbers'),
      (['--html', '--geoviewpoint=a,b,c,d,e,f'], 'is not a number'),
      (['--html', '--geoviewpoint=36.6,-122.4,-900,36.61,-122.39,-950'], 'lon,lat,elev'),
      (['--geoviewpoint=-122.4,36.6,-900,-122.39,36.61,-950'], 'use one of them too'),
    )
    for args, message in cases:
      proc = subprocess.run([self.cmd, '-I', 'nonexistent.grd'] + args,
                            stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
      self.assertNotEqual(proc.returncode, 0, args)
      self.assertIn(message, proc.stdout.decode(), args)

  def testDracoOptionsAreValidated(self):
    # The options are those of mbmesh: --draco-precision is a size in meters above zero and --draco-normal-bits a number of
    # bits from 2 to 30. The quantize options that they replace are refused with a message that says so.
    cases = (
      (['--draco-precision=abc'], 'not a size in meters'),
      (['--draco-precision=0'], 'not a size in meters'),
      (['--draco-normal-bits=1'], 'not a number of bits'),
      (['--draco-normal-bits=31'], 'not a number of bits'),
      (['--quantize-position', '20'], 'replaced by --draco-precision'),
      (['-Qp', '20'], 'replaced by --draco-precision'),
    )
    for args, message in cases:
      proc = subprocess.run([self.cmd, '-I', 'nonexistent.grd'] + args,
                            stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
      self.assertNotEqual(proc.returncode, 0, args)
      self.assertIn(message, proc.stdout.decode(), args)

  def testHelpDocumentsDracoOptions(self):
    output = subprocess.check_output([self.cmd, '-h'],
                                     stderr=subprocess.STDOUT).decode()
    for option in ('--no-binary', '--no-draco', '--draco-precision', '--draco-normal-bits'):
      self.assertIn(option, output)
    self.assertNotIn('--quantize-position', output)

  def testHelpDocumentsGeoViewpoint(self):
    output = subprocess.check_output([self.cmd, '-h'],
                                     stderr=subprocess.STDOUT).decode()
    self.assertIn('--geoviewpoint', output)
    self.assertIn('Copy GeoViewpoint', output)


if __name__ == '__main__':
  unittest.main()
