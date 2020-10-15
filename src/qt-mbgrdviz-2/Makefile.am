bin_PROGRAMS = qt-mbgrdviz-2

include_HEADERS = BackEnd.h \
  MyTimer.h \
  TopographicSeries.h \
  datavisualizationqml2/abstractdeclarative_p.h \
  datavisualizationqml2/colorgradient_p.h \
  datavisualizationqml2/datavisualizationqml2_plugin.h \
  datavisualizationqml2/declarativebars_p.h \
  datavisualizationqml2/declarativecolor_p.h \
  datavisualizationqml2/declarativerendernode_p.h \
  datavisualizationqml2/declarativescatter_p.h \
  datavisualizationqml2/declarativescene_p.h \
  datavisualizationqml2/declarativeseries_p.h \
  datavisualizationqml2/declarativesurface_p.h \
  datavisualizationqml2/declarativetheme_p.h \
  datavisualizationqml2/enumtostringmap_p.h \
  datavisualizationqml2/glstatestore_p.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS} ${qt_CPPFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += ${qt_CPPFLAGS}
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}

AM_LDFLAGS = ${qt_LIBS}

LIBS += ${qt_LIBS}
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += $(LIBM)

qt-mbgrdviz-2_SOURCES =
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/abstractdeclarative.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/colorgradient.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/datavisualizationqml2_plugin.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativebars.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativecolor.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativerendernode.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativescatter.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativescene.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativeseries.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativesurface.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/declarativetheme.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/enumtostringmap.cpp \
qt-mbgrdviz-2_SOURCES += datavisualizationqml2/glstatestore.cpp \
qt-mbgrdviz-2_SOURCES += main.cpp \
qt-mbgrdviz-2_SOURCES += MyTimer.cpp \
qt-mbgrdviz-2_SOURCES += TopographicSeries.cpp \
qt-mbgrdviz-2_SOURCES += BackEnd.cpp
