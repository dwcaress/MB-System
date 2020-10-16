QT += quick

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        BackEnd.cpp \
        main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    BackEnd.h 

INCLUDEPATH += /usr/local/include/vtk-8.2
INCLUDEPATH += $(HOME)/projects/mb-system/MB-System/src/qt-guilib

QMAKE_RPATHDIR += ../qt-guilib/

LIBS += -L../qt-guilib -lMBGui
LIBS += -L/usr/local/lib -lgmt

unix|win32|macos: LIBS +=  -lvtkGUISupportQt-8.2 -lvtkCommonColor-8.2 -lvtkRenderingFreeType-8.2 -lvtkRenderingAnnotation-8.2 -lvtkCommonTransforms-8.2 -lvtkCommonCore-8.2 -lvtkCommonDataModel-8.2 -lvtkCommonExecutionModel-8.2 -lvtkInteractionWidgets-8.2 -lvtkInteractionStyle-8.2 -lvtkRenderingCore-8.2 -lvtkFiltersSources-8.2 -lvtkGeovisCore-8.2 -lvtkRenderingOpenGL2-8.2 -lvtkFiltersHybrid-8.2 -lvtkIOGeometry-8.2 -lvtkIOCore-8.2 -lvtkIOLegacy-8.2 -lvtkRenderingVolumeOpenGL2-8.2 -lvtkFiltersCore-8.2 -lvtkFiltersGeneral-8.2 -lvtksys-8.2
