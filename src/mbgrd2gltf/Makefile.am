bin_PROGRAMS = mbgrd2gltf

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += -I$(top_srcdir)/src/mbdraco
AM_CPPFLAGS += -I$(top_srcdir)/src/mbmesh/include

# The Draco compression is shared with mbmesh and mbglb2tiles (src/mbdraco, the convenience library libmbdraco.la).
mbgrd2gltf_CXXFLAGS = -std=c++17
# The page for CesiumJS (--cesium-html) is written by the source of mbmesh and mbglb2tiles (src/mbmesh/src/io/cesium_writer.cpp), which uses only the standard library.
mbgrd2gltf_SOURCES = main.cpp bathymetry.cpp geometry.cpp logger.cpp model.cpp options.cpp provenance.cpp viewpoint.cpp \
	../mbmesh/src/io/cesium_writer.cpp
mbgrd2gltf_LDADD =
mbgrd2gltf_LDADD += $(top_builddir)/src/mbdraco/libmbdraco.la
mbgrd2gltf_LDADD += ${libnetcdf_LIBS}

# The scale object of the pages is a header shared with mbmesh and mbglb2tiles; its test needs no other source.
check_PROGRAMS = mbgrd2gltf_test_scale_axes
TESTS = mbgrd2gltf_test_scale_axes
mbgrd2gltf_test_scale_axes_CXXFLAGS = -std=c++17
mbgrd2gltf_test_scale_axes_SOURCES = tests/test_scale_axes.cpp
