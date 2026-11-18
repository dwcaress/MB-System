# mbglb2tiles: OGC 3D Tiles of levels of detail from a GLB mesh of mbmesh or mbgrd2gltf. The tileset writer, the coordinate frame,
# the GLB writer and the page writer are those of mbmesh, and the Draco compression is shared with it (src/mbdraco).
bin_PROGRAMS = mbglb2tiles

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)/src/mbio
AM_CPPFLAGS += -I$(top_srcdir)/src/mbgrd2gltf
AM_CPPFLAGS += -I$(top_srcdir)/src/mbdraco
AM_CPPFLAGS += -I$(top_srcdir)/src/mbmesh/include
AM_CPPFLAGS += -I$(top_srcdir)/src/mbglb2tiles

mbglb2tiles_CXXFLAGS = -std=c++17
mbglb2tiles_SOURCES = \
	mbglb2tiles.cpp \
	glb_reader.cpp \
	mesh_levels.cpp \
	../mbmesh/src/io/coordinate_frame.cpp \
	../mbmesh/src/io/provenance.cpp \
	../mbmesh/src/io/glb_writer.cpp \
	../mbmesh/src/io/tileset_writer.cpp \
	../mbmesh/src/io/x3dom_writer.cpp \
	../mbmesh/src/io/cesium_writer.cpp \
	../mbmesh/src/algorithms/seam_blending.cpp \
	../mbmesh/src/algorithms/marching_cubes.cpp

noinst_HEADERS = glb_reader.h mesh_levels.h

mbglb2tiles_LDADD = $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbdraco/libmbdraco.la -lpthread

check_PROGRAMS = mbglb2tiles_test_levels
TESTS = mbglb2tiles_test_levels

mbglb2tiles_test_levels_CXXFLAGS = -std=c++17
mbglb2tiles_test_levels_SOURCES = \
	tests/test_levels.cpp \
	mesh_levels.cpp \
	glb_reader.cpp \
	../mbmesh/src/io/coordinate_frame.cpp \
	../mbmesh/src/io/provenance.cpp \
	../mbmesh/src/io/glb_writer.cpp \
	../mbmesh/src/io/tileset_writer.cpp \
	../mbmesh/src/io/x3dom_writer.cpp \
	../mbmesh/src/io/cesium_writer.cpp \
	../mbmesh/src/algorithms/seam_blending.cpp \
	../mbmesh/src/algorithms/marching_cubes.cpp
mbglb2tiles_test_levels_LDADD = $(top_builddir)/src/mbdraco/libmbdraco.la -lpthread
