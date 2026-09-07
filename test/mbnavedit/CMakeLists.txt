set(tests mbnavedit_test)

foreach(test ${tests})
  add_test(NAME ${test} COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${test}.py")
endforeach()
