#ifndef SlopeShader_H
#define SlopeShader_H
#include <cmath>
#include <iostream>
#include <memory>

// VTK core
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyDataAlgorithm.h>

// Filters
#include <vtkElevationFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>



namespace mb_system {
  /**
   * SlopeColorApp.cxx
   *
   * VTK 9 example: colors a terrain surface by elevation (via a lookup table),
   * then darkens each vertex in proportion to local surface slope.
   *
   * Pipeline:
   *   vtkSphereSource (synthetic terrain)
   *     ├─► vtkElevationFilter   → elevation scalar "Elevation"
   *     └─► vtkPolyDataNormals   → per-vertex normals
   *           └─► vtkProgrammableFilter  (merges both branches → RGBA "Colors")
   *                 └─► vtkPolyDataMapper (SetColorModeToDirectScalars)
   *                       └─► vtkActor ► vtkRenderer ► vtkRenderWindow
   *
   * Build:
   *   See CMakeLists.txt in the same directory.
   */
  class SlopeShader {

  public:
    
    ///  Data shared between the programmable-filter callback and the 
    ///  application
    struct CallbackData {
      vtkProgrammableFilter* filter;      // the filter that owns this callback
      vtkPolyDataNormals*    normalsFilter; // separate normals branch
      vtkLookupTable*        lut;           // elevation LUT
      double                 elevMin;
      double                 elevMax;
      
      // >1 → only very steep areas darken      
      double                 slopeGamma;
      
      // floor (0–1); prevents pure-black cliffs
      double                 minBrightness; 
    };


    ///  The programmable filter's execute callback
    static void execute(void* userData);

    ///  Cleanup callback so VTK properly frees CallbackData
    static void deleteCallbackData(void* userData);

  };

}



#endif  // header guard
