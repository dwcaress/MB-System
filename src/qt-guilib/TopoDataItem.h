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
#include <vtkStripper.h>
#include <vtkPointDataToCellData.h>
#include <vtkCellCenters.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLabeledDataMapper.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkAreaPicker.h>
#include <vtkTrivialProducer.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <QList>
#include <QVector2D>
#include <QVariant>
#include "TopoDataset.h"       // provides DATA_QUALITY_NAME, BAD/GOOD_DATA macros
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

namespace mb_system {
  /**
     Renders bathymetric data from a shared TopoDataset within a
     QtQuickVTKItem.  The dataset owns the vtkPolyData; this class owns
     only the rendering pipeline (camera, light, actors, mapper, interactor).

     The pipeline is built once on data load (connectDataset / assemblePipeline)
     and from then on individual settings — colored scalar, shadow source,
     colormap, render type, axes visibility, vertical exaggeration — are
     applied through narrow apply* methods that only touch the affected stage.

     Two TopoDataItems can share one TopoDataset.  When the dataset emits
     qualityChanged() each item dispatches its own re-render independently.
  */
  class TopoDataItem : public QQuickVTKItem {
    Q_OBJECT
  public:
    /// 'Persistent' VTK visualization pipeline objects.
    /// No data-ownership fields here — those live in TopoDataset.
    struct Pipeline : vtkObject {
      Pipeline();

      static Pipeline* New();
      vtkTypeMacro(Pipeline, vtkObject);

      /// Wraps the shared TopoDataset::polyData() as a pipeline source so
      /// downstream filters (normalsFilter_, slopeCalc_, contourFilter_,
      /// etc.) can use SetInputConnection rather than SetInputData.
      /// Assigned in connectDataset().
      vtkNew<vtkTrivialProducer>   source_;

      /// Alias for TopoDataset::polyData(), set in connectDataset().
      /// Used by interactor styles and other code that needs direct access.
      vtkPolyData *polyData_ = nullptr;

      vtkNew<vtkLight>                   lightSource_;
      vtkNew<vtkElevationFilter>         elevFilter_;    // kept: used as LUT source in some paths
      vtkNew<vtkPolyDataNormals>         normalsFilter_;
      vtkNew<vtkPolyDataNormals>         shadeNormalsFilter_;
      vtkNew<vtkArrayCalculator>         slopeCalc_;
      vtkNew<vtkProgrammableFilter>      slopeColorFilter_;
      vtkNew<vtkContourFilter>           contourFilter_;
      vtkNew<vtkPolyDataMapper>          contourMapper_;
      vtkNew<vtkActor>                   contourActor_;
      vtkNew<vtkStripper>                contourStripper_;
      vtkNew<vtkPointDataToCellData>     contourPointToCell_;
      vtkNew<vtkCellCenters>             contourLabelPoints_;
      vtkNew<vtkTransform>               contourLabelTransform_;
      vtkNew<vtkTransformPolyDataFilter> contourLabelTransformFilter_;
      vtkNew<vtkLabeledDataMapper>       contourLabelMapper_;
      vtkNew<vtkTextProperty>            contourLabelTextProperty_;
      vtkNew<vtkActor2D>                 contourLabelActor_;
      vtkNew<vtkLookupTable>             elevLookupTable_;
      vtkNew<vtkActor>                   surfaceActor_;
      vtkNew<vtkPolyDataMapper>          surfaceMapper_;
      vtkNew<vtkRenderer>                renderer_;
      vtkNew<QVTKInteractor>             windowInteractor_;
      vtkNew<vtkAreaPicker>              areaPicker_;
      vtkNew<vtkCubeAxesActor>           axesActor_;
      vtkNew<vtkNamedColors>             colors_;
      vtkNew<vtkPolyDataMapper>          trackMapper_;
      vtkNew<vtkActor>                   trackActor_;
      vtkInteractorStyle                *interactorStyle_ = nullptr;
      bool firstRender_ = true;
    };

    // ── Enums ─────────────────────────────────────────────────────────────────

    enum class ColoredScalar : int { Elevation, Slope, DataQuality };
    Q_ENUM(ColoredScalar)

    enum class SurfaceRenderType { Polys, Wireframe, PointCloud };
    Q_ENUM(SurfaceRenderType)

    enum class ShadowSource { NoShadows, Illumination, LocalSlope, LocalSlopeGpu };
    Q_ENUM(ShadowSource)

    // ── Constructor / QQuickVTKItem overrides ─────────────────────────────────

    TopoDataItem();

    vtkUserData initializeVTK(vtkRenderWindow *renderWindow) override;
    void destroyingVTK(vtkRenderWindow *renderWindow,
                       vtkUserData userData) override;

    // ── Dataset binding ───────────────────────────────────────────────────────

    /// Bind (or rebind) the shared dataset.  Connects qualityChanged() and
    /// dataLoaded() signals.  Safe to call before or after initializeVTK().
    void setDataset(mb_system::TopoDataset *dataset);
    mb_system::TopoDataset *dataset() const { return dataset_; }

    Q_PROPERTY(mb_system::TopoDataset* dataset
               READ dataset WRITE setDataset NOTIFY datasetChanged)

    // ── Q_INVOKABLEs (called from QML) ───────────────────────────────────────

    /// Convenience: delegates to dataset_->loadFile().  Kept so existing QML
    /// FileDialog onAccepted handlers require no change.
    Q_INVOKABLE bool loadDatafile(QUrl file);

    Q_INVOKABLE bool   setColormap(QString cmapName);
    Q_INVOKABLE void   setShowAxes(bool plotAxes);
    Q_INVOKABLE void   setContours(bool enabled);
    Q_INVOKABLE void   setShowContourLabels(bool enabled);
    Q_INVOKABLE void   setContourCount(int n);
    Q_INVOKABLE void   setContourInterval(double interval);
    Q_INVOKABLE void   setVerticalExagg(float verticalExagg);
    Q_INVOKABLE bool   setMouseMode(QString mouseMode);
    Q_INVOKABLE float  getVerticalExagg() { return verticalExagg_; }
    Q_INVOKABLE void   setColoredScalar(ColoredScalar coloredScalar);
    Q_INVOKABLE void   setSurfaceRenderType(SurfaceRenderType renderType);
    Q_INVOKABLE void   setShadowSource(ShadowSource source);
    Q_INVOKABLE void   setSlopeGamma(double gamma);
    Q_INVOKABLE void   setMinBrightness(double minBrightness);
    Q_INVOKABLE double getSlopeGamma()    { return slopeGamma_; }
    Q_INVOKABLE double getMinBrightness() { return minBrightness_; }
    Q_INVOKABLE void   setupLightSource();
    Q_INVOKABLE void   setLight(bool lightsEnabled, float intensity,
                                double x, double y, double z);
    Q_INVOKABLE QVariantList getLightPosition();
    Q_INVOKABLE double getLightIntensity();
    Q_INVOKABLE QString printMouseHelp() { return "Mouse help goes here"; }
    Q_INVOKABLE void   resetCamera();
    Q_INVOKABLE void   setPBR(double roughness, double metallic);
    Q_INVOKABLE void   setOrthographicView();
    Q_INVOKABLE bool   saveSettings();
    Q_INVOKABLE bool   loadSettings();
    Q_INVOKABLE double getContourInterval();
    Q_INVOKABLE void   foo();

    // ── Navigation track ─────────────────────────────────────────────────────

    /// Set the navigation track from parallel x, y, z coordinate arrays.
    /// A polyline is built from the points in order and rendered in black
    /// overlaid on the surface.  Replaces any previously set track.
    /// Safe to call before initializeVTK(); the track is applied when the
    /// pipeline is next assembled or via dispatch if already initialised.
    bool setNavigationTrack(const std::vector<double> &x,
                            const std::vector<double> &y,
                            const std::vector<double> &z);

    bool setNavigationTrack(vtkPoints *points);
    
    /// Show or hide the navigation track without rebuilding the pipeline.
    Q_INVOKABLE void setShowNavTrack(bool show);

    // ── Non-invokable public API ──────────────────────────────────────────────

    /// Returns the underlying reader (via dataset_) for units/CRS queries.
    mb_system::TopoDataReader *getDataReader();

    virtual void setPickedPoint(double *worldCoords);
    Pipeline *getPipeline()           { return pipeline_; }
    vtkRenderer *getRenderer()        { return pipeline_->renderer_; }
    vtkPolyData *getPolyData();

    void render() { reassemblePipeline(); }

    // ── Q_PROPERTYs ───────────────────────────────────────────────────────────

    Q_PROPERTY(ColoredScalar coloredScalar
               READ coloredScalar      NOTIFY coloredScalarChanged)
    Q_PROPERTY(ShadowSource shadowSource
               READ shadowSource        NOTIFY shadowSourceChanged)
    Q_PROPERTY(SurfaceRenderType surfaceRenderType
               READ surfaceRenderType   NOTIFY surfaceRenderTypeChanged)
    Q_PROPERTY(bool showAxes
               READ showAxes            NOTIFY showAxesChanged)
    Q_PROPERTY(bool showContours
               READ showContours        NOTIFY showContoursChanged)
    Q_PROPERTY(bool showContourLabels
               READ showContourLabels   NOTIFY showContourLabelsChanged)
    Q_PROPERTY(float verticalExagg
               READ getVerticalExagg    NOTIFY verticalExaggChanged)
    Q_PROPERTY(double slopeGamma
               READ getSlopeGamma       NOTIFY slopeGammaChanged)
    Q_PROPERTY(double minBrightness
               READ getMinBrightness    NOTIFY minBrightnessChanged)
    Q_PROPERTY(double lightIntensity
               READ getLightIntensity   NOTIFY lightChanged)
    Q_PROPERTY(QVariantList lightPosition
               READ getLightPosition    NOTIFY lightChanged)
    Q_PROPERTY(QString dataFilename
               READ getDataFileName WRITE setDataFilename NOTIFY dataFilenameChanged)
    Q_PROPERTY(MyRubberBandStyle* dataSelector
               READ getPointsSelectInteractorStyle CONSTANT)

    // READ accessors for the properties above
    ColoredScalar     coloredScalar()     const { return coloredScalar_; }
    ShadowSource      shadowSource()      const { return shadowSource_; }
    SurfaceRenderType surfaceRenderType() const { return surfaceRenderType_; }
    bool showAxes()         const { return showAxes_; }
    bool showContours()     const { return showContours_; }
    bool showContourLabels()const { return showContourLabels_; }
    float getVerticalExagg()const { return verticalExagg_; }
    QString getDataFileName()const;

    bool getShowAxes()          const { return showAxes_; }
    bool getShowContours()      const { return showContours_; }
    bool getShowContourLabels() const { return showContourLabels_; }

    void setDataFilename(const QString name);

    MyRubberBandStyle *getPointsSelectInteractorStyle() {
      return pointsSelectInteractorStyle_;
    }

    const char *getColormapScheme();

  signals:
    void datasetChanged();
    void coloredScalarChanged();
    void shadowSourceChanged();
    void surfaceRenderTypeChanged();
    void showAxesChanged();
    void showContoursChanged();
    void showContourLabelsChanged();
    void verticalExaggChanged();
    void slopeGammaChanged();
    void minBrightnessChanged();
    void lightChanged();
    void dataFilenameChanged(QString name);
    void lineDefined(QList<QVector2D> elevProfile);
    void errorOccurred(QString message);


  public slots:
    /// Called by a region-selection interactor style when the user completes a
    /// rubber-band selection.  worldBounds = {xMin,xMax,yMin,yMax,zMin,zMax}
    /// of the selected bounding box in world space.
    /// Base implementation is a no-op.  SurfaceDataItem overrides it to emit
    /// editBoundsChanged() for the companion edit window.
    virtual void onRegionSelected(double worldBounds[6]) {
      qDebug() << "TopoDataItem::onRegionSelected()";
      (void)worldBounds;
    }

  protected:
    void initializePipeline();
    virtual void assemblePipeline(Pipeline *pipeline);
    void reassemblePipeline();

    // ── Staged pipeline assembly ───────────────────────────────────────────

    /// Wire the shared TopoDataset::polyData() into this pipeline via
    /// source_.  Replaces loadDataPipeline().  Returns false if the dataset
    /// is not yet loaded.
    virtual bool connectDataset(Pipeline *pipeline);

    /// Returns the algorithm output port that the rendering pipeline tail
    /// (normalsFilter_, slopeCalc_, contourFilter_, surfaceMapper_) reads
    /// from.  Base returns source_->GetOutputPort() — the full shared dataset.
    /// EditDataItem overrides this to interpose a spatial clip filter so only
    /// the edit-volume subset reaches the rendering stages.
    virtual vtkAlgorithmOutput *dataOutputPort(Pipeline *pipeline);

    void applyColoredScalar(Pipeline *pipeline);
    void applyShadowSource(Pipeline *pipeline);
    void applyColormap(Pipeline *pipeline);
    void applyRenderType(Pipeline *pipeline);
    void applyAxes(Pipeline *pipeline);
    void applyVerticalExagg(Pipeline *pipeline);
    void applyContours(Pipeline *pipeline);
    void applyNavTrack(Pipeline *pipeline);

    void setupAxes(vtkCubeAxesActor *axesActor,
                   vtkNamedColors *colors,
                   double *surfaceBounds,
                   double *gridBounds,
                   const char *xUnits, const char *yUnits,
                   const char *zUnits,
                   bool geographicCRS);

    // ── Member variables ───────────────────────────────────────────────────

    /// Shared dataset — NOT owned by this item.
    mb_system::TopoDataset *dataset_ = nullptr;

    QString dataFilename_;
    double  pickedCoords_[3];
    bool    pointPicked_     = false;
    bool    lightsEnabled_   = true;
    double  lightPosition_[3]= { -0.03, 0.24, 0.50 };
    double  lightIntensity_  = 1.0;
    bool    forceRender_     = false;
    float   verticalExagg_   = 1.f;
    bool    showAxes_        = false;
    bool    showContours_    = false;
    bool    showContourLabels_= false;
    int     contourCount_    = 20;

    mb_system::TopoColorMap::Scheme colormapScheme_;
    ColoredScalar     coloredScalar_     = ColoredScalar::Elevation;
    ShadowSource      shadowSource_      = ShadowSource::Illumination;
    SurfaceRenderType surfaceRenderType_ = SurfaceRenderType::Polys;

    Pipeline         *pipeline_          = nullptr;
    vtkRenderWindow  *renderWindow_      = nullptr;

    /// Navigation track polydata — owned here so it survives pipeline rebuilds.
    /// Null points / no cells = no track set yet.
    vtkNew<vtkPolyData> trackPolyData_;
    bool showNavTrack_ = false;

    double slopeGamma_       = 0.35;
    double minBrightness_    = 0.15;

    SlopeShader::CallbackData *slopeCallbackData_ = nullptr;

    /// Upstream port carrying the active colored scalar; set by
    /// applyColoredScalar, consumed by applyShadowSource.
    vtkAlgorithmOutput *coloredOutputPort_ = nullptr;

    double coloredMin_ = 0.0;
    double coloredMax_ = 0.0;

    // ── Interactor styles ──────────────────────────────────────────────────
    PickInteractorStyle              *pickInteractorStyle_         = nullptr;
    LightingInteractorStyle          *lightingInteractorStyle_     = nullptr;
    vtkNew<PointsSelectInteractorStyle> pointsSelectInteractorStyle_;
    vtkNew<DrawInteractorStyle>         drawInteractorStyle_;
    vtkNew<DrawInteractorStyle>         testStyle_;

  protected slots:
    /// Called when the dataset finishes loading a file.
    /// Triggers a full pipeline rebuild and updates the filename label.
    void onDatasetLoaded();

    /// Called when the dataset's quality array is modified in place.
    /// Dispatches a lightweight re-render (no pipeline rebuild).
    virtual void onQualityChanged();
  };
}
#endif
