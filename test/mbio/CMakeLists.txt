message("In test/mbio")
# Use 'Find or Fetch' pattern to access googletest libraries.
# googletest is accessible as source and compiled library through homebrew
# (use find_package), but is available as source-only on ubuntu/debian
# (use Fetch)

# 1. Look for pre-installed GTest libraries
find_package(GTest QUIET)

if (NOT GTest_FOUND)
  message(STATUS 'googletest library not installed on system; Fetch it...')
  
  # 2. Include the FetchContent module
  include(FetchContent)

  # 3. Declare the dependency
  FetchContent_Declare(
     googletest
     URL https://github.com/google/googletest/archive/refs/heads/main.zip
  )
  
  # 4. Download and make the targets available
  message('call FetchContent_MakeAvailable(googletest)')
  FetchContent_MakeAvailable(googletest)

else()
  message('Using system-installed googletest')

endif()

set(tests mb_defaults_test mb_error_test mb_format_test mb_mem_test
          mb_read_init_test mb_time_test)

foreach(test ${tests})
  add_executable(${test} ${test}.cc)
  target_include_directories(${test} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../src)
  target_link_libraries(${test} PRIVATE mbio GTest::gmock_main)
  
  add_test(NAME ${test} COMMAND ${test})
endforeach()
