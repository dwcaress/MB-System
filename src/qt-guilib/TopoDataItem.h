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
#include <vtk/vtkGenericRenderWindowInteractor.h>
#include <QVTKInteractor.h>
#include <QList>
#include <QVector2D>
#include <QVariant>
#include "SlopeFilter.h"
#include "TopoDataReader.h"
#include "TopoColorMap.h"
#include "TopoDataPickerInteractorStyle.h"

namespace mb_system {

  /* **
     Manage VTK rendering of MB grid or swath file within a QtQuickVTKItem
  */
  class TopoDataItem : public QQuickVTKItem {
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
      vtkNew<mb_system::TopoDataReader> topoReader_;

      vtkNew<vtkElevationFilter> elevFilter_;
      vtkNew<SlopeFilter> slopeFilter_;      
      vtkNew<vtkLookupTable> elevLookupTable_;
      vtkNew<vtkActor> surfaceActor_;
      vtkNew<vtkPolyDataMapper> surfaceMapper_;
      vtkNew<vtkRenderer> renderer_;
      /// vtkNew<vtkGenericRenderWindowInteractor> windowInteractor_;
      vtkNew<QVTKInteractor> windowInteractor_;            
      vtkNew<mb_system::TopoDataPickerInteractorStyle> interactorStyle_;      

      /// x,y,z axes
      vtkNew<vtkCubeAxesActor> axesActor_;
      vtkNew<vtkNamedColors>colors_;
    
      bool firstRender_ = true;
    };

    /// Type of surface to display; elevation, gradient...
    enum class DisplayedSurface : int {
      Elevation,
      Gradient
    };

    Q_ENUM(DisplayedSurface)
    
    /// Constructor
    TopoDataItem();

    /// Get pointer to grid reader
    mb_system::TopoDataReader *getGridReader();
    
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

    /// Set vertical exaggeration
    Q_INVOKABLE void setVerticalExagg(float verticalExagg) {
      verticalExagg_ = verticalExagg;
    }


    /// Get vertical exaggeration
    Q_INVOKABLE float getVerticalExagg() {
      return verticalExagg_;
    }

    
    /// Set type of surface to display
    Q_INVOKABLE void setDisplayedSurface(DisplayedSurface surfaceType) {
      qDebug() << "setDisplayedSurface to " << surfaceType;
      displayedSurface_ = surfaceType;
      reassemblePipeline();
    }


    /// Test
    Q_INVOKABLE QList<QVector2D> runTest2(void);    

    /// Return elevation profile from encapsulated TopoData object, as a
    /// QList of QVector2D objects, removing need to register additional
    /// types with metadata system. QList has zero-length in case of error.
    Q_INVOKABLE QList<QVector2D> getElevProfile(int row1, int col1,
					     int row2, int col2,
					     int nPieces);

    /// Set picked point
    void setPickedPoint(double *worldCoords);

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

    /// Print polydata output of pipeline algorithm
    void printPolyDataOutput(vtkDataSetAlgorithm *algorithm,
			     const char *outputName);

    
    /// Name of source grid file
    char *gridFilename_;


    /// Latest picked coordinates
    double pickedCoords_[3];

    /// Indicates if point has been picked by user
    bool pointPicked_;

    /// Indicates whether to render on next update()
    bool forceRender_;

    
    /// Vertical exaggeration
    float verticalExagg_;

    /// Show axes or not
    bool showAxes_;

    /// Colormap scheme
    mb_system::TopoColorMap::Scheme scheme_;

    /// Type of surface to display (elevation, gradient...)
    DisplayedSurface displayedSurface_;
    
    Pipeline *pipeline_;
    vtkRenderWindow *renderWindow_;


  };
}

#endif



