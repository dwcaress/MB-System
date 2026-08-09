bin_PROGRAMS = mbmesh

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)/src/mbio
AM_CPPFLAGS += -I$(top_srcdir)/src/mbgrd2gltf
AM_CPPFLAGS += -I$(top_srcdir)/src/mbmesh/include

mbmesh_CXXFLAGS = -std=c++17
mbmesh_SOURCES = \
	mbmesh.cpp \
	settings.cpp \
	src/io/datalist_reader.cpp \
	src/io/xyz_writer.cpp \
	src/io/glb_writer.cpp \
	src/io/x3dom_writer.cpp \
	src/algorithms/point_decimation.cpp \
	src/algorithms/normal_estimation.cpp \
	src/algorithms/screened_poisson.cpp \
	src/algorithms/marching_cubes.cpp \
	src/algorithms/support_trimming.cpp

mbmesh_LDADD = $(top_builddir)/src/mbio/libmbio.la

check_PROGRAMS = mbmesh_test_support_trimming
TESTS = mbmesh_test_support_trimming

mbmesh_test_support_trimming_CXXFLAGS = -std=c++17
mbmesh_test_support_trimming_SOURCES = \
	tests/test_support_trimming.cpp \
	src/algorithms/support_trimming.cpp
