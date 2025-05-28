#ifndef TOPOGRIDITEM_H
#define TOPOGRIDITEM_H
#include <QObject>
#include <QQuickVTKItem.h>
#include <vtkActor.h>    // Does it require vtk/?
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkElevationFilter.h>
#include <vtkLookupTable.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCubeAxesActor.h>
#include <vtkNamedColors.h>
#include <vtkLight.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <QVTKInteractor.h>
#include <QList>
#include <QVector2D>
#include <QVariant>
#include "SlopeFilter.h"
#include "TopoDataReader.h"
#include "TopoColorMap.h"
#include "InteractorStyle.h"
#include "PickInteractorStyle.h"
#include "LightPositionInteractorStyle.h"

namespace mb_system {

  /* **
     Renders bathymetric data of MB grid or swath file, within a QtQuickVTKItem.
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

      /// Light source
      vtkNew<vtkLight> lightSource_;
      
      /// Topo grid reader
      vtkNew<mb_system::TopoDataReader> topoReader_;

      vtkNew<vtkElevationFilter> elevFilter_;
      vtkNew<SlopeFilter> slopeFilter_;      
      vtkNew<vtkLookupTable> elevLookupTable_;
      vtkNew<vtkActor> surfaceActor_;
      vtkNew<vtkPolyDataMapper> surfaceMapper_;
      vtkNew<vtkRenderer> renderer_;
      vtkNew<QVTKInteractor> windowInteractor_;

      /// Assign this pointer to appropriate interactor style,
      /// depending on how 'mouse mode' is set
      // /vtkNew<mb_system::TopoDataPickerInteractorStyle> interactorStyle_;
      mb_system::InteractorStyle *interactorStyle_;            

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
    mb_system::TopoDataReader *getDataReader();
    
    /// Initialize and connect VTK pipeline components, attach it to
    /// vtkRenderWindow, return latest pipeline object.
    /// (Return type vtkUserData is defined in parent class)
    vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;

    /// Clean up and free resources as needed
    void destroyingVTK(vtkRenderWindow
		       *renderWindow, vtkUserData userData) override;

    /// Load specified grid file
    Q_INVOKABLE bool loadDatafile(QUrl file);

    /// Set color map
    Q_INVOKABLE bool setColormap(QString cmapName);    

    /// Toggle axes plot
    Q_INVOKABLE void showAxes(bool plotAxes);

    /// Set vertical exaggeration
    Q_INVOKABLE void setVerticalExagg(float verticalExagg) {
      verticalExagg_ = verticalExagg;
    }

    /// Set mouse mode
    Q_INVOKABLE bool setMouseMode(QString mouseMode);


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

    /// Set up the light source
    Q_INVOKABLE void setupLightSource(void);

    
    /// Set picked point
    void setPickedPoint(double *worldCoords);

    /// Set grid filename
    void setDataFilename(char *filename) {
      if (dataFilename_) {
        free((void *)dataFilename_);
      }
      if (filename) {
	dataFilename_ = strdup(filename);
      }
      else {
	dataFilename_ = strdup("");
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

    
    /// Name of source data file
    char *dataFilename_;


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

    /// Interactor styles (can be selected by user)
    mb_system::PickInteractorStyle *pickInteractorStyle_;
    LightPositionInteractorStyle *lightPositionInteractorStyle_;
    
  };
}

#endif



