cmake_minimum_required(VERSION 3.10)
project(mbmesh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
find_package(netCDF REQUIRED)
include_directories(${netCDF_INCLUDE_DIRS})

add_executable(mbmesh
    main.cpp
    bathymetry.cpp
    geometry_3d.cpp
    gltf_writer.cpp
)

target_link_libraries(mbmesh
    ${netCDF_LIBRARIES}
    m
)

target_include_directories(mbmesh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
