##find_package(GTest REQUIRED)
message("In test/mbio")

set(tests mb_defaults_test mb_error_test mb_format_test mb_mem_test
          mb_read_init_test mb_time_test)

foreach(test ${tests})
  add_executable(${test} ${test}.cc)
  target_include_directories(${test} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../src)
  target_link_libraries(${test} PRIVATE mbio GTest::gmock_main)
  add_test(NAME ${test} COMMAND ${test})
endforeach()
