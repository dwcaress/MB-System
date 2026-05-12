# SlopeColorApp — VTK 9 Elevation + Slope Darkening Demo

## What it demonstrates

Colours a surface by elevation via a lookup table, then darkens each vertex
in proportion to local slope:

- **Flat areas** → full elevation colour
- **Steep cliffs** → same hue, darkened toward `minBrightness`

Two tuneable parameters in `main()`:

| Parameter | Default | Effect |
|---|---|---|
| `slopeGamma` | `1.5` | >1 limits darkening to only the steepest slopes |
| `minBrightness` | `0.15` | Floor brightness of vertical surfaces (0 = black) |

---

## Requirements

| Dependency | Minimum version |
|---|---|
| CMake | 3.12 |
| C++ compiler | C++17 (GCC 8+, Clang 7+, MSVC 2019+) |
| VTK | 9.0 |

---

## Build

### Linux / macOS

```bash
mkdir build && cd build
cmake ..                          # if VTK is on your system PATH / default prefix
# or, if VTK is in a custom location:
cmake -DVTK_DIR=/opt/vtk9/lib/cmake/vtk-9.3 ..

cmake --build . --parallel
./SlopeColorApp
```

### Windows (Visual Studio)

```bat
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DVTK_DIR="C:\vtk9\lib\cmake\vtk-9.3" ..
cmake --build . --config Release
Release\SlopeColorApp.exe
```

---

## Adapting to your own data

Replace the `vtkSphereSource` block in `main()` with your reader:

```cpp
// Example: VTK XML PolyData
#include <vtkXMLPolyDataReader.h>
auto source = vtkSmartPointer<vtkXMLPolyDataReader>::New();
source->SetFileName("terrain.vtp");
source->Update();

// Then adjust elevMin / elevMax to match your data's Z range:
double bounds[6];
source->GetOutput()->GetBounds(bounds);
const double elevMin = bounds[4]; // Z min
const double elevMax = bounds[5]; // Z max
```

Everything downstream (`elevationFilter`, `normalsFilter`, etc.) connects via
`source->GetOutputPort()` and requires no other changes.

---

## Pipeline summary

```
vtkSphereSource  (or your reader)
    │
    ├─► vtkElevationFilter  ──────────────────────────────────────┐
    │       produces scalar "Elevation" (float, range elevMin…elevMax)│
    │                                                              │
    └─► vtkPolyDataNormals                                         │
            SplittingOff() ← critical                             │
            produces per-vertex normals                           │
                │                                                  │
                └──────────► vtkProgrammableFilter ◄──────────────┘
                                 SlopeColorExecute()
                                 · maps elevation → RGBA via LUT
                                 · computes brightness from |Nz|
                                 · writes uchar RGBA array "Colors"
                                         │
                                vtkPolyDataMapper
                                 SetColorModeToDirectScalars()
                                         │
                                    vtkActor
```
