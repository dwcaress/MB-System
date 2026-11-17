bin_PROGRAMS = mbmesh

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)/src/mbio
AM_CPPFLAGS += -I$(top_srcdir)/src/mbgrd2gltf
AM_CPPFLAGS += -I$(top_srcdir)/src/mbdraco
AM_CPPFLAGS += -I$(top_srcdir)/src/mbmesh/include

mbmesh_CXXFLAGS = -std=c++17
mbmesh_SOURCES = \
	mbmesh.cpp \
	settings.cpp \
	src/io/coordinate_frame.cpp \
	src/io/datalist_reader.cpp \
	src/io/point_spill.cpp \
	src/io/xyz_writer.cpp \
	src/io/track.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp \
	src/io/tileset_writer.cpp \
	src/io/x3dom_writer.cpp \
	src/io/cesium_writer.cpp \
	src/algorithms/point_decimation.cpp \
	src/algorithms/normal_estimation.cpp \
	src/algorithms/screened_poisson.cpp \
	src/algorithms/marching_cubes.cpp \
	src/algorithms/support_trimming.cpp \
	src/algorithms/seam_blending.cpp \
	src/algorithms/reconstruction.cpp

mbmesh_LDADD = $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbdraco/libmbdraco.la -lpthread

# The unit tests are those of the CMake build (mbmesh_test.py, which runs the program, is run by CMake only).
check_PROGRAMS = mbmesh_test_support_trimming mbmesh_test_settings mbmesh_test_coverage_footprint mbmesh_test_seam_blending mbmesh_test_point_spill mbmesh_test_coordinate_frame mbmesh_test_chunking mbmesh_test_track mbmesh_test_x3dom_writer mbmesh_test_cesium_writer mbmesh_test_tileset mbmesh_test_provenance mbmesh_test_marching_cubes mbmesh_test_normal_estimation mbmesh_test_draco_mesh
TESTS = $(check_PROGRAMS)

mbmesh_test_support_trimming_CXXFLAGS = -std=c++17
mbmesh_test_support_trimming_SOURCES = \
	tests/test_support_trimming.cpp \
	src/algorithms/support_trimming.cpp \
	src/algorithms/screened_poisson.cpp \
	src/algorithms/marching_cubes.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp
mbmesh_test_support_trimming_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

mbmesh_test_settings_CXXFLAGS = -std=c++17
mbmesh_test_settings_SOURCES = \
	tests/test_settings.cpp \
	settings.cpp \
	src/io/coordinate_frame.cpp \
	src/io/datalist_reader.cpp \
	src/io/point_spill.cpp \
	src/algorithms/point_decimation.cpp \
	src/algorithms/normal_estimation.cpp \
	src/algorithms/screened_poisson.cpp \
	src/algorithms/marching_cubes.cpp \
	src/algorithms/support_trimming.cpp \
	src/algorithms/seam_blending.cpp \
	src/algorithms/reconstruction.cpp
mbmesh_test_settings_LDADD = $(top_builddir)/src/mbio/libmbio.la -lpthread

mbmesh_test_coverage_footprint_CXXFLAGS = -std=c++17
mbmesh_test_coverage_footprint_SOURCES = \
	tests/test_coverage_footprint.cpp \
	src/algorithms/support_trimming.cpp

mbmesh_test_seam_blending_CXXFLAGS = -std=c++17
mbmesh_test_seam_blending_SOURCES = \
	tests/test_seam_blending.cpp \
	src/algorithms/seam_blending.cpp \
	src/algorithms/marching_cubes.cpp

mbmesh_test_point_spill_CXXFLAGS = -std=c++17
mbmesh_test_point_spill_SOURCES = \
	tests/test_point_spill.cpp \
	src/io/point_spill.cpp

mbmesh_test_coordinate_frame_CXXFLAGS = -std=c++17
mbmesh_test_coordinate_frame_SOURCES = \
	tests/test_coordinate_frame.cpp \
	src/io/coordinate_frame.cpp

mbmesh_test_chunking_CXXFLAGS = -std=c++17
mbmesh_test_chunking_SOURCES = \
	tests/test_chunking.cpp \
	src/io/point_spill.cpp \
	src/algorithms/reconstruction.cpp \
	src/algorithms/seam_blending.cpp \
	src/algorithms/normal_estimation.cpp \
	src/algorithms/screened_poisson.cpp \
	src/algorithms/marching_cubes.cpp \
	src/algorithms/support_trimming.cpp
mbmesh_test_chunking_LDADD = -lpthread

mbmesh_test_track_CXXFLAGS = -std=c++17
mbmesh_test_track_SOURCES = \
	tests/test_track.cpp \
	src/io/track.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp
mbmesh_test_track_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

mbmesh_test_x3dom_writer_CXXFLAGS = -std=c++17
mbmesh_test_x3dom_writer_SOURCES = \
	tests/test_x3dom_writer.cpp \
	src/io/coordinate_frame.cpp \
	src/io/provenance.cpp \
	src/io/x3dom_writer.cpp

mbmesh_test_cesium_writer_CXXFLAGS = -std=c++17
mbmesh_test_cesium_writer_SOURCES = \
	tests/test_cesium_writer.cpp \
	src/io/cesium_writer.cpp

mbmesh_test_tileset_CXXFLAGS = -std=c++17
mbmesh_test_tileset_SOURCES = \
	tests/test_tileset.cpp \
	src/io/coordinate_frame.cpp \
	src/io/provenance.cpp \
	src/io/tileset_writer.cpp

mbmesh_test_provenance_CXXFLAGS = -std=c++17
mbmesh_test_provenance_SOURCES = \
	tests/test_provenance.cpp \
	src/io/coordinate_frame.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp \
	src/io/x3dom_writer.cpp
mbmesh_test_provenance_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

mbmesh_test_marching_cubes_CXXFLAGS = -std=c++17
mbmesh_test_marching_cubes_SOURCES = \
	tests/test_marching_cubes.cpp \
	src/algorithms/marching_cubes.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp
mbmesh_test_marching_cubes_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

mbmesh_test_normal_estimation_CXXFLAGS = -std=c++17
mbmesh_test_normal_estimation_SOURCES = \
	tests/test_normal_estimation.cpp \
	src/algorithms/normal_estimation.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp
mbmesh_test_normal_estimation_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

mbmesh_test_draco_mesh_CXXFLAGS = -std=c++17
mbmesh_test_draco_mesh_SOURCES = \
	tests/test_draco_mesh.cpp \
	src/io/provenance.cpp \
	src/io/glb_writer.cpp
mbmesh_test_draco_mesh_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la

noinst_HEADERS = \
	settings.h \
	include/algorithms/coverage_footprint.h \
	include/algorithms/marching_cubes.h \
	include/algorithms/normal_estimation.h \
	include/algorithms/point_decimation.h \
	include/algorithms/reconstruction.h \
	include/algorithms/screened_poisson.h \
	include/algorithms/seam_blending.h \
	include/algorithms/support_trimming.h \
	include/data_types/geometry.h \
	include/data_types/swathfile.h \
	include/io/coordinate_frame.h \
	include/io/datalist_reader.h \
	include/io/cesium_convert.h \
	include/io/cesium_writer.h \
	include/io/glb_writer.h \
	include/io/point_spill.h \
	include/io/provenance.h \
	include/io/tileset_writer.h \
	include/io/track.h \
	include/io/x3dom_writer.h \
	include/io/xyz_writer.h \
	include/math/eigen.h \
	include/math/kdtree.h \
	include/math/mat3.h \
	include/math/vec2.h \
	include/math/vec3.h \
	include/util/logger.h \
	include/util/memory.h \
	include/util/parallel.h \
	include/util/stopwatch.h
