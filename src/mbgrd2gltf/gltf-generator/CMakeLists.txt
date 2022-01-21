cmake_minimum_required(VERSION 3.13)
project("grd-to-gltf")

list(APPEND CMAKE_PREFIX_PATH "/home/vagrant/Downloads/netcdf-cxx4/build")

# getting libraries
find_package(netCDFCxx CONFIG REQUIRED)

if (WIN32)
	add_library(netcdf SHARED IMPORTED)
	set_target_properties(netcdf PROPERTIES IMPORTED_IMPLIB "${netCDF_LIB_DIR}/netcdf.lib")
	set(LIBRARIES "netcdf")
else()
	set(LIBRARIES "netcdf")
endif()

if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -pedantic -std=c++11)
endif()

# getting sources & include dirs
file(GLOB_RECURSE SOURCES "src/stoqs/*.cpp")
set(INCLUDE_DIRS "include" "lib/tinygltf" ${netCDF_INCLUDE_DIR})

# universal objects
add_library(common_objects OBJECT ${SOURCES})
target_include_directories(common_objects PRIVATE ${INCLUDE_DIRS})

# main executable
add_executable(${PROJECT_NAME} "src/main.cpp" $<TARGET_OBJECTS:common_objects>)
list(APPEND TARGETS ${PROJECT_NAME})

# getting tests
file(GLOB TEST_SRCS "src/test/*.cpp")

# creating executables for tests
foreach(TEST ${TEST_SRCS})
	get_filename_component(TEST_TARGET_NAME ${TEST} NAME_WE)
	add_executable(${TEST_TARGET_NAME} ${TEST} $<TARGET_OBJECTS:common_objects>)
	list(APPEND TARGETS ${TEST_TARGET_NAME})
endforeach()

# configuring executables
foreach(TARGET ${TARGETS})
	set_target_properties(${TARGET} PROPERTIES C_STANDARD 11 CXX_STANDARD 11)
	target_include_directories(${TARGET} PRIVATE ${INCLUDE_DIRS})
	target_link_libraries(${TARGET} ${LIBRARIES})
endforeach()
