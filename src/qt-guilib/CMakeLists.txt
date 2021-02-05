#
# Variables are passed from top-level CMakeLists.txt

message("in qt-guilib")

# Run moc when needed
set(CMAKE_AUTOMOC ON)

# Run rcc when needed
set(CMAKE_AUTORCC ON)

# Run uic automatically when needed.
set(CMAKE_AUTOUIC ON)

set(src
    Camera.cpp
    MBQuickItem.cpp
    ColorMap.cpp
    MBQuickView.cpp
    QVtkItem.cpp
    GmtGridReader.cpp
    MBQuickItem.cpp
    QVtkRenderer.cpp
    GmtGridSurface.cpp
    Surface.cpp
    MBGui.cpp
    SurfaceRenderer.cpp
    resources.qrc)


# Static library
add_library(MBGui
            STATIC
            ${src})
            
target_include_directories(MBGui
                           PUBLIC
                           .
			   ${OPENGL_INCLUDE_DIR}
                           ${GMT_INCLUDE_DIR}
                           ${VTK_INCLUDE_DIR}
                           ${QT5_INCLUDE_DIR})

message("*** OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")

set_target_properties(MBGui PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Shared library
add_library(MBGuiShared
            SHARED
            ${src})

set_target_properties(MBGuiShared PROPERTIES OUTPUT_NAME MBGui)

target_include_directories(MBGuiShared
                           PUBLIC
                           .
			   ${OPENGL_INCLUDE_DIR}			   
                           ${GMT_INCLUDE_DIR}
                           ${VTK_INCLUDE_DIR}                           
                           ${QT5_INCLUDE_DIR})

target_link_libraries(MBGuiShared ${EXTRA_LIBRARIES})

