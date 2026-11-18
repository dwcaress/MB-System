#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   1 November 2026
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
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

# mbio_py (Python API to MBIO through ctypes), its examples and the Python
# water column viewer: pure Python, installed to share/mbsystem/python; the
# launcher mbpy_wcdview goes to bin. Nothing is compiled.

message("In src/python")

set(mbpy_install_dir ${levitusDir}/python)

# where the installed libmbio is, written into mbio_py/_config.py
if(WIN32)
  set(LIBMBIO_DIR ${CMAKE_INSTALL_FULL_BINDIR})
else()
  set(LIBMBIO_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()
if(BUILD_SHARED_LIBS)
  set(LIBMBIO_PATH ${LIBMBIO_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}mbio${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
  message("mbio_py needs a shared libmbio (BUILD_SHARED_LIBS=ON); installed without a library path")
  set(LIBMBIO_PATH "")
endif()
configure_file(mbio_py/_config.py.in ${CMAKE_CURRENT_BINARY_DIR}/mbio_py/_config.py @ONLY)

set(mbio_py_modules
    mbio_py/__init__.py
    mbio_py/_lib.py
    mbio_py/constants.py
    mbio_py/errors.py
    mbio_py/raytrace.py
    mbio_py/reader.py
    mbio_py/survey.py
    mbio_py/watercolumn.py)

set(mbio_py_examples
    examples/_common.py
    examples/plot_bottom.py
    examples/plot_nav.py
    examples/plot_paired.py
    examples/plot_wc_ping.py
    examples/print_records.py
    examples/print_wc_ping.py)

install(FILES ${mbio_py_modules} ${CMAKE_CURRENT_BINARY_DIR}/mbio_py/_config.py
        DESTINATION ${mbpy_install_dir}/mbio_py)
install(FILES ${mbio_py_examples} DESTINATION ${mbpy_install_dir}/examples)
install(FILES wcd_viewer/wcd_viewer.py DESTINATION ${mbpy_install_dir}/wcd_viewer)
install(FILES README.md DESTINATION ${mbpy_install_dir})
install(PROGRAMS mbpy_wcdview DESTINATION ${CMAKE_INSTALL_BINDIR})
