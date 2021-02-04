#
# Variables are passed from top-level CMakeLists.txt

message("in qt-guilib")


set(src
    Camera.cpp
    MBQuickItem.cpp
    moc_SurfaceRenderer.cpp
    ColorMap.cpp
    MBQuickView.cpp
    qrc_resources.cpp
    QVtkItem.cpp
    GmtGridReader.cpp
    MBQuickItem.cpp
    QVtkRenderer.cpp
    GmtGridSurface.cpp
    moc_MBQuickView.cpp
    Surface.cpp
    MBGui.cpp
    moc_QVtkItem.cpp
    SurfaceRenderer.cpp)


# Static library
add_library(MBGui
            STATIC
            ${src})
            
target_include_directories(MBGui
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${VTK_INCLUDE_DIR}                           
                           ${Qt5Gui_INCLUDE_DIRS}
                           ${Qt5Quick_INCLUDE_DIRS})

set_target_properties(MBGui PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Shared library
add_library(MBGuiShared
            SHARED
            ${src})

set_target_properties(MBGuiShared PROPERTIES OUTPUT_NAME MBGui)

target_include_directories(MBGuiShared
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${VTK_INCLUDE_DIR}                           
                           ${Qt5Gui_INCLUDE_DIRS}
                           ${Qt5Quick_INCLUDE_DIRS})                           

target_link_libraries(MBGuiShared ${EXTRA_LIBRARIES})

