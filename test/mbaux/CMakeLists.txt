message("In test/mbaux")

find_package(GDAL CONFIG)
if (NOT GDAL_FOUND)
  message(STATUS "GDAL package config file GDALConfig.cmake not found, looking for FindGDAL.cmake file...")
  find_package(GDAL REQUIRED)
endif (NOT GDAL_FOUND)

set(tests mb_cube_test mb_gisfile_test)

foreach(test ${tests})
  add_executable(${test} ${test}.cc)
  target_compile_features(${test} PRIVATE cxx_std_17)
  target_include_directories(${test} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../src)
  target_link_libraries(${test} PRIVATE mbaux mbio GDAL::GDAL GTest::gmock_main)
  add_test(NAME ${test} COMMAND ${test})
endforeach()
