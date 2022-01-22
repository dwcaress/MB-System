cmake_minimum_required(VERSION 3.13)
project("mbgrd2gltf")

list(APPEND CMAKE_PREFIX_PATH "/usr/local/lib64/cmake/netCDF/")

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
# https://stackoverflow.com/a/22177285/1281657
#file(GLOB_RECURSE SOURCES "*.cpp")
set(SOURCE_FILES bathymetry.cpp  compression.cpp  geometry.cpp  main.cpp  model.cpp  options.cpp)
set(INCLUDE_DIRS "include" "tinygltf" ${netCDF_INCLUDE_DIR})

# universal objects
add_library(common_objects OBJECT ${SOURCE_FILES})
target_include_directories(common_objects PRIVATE ${INCLUDE_DIRS})

# main executable
# https://stackoverflow.com/a/22177285/1281657
add_executable(${PROJECT_NAME} "main.cpp" ${SOURCE_FILES})
#add_executable(${PROJECT_NAME} "main.cpp" $<TARGET_OBJECTS:common_objects>)
list(APPEND TARGETS ${PROJECT_NAME})

# getting tests
file(GLOB TEST_SRCS "test/*.cpp")

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
