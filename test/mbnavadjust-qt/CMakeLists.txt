#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   10 November 2026
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

# Test of mbnavadjust-qt on a synthetic project, without windows.

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbnavadjust_qt_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbnavadjust_qt_test.py")
  set_tests_properties(
    mbnavadjust_qt_test
    PROPERTIES ENVIRONMENT
               "MBNAVADJUST_QT=$<TARGET_FILE:mbnavadjust-qt>;MBNAVADJUSTMERGE=$<TARGET_FILE:mbnavadjustmerge>;MBINFO=$<TARGET_FILE:mbinfo>;SYNTH_KMALL_DIR=${CMAKE_SOURCE_DIR}/test/python")
endif()
