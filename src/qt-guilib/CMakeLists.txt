#
# Variables are passed from top-level CMakeLists.txt

message("in qt-guilib")

find_package(Qt5 COMPONENTS Core Gui Quick REQUIRED)
if (Qt5_FOUND)
  message("Found Qt5 package")
else()
  message("Qt5 package not found")
endif()

#[[
### TEST TEST
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
]]

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
                           ${GMT_INCLUDE_DIRS}
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
                           ${GMT_INCLUDE_DIRS}                           
                           ${Qt5Gui_INCLUDE_DIRS}
                           ${Qt5Quick_INCLUDE_DIRS})                           

target_link_libraries(MBGuiShared ${EXTRA_LIBRARIES})

