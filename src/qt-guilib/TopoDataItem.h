#ifndef TOPOGRIDITEM_H
#define TOPOGRIDITEM_H
#include <QObject>
#include <QQuickVTKItem.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkElevationFilter.h>
#include <vtkLookupTable.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkProgrammableFilter.h>
#include <vtkArrayCalculator.h>
#include <vtkCubeAxesActor.h>
#include <vtkNamedColors.h>
#include <vtkLight.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleDrawPolygon.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <QVTKInteractor.h>
#include <vtkInteractorStyleTerrain.h>
#include <vtkPolyDataNormals.h>
#include <vtkContourFilter.h>
#include <vtkAreaPicker.h>
#include <vtkIdFilter.h>
#include <QList>
#include <QVector2D>
#include <QVariant>
#include "TopoDataReader.h"
#include "TopoColorMap.h"
#include "PickInteractorStyle.h"
#include "LightingInteractorStyle.h"
#include "PointsSelectInteractorStyle.h"
#include "MyRubberBandStyle.h"
#include "Point.h"
#include "PickerInteractorStyle.h"
#include "RestrictCameraStyle.h"
#include "DrawInteractorStyle.h"
#include "SlopeShader.h"

#define DATA_QUALITY_NAME "dataQuality"
#define ORIGINAL_IDS "originalIDs"
#define BAD_DATA 0
#define GOOD_DATA 1



namespace mb_system {
  /**
     Renders bathymetric data of grid or swath file, within a QtQuickVTKItem.

     The pipeline is built once on data load (loadDataPipeline) and from then on
     individual settings — colored scalar, shadow source, colormap, render type,
     axes visibility, vertical exaggeration — are applied through narrow apply*
     methods that only touch the affected stage.  This avoids re-reading the
     data file on every UI change.
  */
  class TopoDataItem : public QQuickVTKItem {
    Q_OBJECT

  public:

    /// 'Persistent' VTK visualization pipeline objects
    struct Pipeline : vtkObject {

      Pipeline() {
        firstRender_ = true;
        // One-time wiring: the actor uses our mapper.  Done here (not in
        // assemblePipeline) so that mode-change setters that bypass the
        // full rebuild don't have to remember to call SetMapper.
        surfaceActor_->SetMapper(surfaceMapper_);

        // Contour actor: solid-color lines, no lighting, lifted slightly
        // toward camera to avoid z-fighting with the surface underneath.
        contourActor_->SetMapper(contourMapper_);
        contourMapper_->SetInputConnection(contourFilter_->GetOutputPort());
        contourMapper_->ScalarVisibilityOff();
        contourMapper_->SetResolveCoincidentTopologyToPolygonOffset();
        contourMapper_->SetResolveCoincidentTopologyLineOffsetParameters(
									 -1.0, -1.0);
        contourActor_->GetProperty()->SetColor(0.0, 0.0, 0.0);
        contourActor_->GetProperty()->SetLineWidth(1.2);
        contourActor_->GetProperty()->LightingOff();
      }

      /// Declare static New() method expected by VTK factory classes
      static Pipeline* New();

      vtkTypeMacro(Pipeline, vtkObject);

      /// Light source
      vtkNew<vtkLight> lightSource_;

      /// Topo data reader
      vtkNew<mb_system::TopoDataReader> topoReader_;

      vtkNew<vtkElevationFilter> elevFilter_;

      /// Computes per-vertex normals for "Slope" colored-scalar mode
      vtkNew<vtkPolyDataNormals> normalsFilter_;

      /// Computes per-vertex normals for slope-darkening (CPU and GPU paths)
      vtkNew<vtkPolyDataNormals> shadeNormalsFilter_;

      /// Computes a "Slopes" point-data array from Normals (acos of Nz)
      vtkNew<vtkArrayCalculator> slopeCalc_;

      /// CPU slope-darkening (RGBA written per vertex)
      vtkNew<vtkProgrammableFilter> slopeColorFilter_;

      /// Contour lines (off elevFilter_ output)
      vtkNew<vtkContourFilter>   contourFilter_;
      vtkNew<vtkPolyDataMapper>  contourMapper_;
      vtkNew<vtkActor>           contourActor_;

      vtkNew<vtkIdFilter> idFilter_;
      vtkNew<vtkLookupTable> elevLookupTable_;
      vtkNew<vtkActor> surfaceActor_;
      vtkNew<vtkPolyDataMapper> surfaceMapper_;
      vtkNew<vtkRenderer> renderer_;
      vtkNew<QVTKInteractor> windowInteractor_;
      vtkNew<vtkAreaPicker> areaPicker_;
      /// data quality array for input vtkPolyData
      vtkNew<vtkIntArray> quality_;

      /// Assign this pointer to appropriate interactor style,
      /// depending on how 'mouse mode' is set
      vtkInteractorStyle *interactorStyle_;

      /// Source polydata (alias for elevFilter_->GetOutput())
      vtkPolyData *polyData_;

      /// x,y,z axes
      vtkNew<vtkCubeAxesActor> axesActor_;
      vtkNew<vtkNamedColors>colors_;

      bool firstRender_ = true;
    };

    /// Color surface by this scalar value
    enum class ColoredScalar : int {
      Elevation,
      Slope,
      DataQuality
    };
    Q_ENUM(ColoredScalar)


    /// Render surface style
    enum class SurfaceRenderType {
      Polys,
      Wireframe,
      PointCloud
    };
    Q_ENUM(SurfaceRenderType)

    /// Render shadow method
    enum class ShadowSource {
      Illumination,
      LocalSlope,
      LocalSlopeGpu,
      NoShadows
    };
    Q_ENUM(ShadowSource)

    /// Constructor
    TopoDataItem();

    /// Get pointer to grid reader
    mb_system::TopoDataReader *getDataReader();

    /// Initialize and connect VTK pipeline components, attach it to
    /// vtkRenderWindow, return latest pipeline object.
    vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;

    /// Clean up and free resources as needed
    void destroyingVTK(vtkRenderWindow *renderWindow,
		       vtkUserData userData) override;

    /// Load specified grid file (triggers full pipeline rebuild)
    Q_INVOKABLE bool loadDatafile(QUrl file);

    /// Set color map (rebuilds LUT only)
    Q_INVOKABLE bool setColormap(QString cmapName);

    /// Toggle axes plot
    Q_INVOKABLE void showAxes(bool plotAxes);

    /// Toggle contour lines on the data surface
    Q_INVOKABLE void setContours(bool enabled);

    /// Set how many contour levels to draw between elevMin and elevMax
    /// (default 20).  Takes effect immediately if contours are enabled.
    Q_INVOKABLE void setContourCount(int n);

    /// Set contour interval 
    /// Takes effect immediately if contours are enabled.
    Q_INVOKABLE void setContourInterval(double interval);
    
    /// Set vertical exaggeration
    Q_INVOKABLE void setVerticalExagg(float verticalExagg);

    /// Set mouse mode (swaps interactor style only)
    Q_INVOKABLE bool setMouseMode(QString mouseMode);

    /// Get vertical exaggeration
    Q_INVOKABLE float getVerticalExagg() {
      return verticalExagg_;
    }

    /// Set type of colored scalar (Elevation / Slope / DataQuality)
    Q_INVOKABLE void setColoredScalar(ColoredScalar coloredScalar);

    /// Set surface representation (Polys / Wireframe / PointCloud)
    Q_INVOKABLE void setSurfaceRenderType(SurfaceRenderType renderType);

    /// Set shadow source (Illumination / LocalSlope / LocalSlopeGpu / None)
    Q_INVOKABLE void setShadowSource(ShadowSource source);

    /// Slope-darkening gamma (>1 → only steep areas darken).
    /// Cheap: just updates a uniform in GPU mode, or marks the CPU
    /// programmable filter dirty in CPU mode.  Does not rebuild.
    Q_INVOKABLE void setSlopeGamma(double gamma);

    /// Slope-darkening floor (0–1; prevents pure-black cliffs)
    Q_INVOKABLE void setMinBrightness(double minBrightness);

    Q_INVOKABLE double getSlopeGamma()    { return slopeGamma_; }
    Q_INVOKABLE double getMinBrightness() { return minBrightness_; }

    /// Set up the light source
    Q_INVOKABLE void setupLightSource(void);

    /// Set light source intensity and position
    Q_INVOKABLE void setLight(bool lightsEnabled, float intensity,
                              double x, double y, double z);

    /// Get light source position
    Q_INVOKABLE QVariantList getLightPosition(void);

    /// Get light source intensity
    Q_INVOKABLE double getLightIntensity(void);

    /// Print mouse help message
    Q_INVOKABLE QString printMouseHelp() {
      return "Mouse help goes here";
    }

    /// Reset camera, all actors in bounding box
    Q_INVOKABLE void resetCamera();

    /// Set physically-based rendering parameters on surface actor
    Q_INVOKABLE void setPBR(double roughness, double metallic);

    /// Set camera for orthographic view
    Q_INVOKABLE void setOrthographicView();

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

    /// Get pipeline
    Pipeline *getPipeline() {
      return pipeline_;
    }

    /// Get surface renderer from pipeline
    vtkRenderer *getRenderer() {
      return pipeline_->renderer_;
    }

    /// Get source polydata
    vtkPolyData *getPolyData();

    /// Trigger re-render (full rebuild via render thread)
    void render() {
      reassemblePipeline();
    }

    /// UI-visible state properties
    Q_PROPERTY(ColoredScalar coloredScalar
	       READ coloredScalar NOTIFY coloredScalarChanged)
    Q_PROPERTY(ShadowSource shadowSource
	       READ shadowSource   NOTIFY shadowSourceChanged)
    Q_PROPERTY(SurfaceRenderType surfaceRenderType
	       READ surfaceRenderType NOTIFY surfaceRenderTypeChanged)
    Q_PROPERTY(bool showAxes
	       READ showAxes   NOTIFY showAxesChanged)
    Q_PROPERTY(bool contoursEnabled
	       READ contoursEnabled NOTIFY contoursEnabledChanged)
    Q_PROPERTY(float verticalExagg
	       READ getVerticalExagg NOTIFY verticalExaggChanged)
    Q_PROPERTY(double slopeGamma
	       READ getSlopeGamma  NOTIFY slopeGammaChanged)
    Q_PROPERTY(double minBrightness
	       READ getMinBrightness NOTIFY minBrightnessChanged)
    Q_PROPERTY(double lightIntensity
	       READ getLightIntensity NOTIFY lightChanged)
    Q_PROPERTY(QVariantList lightPosition
	       READ getLightPosition  NOTIFY lightChanged)

    // Q_PROPERTY READ methods (trivial for the ones already declared Q_INVOKABLE):
    ColoredScalar    coloredScalar()    const { return coloredScalar_; }
    ShadowSource     shadowSource()     const { return shadowSource_; }
    SurfaceRenderType surfaceRenderType() const { return surfaceRenderType_; }
    bool             showAxes()         const { return showAxes_; }
    bool             contoursEnabled()  const { return contoursEnabled_; }
    float getVerticalExagg() const { return verticalExagg_; }
    
    /// Set pointsSelectInteractorStyle_ as a property so that its emitted
    /// signals can be received by QML
    Q_PROPERTY(MyRubberBandStyle* dataSelector
               READ getPointsSelectInteractorStyle CONSTANT)

    MyRubberBandStyle *getPointsSelectInteractorStyle() {
      return pointsSelectInteractorStyle_;
    }


  signals:

signals:

    void coloredScalarChanged();
    void shadowSourceChanged();
    void surfaceRenderTypeChanged();
    void showAxesChanged();
    void contoursEnabledChanged();
    void verticalExaggChanged();
    void slopeGammaChanged();
    void minBrightnessChanged();
    void lightChanged();       // covers both position and intensity

    
    /// Emit when user defines a line with mouse
    void lineDefined(QList<QVector2D> elevProfile);

    /// Emit when error occurs, QML will pop up message
    void errorOccurred(QString message);


  protected:

    /// Initialize pipeline structure (currently a stub; kept for compat)
    void initializePipeline(void);

    /// Full pipeline rebuild — re-reads data file, calls every apply*.
    /// Used on first init and on data file load.
    virtual void assemblePipeline(Pipeline *pipeline);

    /// Dispatch a full rebuild to the Qt render thread, then render.
    void reassemblePipeline(void);

    // ── Staged pipeline assembly ────────────────────────────────────────────
    //
    // Each apply* method only touches the parts of the pipeline that depend
    // on its corresponding piece of state.  Setters above dispatch only the
    // method(s) they need.
    //
    /// Stage 1: read data file, set up reader/idFilter/elevFilter,
    /// cache bounds, build quality array.  Called once per file load.
    /// Returns false if the file could not be read.
    bool loadDataPipeline(Pipeline *pipeline);

    /// Stage 2: select the "colored" scalar and the upstream port that
    /// carries it.  Also calls applyShadowSource() since the shadow source
    /// consumes that port.
    void applyColoredScalar(Pipeline *pipeline);

    /// Stage 3: configure mapper tail (LUT, ColorMode, ScalarRange),
    /// install/remove the GPU shader replacements, set lighting on/off.
    void applyShadowSource(Pipeline *pipeline);

    /// Rebuild the lookup table from scheme_ (in place).
    void applyColormap(Pipeline *pipeline);

    /// Set actor representation (Polys/Wireframe/PointCloud).
    void applyRenderType(Pipeline *pipeline);

    /// Add/remove the cube-axes actor based on showAxes_.
    void applyAxes(Pipeline *pipeline);

    /// Apply vertical exaggeration scale to the surface actor.
    void applyVerticalExagg(Pipeline *pipeline);

    /// Add/remove the contour actor and refresh its contour values.
    void applyContours(Pipeline *pipeline);

    /// Set up axes (geometry/units configuration)
    void setupAxes(vtkCubeAxesActor *axesActor,
                   vtkNamedColors *colors,
                   double *surfaceBounds,
                   double *gridBounds,
                   const char *xUnits, const char *yUnits,
                   const char *zUnits,
                   bool geographicCRS);

    /// Name of source data file
    char *dataFilename_;

    /// Latest picked coordinates
    double pickedCoords_[3];

    /// Indicates if point has been picked by user
    bool pointPicked_;

    /// Shade display with lights?  (derived from shadowSource_)
    bool lightsEnabled_ = true;

    double lightPosition_[3] = { -0.03, 0.24, 0.50 }; 
    double lightIntensity_    = 1.0;
    
    /// Indicates whether to render on next update()
    bool forceRender_;

    /// Vertical exaggeration
    float verticalExagg_;

    /// Show axes or not
    bool showAxes_;

    /// Show contour lines or not
    bool contoursEnabled_ = false;

    /// Number of contour levels between elevMin and elevMax
    int  contourCount_ = 20;

    /// Colormap scheme
    mb_system::TopoColorMap::Scheme scheme_;

    /// Type of surface to display (Elevation, Slope, DataQuality)
    ColoredScalar coloredScalar_;

    /// Shadow source
    ShadowSource shadowSource_ = ShadowSource::Illumination;

    /// Type of surface rendering
    SurfaceRenderType surfaceRenderType_;

    /// VTK pipeline
    Pipeline *pipeline_;

    vtkRenderWindow *renderWindow_;

    // ── Cached after loadDataPipeline ──────────────────────────────────────
    double elevMin_      = 0.0;
    double elevMax_      = 0.0;
    double gridBounds_[6]= {0,0,0,0,0,0};
    bool   dataLoaded_   = false;

    // ── Slope-darkening parameters (sliders feed these) ────────────────────
    double slopeGamma_    = 0.35;
    double minBrightness_ = 0.15;

    /// Persistent callback data for the CPU slope filter.  Owned by
    /// pipeline_->slopeColorFilter_ via SetExecuteMethodArgDelete.
    SlopeShader::CallbackData *slopeCallbackData_ = nullptr;

    /// Upstream port carrying the active colored scalar.  Set by
    /// applyColoredScalar, consumed by applyShadowSource.
    vtkAlgorithmOutput *coloredOutputPort_ = nullptr;

    /// Cached scalar range for the colored scalar
    double coloredMin_ = 0.0;
    double coloredMax_ = 0.0;

    // ── Interactor styles ──────────────────────────────────────────────────
    PickInteractorStyle *pickInteractorStyle_;
    LightingInteractorStyle *lightingInteractorStyle_;
    vtkNew<PointsSelectInteractorStyle> pointsSelectInteractorStyle_;
    vtkNew<DrawInteractorStyle> drawInteractorStyle_;
    vtkNew<DrawInteractorStyle> testStyle_;
  };
 }

#endif
