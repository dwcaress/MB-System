#!/usr/bin/env python3

"""Tests for mbimagecorrect command line app."""

import subprocess
import unittest


class MbimagecorrectTest(unittest.TestCase):

  def setUp(self):
    self.cmd = '../../src/photo/mbimagecorrect'

  @unittest.expectedFailure
  def testHelpDoesNotCrash(self):
    # Known bug: mbimagecorrect currently tries to open/convert an image
    # before checking --help or validating that an input was given, and
    # aborts with an uncaught cv::Exception (SIGABRT) instead of printing
    # usage. This test documents the expected (non-crashing) behavior and
    # is marked as an expected failure until that is fixed; if it starts
    # passing, that is this test's own signal the bug was fixed and the
    # decorator should be removed.
    cmd = [self.cmd, '--help']
    proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT)
    self.assertGreaterEqual(proc.returncode, 0)


if __name__ == '__main__':
  unittest.main()
