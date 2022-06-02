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
    SwathReader.cpp    
    MBQuickItem.cpp
    QVtkRenderer.cpp
    GmtGridSurface.cpp
    Surface.cpp
    MBGui.cpp
    SurfaceRenderer.cpp
    PickerInteractorStyle.cpp
    Utilities.cpp    
    resources.qrc)


# Static library
add_library(MBGui
            STATIC
            ${src})

# VTK 8.x does not fully support find_package()
### target_link_libraries(MBGui ${VTK_LIBRARIES}) 

target_include_directories(MBGui
                           PUBLIC
                           .
                           ${MB_HOME}/src/mbio
			   ${OPENGL_INCLUDE_DIRS}
                           ${GMT_INCLUDE_DIRS}
                           ${QT5_INCLUDE_DIRS}
                           ${VTK_INCLUDE_DIRS})

message("*** OPENGL_INCLUDE_DIRS: ${OPENGL_INCLUDE_DIRS}")

set_target_properties(MBGui PROPERTIES POSITION_INDEPENDENT_CODE ON)


# Shared library
add_library(MBGuiShared
            SHARED
            ${src})

set_target_properties(MBGuiShared PROPERTIES OUTPUT_NAME MBGui)

target_include_directories(MBGuiShared
                           PUBLIC
                           .
			   ${OPENGL_INCLUDE_DIRS}			   
                           ${GMT_INCLUDE_DIRS}
                           ${QT5_INCLUDE_DIRS}
                           ${VTK_INCLUDE_DIRS})

message("qt-guilib - EXTRA_LIBRARIES: ${EXTRA_LIBRARIES}")
target_link_libraries(MBGuiShared ${VTK_LIBRARIES} ${EXTRA_LIBRARIES})

# Install
install(TARGETS MBGui DESTINATION cmake-example/lib)
install(TARGETS MBGuiShared DESTINATION cmake-example/lib)

