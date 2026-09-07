add_test(NAME mbnavadjustmerge_test COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/mbnavadjustmerge_test.py")

# src/mbnavadjust only builds the mbnavadjust GUI executable itself when
# buildGUIs is on (see src/mbnavadjust/CMakeLists.txt); mbnavadjustmerge is
# always built.
if(buildGUIs)
  add_test(NAME mbnavadjust_test COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/mbnavadjust_test.py")
endif()
