set(tests
    mbgetphotocorrection_test
    mbimagecorrect_test
    mbimagelist_test
    mbphotogrammetry_test
    mbphotomosaic_test
    mbtiff2png_test)

foreach(test ${tests})
  add_test(NAME ${test} COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${test}.py")
endforeach()
