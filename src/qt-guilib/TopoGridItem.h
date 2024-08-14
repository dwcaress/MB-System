#ifndef TOPOGRIDITEM_H
#define TOPOGRIDITEM_H
#include <QObject>
#include <vtk-9.3/QQuickVTKItem.h>
#include <vtk-9.3/vtkActor.h>
#include <vtk-9.3/vtkRenderer.h>
#include <vtk-9.3/vtkPolyDataMapper.h>
#include <vtk-9.3/vtkRenderWindow.h>
#include <vtk-9.3/vtkElevationFilter.h>
#include <vtk-9.3/vtkLookupTable.h>
#include <vtk-9.3/vtkTransform.h>
#include <vtk-9.3/vtkTransformFilter.h>

#include "TopoGridReader.h"

/* **
Manage VTK rendering of MB grid or swath file within a QtQuick item
*/
class TopoGridItem : public QQuickVTKItem {
  Q_OBJECT
  
 public:


  /** 'Persistent' VTK pipeline objects, used by QQuickItem infrastructure */
  struct Pipeline : vtkObject {

    /// Factory to create static Data instance 
    static Pipeline* New();
    
    /// Add RTTI to Data
    vtkTypeMacro(Pipeline, vtkObject);

    // Persistent VTK pipeline objects

    /// Topo grid reader
    vtkNew<mb_system::TopoGridReader> gridReader_;

    vtkNew<vtkElevationFilter> elevColorizer_;
    vtkNew<vtkLookupTable> elevLookupTable_;
    vtkNew<vtkActor> surfaceActor_;
    vtkNew<vtkPolyDataMapper> surfaceMapper_;
    vtkNew<vtkRenderer> renderer_;
    vtkNew<vtkTransform> transform_;
    vtkNew<vtkTransformFilter> transformFilter_;
  };

  /// Constructor
  TopoGridItem();

  
  /// Initialize VTK pipeline and attach it to vtkRenderWindow
  /// (Return type vtkUserData is defined in parent class)
  vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;

  /// 
  void destroyingVTK(vtkRenderWindow
		     *renderWindow, vtkUserData userData) override;

  /// Enqueue async command that will be executed just before VTK renders
  void dispatch_async(std::function<void(vtkRenderWindow *renderWindow,
					 vtkUserData userData)>f);
    /// Set grid filename
    void setGridFilename(char *filename) {
      if (gridFilename_) {
        free((void *)gridFilename_);
      }
      gridFilename_ = strdup(filename);
    }
  
  
 protected:

  char *gridFilename_;
  float verticalExagg_;
  
};

#endif



