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


    /// 'Persistent' VTK pipeline objects, used by QQuickItem infrastructure
    struct Pipeline : vtkObject {

      Pipeline() {firstRender_ = true;}
      
      /// Declare static New() method expected by VTK factory classes, that
      /// returns a Pipeline instance
      static Pipeline* New();
      
      /// Enable run-time typing to Pipeline
      vtkTypeMacro(Pipeline, vtkObject);

      /// Topo grid reader
      vtkNew<mb_system::TopoGridReader> gridReader_;

      vtkNew<vtkElevationFilter> elevFilter_;
      vtkNew<vtkLookupTable> elevLookupTable_;
      vtkNew<vtkActor> surfaceActor_;
      vtkNew<vtkPolyDataMapper> surfaceMapper_;
      vtkNew<vtkRenderer> renderer_;

      /// x,y,z axes
      vtkNew<vtkCubeAxesActor> axesActor_;
      vtkNew<vtkNamedColors>colors_;
    
      bool firstRender_ = true;
    };

    /// Constructor
    TopoGridItem();

  
    /// Initialize and connect VTK pipeline components, attach it to
    /// vtkRenderWindow, return latest pipeline object.
    /// (Return type vtkUserData is defined in parent class)
    vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;

    /// Clean up and free resources as needed
    void destroyingVTK(vtkRenderWindow
		       *renderWindow, vtkUserData userData) override;

    /// Load specified grid file
    Q_INVOKABLE bool loadGridfile(QUrl file);

    /// Set color map
    Q_INVOKABLE bool setColormap(QString cmapName);    

    /// Toggle axes plot
    Q_INVOKABLE void showAxes(bool plotAxes);

    Q_INVOKABLE void setVerticalExagg(float verticalExagg) {
      verticalExagg_ = verticalExagg;
    }
    

    /// Set grid filename
    void setGridFilename(char *filename) {
      if (gridFilename_) {
        free((void *)gridFilename_);
      }
      if (filename) {
	gridFilename_ = strdup(filename);
      }
      else {
	gridFilename_ = strdup("");
      }
    }


  
  protected:

    /// Assemble pipeline elements
    void assemblePipeline(Pipeline *pipeline);

    /// Pass pipeline reassembly lambda code to dispatch_async() 
    /// for execution in render thread
    void reassemblePipeline(void);

    /// Set up axes
    void setupAxes(vtkCubeAxesActor *axesActor,
		   vtkNamedColors *colors,
		   double *surfaceBounds,
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



