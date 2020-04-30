bin_PROGRAMS = mbqttest

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

AM_LDFLAGS = -Wl,-rpath,${qt_DIR} ${qt_LIBS}

LIBS += ${qt_LIBS}
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += $(LIBM)

mbqttest_SOURCES =
mbqttest_SOURCES += datavisualizationqml2/abstractdeclarative.cpp
mbqttest_SOURCES += datavisualizationqml2/colorgradient.cpp
mbqttest_SOURCES += datavisualizationqml2/datavisualizationqml2_plugin.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativebars.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativecolor.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativerendernode.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativescatter.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativescene.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativeseries.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativesurface.cpp
mbqttest_SOURCES += datavisualizationqml2/declarativetheme.cpp
mbqttest_SOURCES += datavisualizationqml2/enumtostringmap.cpp
mbqttest_SOURCES += datavisualizationqml2/glstatestore.cpp
mbqttest_SOURCES += main.cpp
mbqttest_SOURCES += MyTimer.cpp
mbqttest_SOURCES += TopographicSeries.cpp
mbqttest_SOURCES += BackEnd.cpp
mbqttest_SOURCES += moc_BackEnd.cpp
mbqttest_SOURCES += moc_MyTimer.cpp
mbqttest_SOURCES += moc_TopographicSeries.cpp
mbqttest_SOURCES += moc_abstractdeclarative_p.cpp
mbqttest_SOURCES += moc_colorgradient_p.cpp
mbqttest_SOURCES += moc_datavisualizationqml2_plugin.cpp
mbqttest_SOURCES += moc_declarativebars_p.cpp
mbqttest_SOURCES += moc_declarativecolor_p.cpp
mbqttest_SOURCES += moc_declarativerendernode_p.cpp
mbqttest_SOURCES += moc_declarativescatter_p.cpp
mbqttest_SOURCES += moc_declarativescene_p.cpp
mbqttest_SOURCES += moc_declarativeseries_p.cpp
mbqttest_SOURCES += moc_declarativesurface_p.cpp
mbqttest_SOURCES += moc_declarativetheme_p.cpp
mbqttest_SOURCES += moc_glstatestore_p.cpp
mbqttest_SOURCES += qrc_qml.cpp

BUILT_SOURCES =
BUILT_SOURCES += moc_BackEnd.cpp
BUILT_SOURCES += moc_MyTimer.cpp
BUILT_SOURCES += moc_TopographicSeries.cpp
BUILT_SOURCES += moc_abstractdeclarative_p.cpp
BUILT_SOURCES += moc_colorgradient_p.cpp
BUILT_SOURCES += moc_datavisualizationqml2_plugin.cpp
BUILT_SOURCES += moc_declarativebars_p.cpp
BUILT_SOURCES += moc_declarativecolor_p.cpp
BUILT_SOURCES += moc_declarativerendernode_p.cpp
BUILT_SOURCES += moc_declarativescatter_p.cpp
BUILT_SOURCES += moc_declarativescene_p.cpp
BUILT_SOURCES += moc_declarativeseries_p.cpp
BUILT_SOURCES += moc_declarativesurface_p.cpp
BUILT_SOURCES += moc_declarativetheme_p.cpp
BUILT_SOURCES += moc_glstatestore_p.cpp
BUILT_SOURCES += qrc_qml.cpp
BUILT_SOURCES += moc_predefs.h

CLEANFILES =
CLEANFILES += moc_BackEnd.cpp
CLEANFILES += moc_MyTimer.cpp
CLEANFILES += moc_TopographicSeries.cpp
CLEANFILES += moc_abstractdeclarative_p.cpp
CLEANFILES += moc_colorgradient_p.cpp
CLEANFILES += moc_datavisualizationqml2_plugin.cpp
CLEANFILES += moc_declarativebars_p.cpp
CLEANFILES += moc_declarativecolor_p.cpp
CLEANFILES += moc_declarativerendernode_p.cpp
CLEANFILES += moc_declarativescatter_p.cpp
CLEANFILES += moc_declarativescene_p.cpp
CLEANFILES += moc_declarativeseries_p.cpp
CLEANFILES += moc_declarativesurface_p.cpp
CLEANFILES += moc_declarativetheme_p.cpp
CLEANFILES += moc_glstatestore_p.cpp
CLEANFILES += qrc_qml.cpp
CLEANFILES += moc_predefs.h

DISTCLEANFILES =
DISTCLEANFILES += moc_BackEnd.cpp
DISTCLEANFILES += moc_MyTimer.cpp
DISTCLEANFILES += moc_TopographicSeries.cpp
DISTCLEANFILES += moc_abstractdeclarative_p.cpp
DISTCLEANFILES += moc_colorgradient_p.cpp
DISTCLEANFILES += moc_datavisualizationqml2_plugin.cpp
DISTCLEANFILES += moc_declarativebars_p.cpp
DISTCLEANFILES += moc_declarativecolor_p.cpp
DISTCLEANFILES += moc_declarativerendernode_p.cpp
DISTCLEANFILES += moc_declarativescatter_p.cpp
DISTCLEANFILES += moc_declarativescene_p.cpp
DISTCLEANFILES += moc_declarativeseries_p.cpp
DISTCLEANFILES += moc_declarativesurface_p.cpp
DISTCLEANFILES += moc_declarativetheme_p.cpp
DISTCLEANFILES += moc_glstatestore_p.cpp
DISTCLEANFILES += qrc_qml.cpp
DISTCLEANFILES += moc_predefs.h

moc_BackEnd.cpp: BackEnd.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_MyTimer.cpp: MyTimer.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_TopographicSeries.cpp: TopographicSeries.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_abstractdeclarative_p.cpp: datavisualizationqml2/abstractdeclarative_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_colorgradient_p.cpp: datavisualizationqml2/colorgradient_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_datavisualizationqml2_plugin.cpp: datavisualizationqml2/datavisualizationqml2_plugin.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativebars_p.cpp: datavisualizationqml2/declarativebars_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativecolor_p.cpp: datavisualizationqml2/declarativecolor_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativerendernode_p.cpp: datavisualizationqml2/declarativerendernode_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativescatter_p.cpp: datavisualizationqml2/declarativescatter_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativescene_p.cpp: datavisualizationqml2/declarativescene_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativeseries_p.cpp: datavisualizationqml2/declarativeseries_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_declarativesurface_p.cpp: datavisualizationqml2/declarativesurface_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all compiler_moc_header_make_all

moc_declarativetheme_p.cpp: datavisualizationqml2/declarativetheme_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_glstatestore_p.cpp: datavisualizationqml2/glstatestore_p.h
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

moc_predefs.h:
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all

qrc_qml.cpp: qml.qrc
	make -f Makefile.qmake compiler_rcc_make_all
	make -f Makefile.qmake compiler_moc_header_make_all
