#ifndef TOPOGRIDITEM_H
#define TOPOGRIDITEM_H
#include <QObject>
#include <vtk/QQuickVTKItem.h>
#include <vtk/vtkActor.h>
#include <vtk/vtkRenderer.h>
#include <vtk/vtkPolyDataMapper.h>
#include <vtk/vtkRenderWindow.h>
#include <vtk/vtkElevationFilter.h>
#include <vtk/vtkLookupTable.h>
#include <vtk/vtkTransform.h>
#include <vtk/vtkTransformFilter.h>
#include <vtk/vtkCubeAxesActor.h>
#include <vtk/vtkNamedColors.h>

#include "TopoGridReader.h"
#include "TopoColorMap.h"

namespace mb_system {
  /* **
     Manage VTK rendering of MB grid or swath file within a QtQuick item
  */
  class TopoGridItem : public QQuickVTKItem {
    Q_OBJECT
  
  public:


    /** 'Persistent' VTK pipeline objects, used by QQuickItem infrastructure */
    struct Pipeline : vtkObject {

      Pipeline() {firstRender_ = true;}
      
      /// Factory to create static Data instance 
      static Pipeline* New();
    
      /// Add RTTI to Data
      vtkTypeMacro(Pipeline, vtkObject);

      // Instantiate persistent VTK pipeline objects

      /// Topo grid reader
      vtkNew<mb_system::TopoGridReader> gridReader_;

      vtkNew<vtkElevationFilter> elevFilter_;
      vtkNew<vtkLookupTable> elevLookupTable_;
      vtkNew<vtkActor> surfaceActor_;
      vtkNew<vtkPolyDataMapper> surfaceMapper_;
      vtkNew<vtkRenderer> renderer_;
      vtkNew<vtkTransform> transform_;
      vtkNew<vtkTransformFilter> transformFilter_;
      vtkNew<vtkCubeAxesActor> axesActor_;
      vtkNew<vtkNamedColors>colors_;
    
      bool firstRender_ = true;
    };

    /// Constructor
    TopoGridItem();

  
    /// Initialize VTK pipeline and attach it to vtkRenderWindow, return
    /// latest pipeline object.
    /// (Return type vtkUserData is defined in parent class)
    vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;

    /// 
    void destroyingVTK(vtkRenderWindow
		       *renderWindow, vtkUserData userData) override;

    /// Set topo colormap scheme; return true if colorMapName corresponds
    /// to a supported colorMap, else return false.
    bool setColorMapScheme(const char *colorMapname);


    /// Load specified grid file
    Q_INVOKABLE bool loadGridfile(QUrl file);
    
    /// Set grid filename
    void setGridFilename(char *filename) {
      if (gridFilename_) {
        free((void *)gridFilename_);
      }
      gridFilename_ = strdup(filename);
    }


    void showAxes(bool plotAxes) {
      plotAxes_ = plotAxes;
    }

    void setVerticalExagg(float verticalExagg) {
      verticalExagg_ = verticalExagg;
    }
    
  
  protected:

    /// Assemble pipeline elements
    void assemblePipeline(Pipeline *pipeline);
    
    /// Set up axes
    void setupAxes(vtkCubeAxesActor *axesActor,
		   vtkNamedColors *colors,                                                    double *surfaceBounds,
		   double *gridBounds,
		   const char *xUnits, const char *yUnits,
		   const char *zUnits,
		   bool geographicCRS);

    /// Name of source grid file
    char *gridFilename_;

    /// Vertical exaggeration
    float verticalExagg_;

    /// Plot axes or not
    bool plotAxes_;

    /// Colormap scheme
    mb_system::TopoColorMap::Scheme scheme_;
  
    Pipeline *pipeline_;
    vtkRenderWindow *renderWindow_;
    
  };
}

#endif



