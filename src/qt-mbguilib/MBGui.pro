QT -= gui
QT += quick

TEMPLATE = lib
DEFINES += MBGUI_LIBRARY

CONFIG += c++11

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
    GmtGridReader.cpp \
    GmtGridSurface.cpp \
    MBGui.cpp \
    MBQuickItem.cpp \
    MBQuickView.cpp \
    QVtkItem.cpp \
    QVtkRenderer.cpp \
    Surface.cpp \
    SurfaceRenderer.cpp

HEADERS += \
    Camera.h \
    ColorMap.h \
    DisplayProperties.h \
    GmtGridReader.h \
    GmtGridSurface.h \
    MBGui_global.h \
    MBGui.h \
    MBQuickItem.h \
    MBQuickView.h \
    QVtkItem.h \
    QVtkRenderer.h \    
    Surface.h \
    SurfaceRenderer.h \
    Vertex.h \
    colorTables.h

INCLUDEPATH += /usr/local/include/vtk-8.2

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    ../../../../tmp/mbgui-model.qmodel \
    shaders/phong.frag \
    shaders/phong.vert \
    shaders/test.frag \
    shaders/test.vert
