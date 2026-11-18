#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbio_py_constants_test.py  1/11/2026
#
#    Copyright (c) 2026 by
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------
"""The constants of mbio_py must equal those in src/mbio/mb_status.h and mb_define.h."""

import os
import re
import unittest

import mbio_py
from mbio_py import constants as K


class ConstantsTest(unittest.TestCase):

  def test_constants_equal_headers(self):
    source = os.environ["MB_SOURCE_DIR"]
    defines = {}
    for header in ("mb_status.h", "mb_define.h"):
      text = open(os.path.join(source, "src", "mbio", header)).read()
      for m in re.finditer(r"^#define\s+(MB_\w+)\s+(-?(?:0x)?[0-9A-Fa-f]+)\b", text, re.M):
        defines.setdefault(m.group(1), int(m.group(2), 0))
    names = [n for n in dir(K) if n.startswith("MB_")]
    self.assertGreater(len(names), 30)
    for name in names:
      self.assertIn(name, defines, name)
      self.assertEqual(getattr(K, name), defines[name], name)

  def test_library_loaded(self):
    self.assertTrue(os.path.exists(mbio_py.lib_path), mbio_py.lib_path)


if __name__ == "__main__":
  unittest.main()
