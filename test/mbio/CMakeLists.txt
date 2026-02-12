##find_package(GTest REQUIRED)   # If GTest is dependency
message("In test/mbio")

# 1. Include the FetchContent module
include(FetchContent)

# 2. Declare the dependency
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# 3. Download and make the targets available
message('call FetchContent_MakeAvailable(googletest)')
FetchContent_MakeAvailable(googletest)

set(tests mb_defaults_test mb_error_test mb_format_test mb_mem_test
          mb_read_init_test mb_time_test)

foreach(test ${tests})
  add_executable(${test} ${test}.cc)
  target_include_directories(${test} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../src)
  target_link_libraries(${test} PRIVATE mbio gmock_main)
  
  add_test(NAME ${test} COMMAND ${test})
endforeach()
