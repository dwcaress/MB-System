# WIP: copied from ../mbgrdviz and removed some lines
bin_PROGRAMS = mbgrd2gltf


AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libgmt_CPPFLAGS}

mbgrdviz_SOURCES = mbgrdviz_main.c mbgrdviz_callbacks.c mbgrdviz_creation.c
mbgrdviz_LDADD =
mbgrdviz_LDADD += ${libgmt_LIBS}
mbgrdviz_LDADD += ${libnetcdf_LIBS}
mbgrdviz_LDADD += $(LIBM)
