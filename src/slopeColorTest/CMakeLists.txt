cmake_minimum_required(VERSION 3.12)
project(SlopeColorApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find VTK 9 ────────────────────────────────────────────────────────────────
#
# If VTK is installed to a non-default prefix, tell CMake where to look:
#   cmake -DVTK_DIR=/path/to/vtk/lib/cmake/vtk-9.x ..
#
find_package(VTK 9.0 REQUIRED
    COMPONENTS
        CommonCore
        CommonDataModel
        CommonExecutionModel
        FiltersCore
        FiltersGeneral       # vtkElevationFilter
        FiltersProgrammable  # vtkProgrammableFilter
        FiltersSources       # vtkSphereSource
        InteractionStyle     # vtkInteractorStyleTrackballCamera
        RenderingCore
        RenderingOpenGL2
)

if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK 9 not found. "
        "Set VTK_DIR to the directory containing VTKConfig.cmake.")
endif()

message(STATUS "Found VTK ${VTK_VERSION} at ${VTK_DIR}")

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(SlopeColorApp SlopeColorApp.cxx)

target_link_libraries(SlopeColorApp PRIVATE ${VTK_LIBRARIES})

# Required VTK boilerplate: auto-initialises factory overrides (OpenGL etc.)
vtk_module_autoinit(
    TARGETS SlopeColorApp
    MODULES ${VTK_LIBRARIES}
)

# ── Optional: copy executable to build root for convenience ───────────────────
set_target_properties(SlopeColorApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
