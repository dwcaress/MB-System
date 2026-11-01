#------------------------------------------------------------------------------
#    The MB-system:  Makefile.am   1 November 2026
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
#------------------------------------------------------------------------------

# mbio_py (Python API to MBIO), its examples and the Python water column viewer:
# pure Python installed to $(datadir)/mbsystem/python, launcher mbpy_wcdview to bin.

bin_SCRIPTS = mbpy_wcdview

mbiopydir = $(datadir)/mbsystem/python/mbio_py
mbiopy_DATA = \
	mbio_py/__init__.py \
	mbio_py/_lib.py \
	mbio_py/constants.py \
	mbio_py/errors.py \
	mbio_py/raytrace.py \
	mbio_py/reader.py \
	mbio_py/survey.py \
	mbio_py/watercolumn.py
nodist_mbiopy_DATA = mbio_py/_config.py

mbiopyexamplesdir = $(datadir)/mbsystem/python/examples
mbiopyexamples_DATA = \
	examples/_common.py \
	examples/plot_bottom.py \
	examples/plot_nav.py \
	examples/plot_paired.py \
	examples/plot_wc_ping.py \
	examples/print_records.py \
	examples/print_wc_ping.py

wcdviewerdir = $(datadir)/mbsystem/python/wcd_viewer
wcdviewer_DATA = wcd_viewer/wcd_viewer.py

mbiopydocdir = $(datadir)/mbsystem/python
mbiopydoc_DATA = README.md

# libmbio is installed by libtool in $(libdir); mbio_py tries the usual file names there
mbio_py/_config.py: mbio_py/_config.py.in Makefile
	$(MKDIR_P) mbio_py
	sed -e 's|@LIBMBIO_PATH@||' -e 's|@LIBMBIO_DIR@|$(libdir)|' $(srcdir)/mbio_py/_config.py.in > $@

EXTRA_DIST = $(bin_SCRIPTS) $(mbiopy_DATA) $(mbiopyexamples_DATA) $(wcdviewer_DATA) $(mbiopydoc_DATA) \
	mbio_py/_config.py.in CMakeLists.txt

CLEANFILES = mbio_py/_config.py
