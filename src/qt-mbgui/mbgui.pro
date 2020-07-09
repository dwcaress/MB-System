# The configure script copies this file to qt-mbguilib.pro and then modifies it
# with sed to include the relevant GMT compile and load flags
# The configure script then runs qmake to generate a Makefile.qmake while
# also generating a Makefile that calls "make -f Makefile.qmake"

QT -= gui
QT += quick

TEMPLATE = lib
DEFINES += MBGUI_LIBRARY

CONFIG += c++11
CONFIG += force_debug_info

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camera.cpp \
    ColorMap.cpp \
    GmtGridSurface.cpp \
    MBGui.cpp \
    MBQuickItem.cpp \
    MBQuickView.cpp \
    Surface.cpp \
    SurfaceRenderer.cpp

HEADERS += \
    Camera.h \
    ColorMap.h \
    GmtGridSurface.h \
    MBGui_global.h \
    MBGui.h \
    MBQuickItem.h \
    MBQuickView.h \
    Surface.h \
    SurfaceRenderer.h \
    Vertex.h \
    colorTables.h

DISTFILES += \
    glsl-shaders/phong.frag \
    glsl-shaders/phong.vert \
    glsl-shaders/test.frag \
    glsl-shaders/test.vert

INCLUDEPATH += /usr/local/include/gmt
INCLUDEPATH += /opt/X11/include

LIBS += -L/usr/local/lib -lgmt
LIBS += -L/opt/X11/lib -lGL -lGLU

RESOURCES += \
    resources.qrc

# Default rules for deployment.
target.path = /usr/local/lib
#target.files =
INSTALLS += target
