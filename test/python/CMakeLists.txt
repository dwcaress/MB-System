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

# Tests of mbio_py (src/python), the Python water column viewer and mbwcdview,
# with the libmbio of this build. They need python3 with numpy (the viewer test
# also uses PySide6 and matplotlib when installed) and a shared libmbio.

message("In test/python")

# the same python3 as the other tests (#!/usr/bin/env python3) before framework installs
set(Python3_FIND_FRAMEWORK LAST)
find_package(Python3 COMPONENTS Interpreter)
set(mbio_py_tests_ok FALSE)
if(Python3_FOUND AND BUILD_SHARED_LIBS)
  execute_process(COMMAND ${Python3_EXECUTABLE} -c "import numpy" RESULT_VARIABLE numpy_result
                  OUTPUT_QUIET ERROR_QUIET)
  if(numpy_result EQUAL 0)
    set(mbio_py_tests_ok TRUE)
  endif()
endif()

if(mbio_py_tests_ok)
  set(test_environment
      "MBSYSTEM_LIBMBIO=$<TARGET_FILE:mbio>"
      "PYTHONPATH=${CMAKE_SOURCE_DIR}/src/python"
      "MB_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
      "MB_BINARY_DIR=${CMAKE_BINARY_DIR}"
      "PYTHONDONTWRITEBYTECODE=1")  # no __pycache__ in the source tree

  set(tests
      mbio_py_constants_test
      mbio_py_reader_test
      mbio_py_watercolumn_test
      mbio_py_watercolumn_all_test
      mbpy_wcdview_test)
  foreach(test ${tests})
    add_test(NAME ${test} COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/${test}.py")
    set_tests_properties(${test} PROPERTIES ENVIRONMENT "${test_environment}")
  endforeach()

  if(buildQt6Tools)
    add_test(NAME mbwcdview_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbwcdview_test.py")
    set_tests_properties(mbwcdview_test PROPERTIES ENVIRONMENT
                         "${test_environment};MBWCDVIEW=$<TARGET_FILE:mbwcdview>;MBWCDVIEW_WINDOW_TEST=$<TARGET_FILE:mbwcdview_window_test>")
  endif()
else()
  message("mbio_py tests not added: they need python3 with numpy and BUILD_SHARED_LIBS=ON")
endif()
