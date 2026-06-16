/// #define QT_NO_DEBUG_OUTPUT
#include <unistd.h>
#include <climits>
#include <filesystem>
#include <array>
#include <vector>
#include <thread>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkLightCollection.h>
#include <vtkArrayCalculator.h>
#include <vtkPolyDataNormals.h>
#include <vtkIdTypeArray.h>
#include <vtkProgrammableFilter.h>
#include <vtkContourFilter.h>
#include <vtkStripper.h>
#include <vtkPointDataToCellData.h>
#include <vtkCellCenters.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLabeledDataMapper.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <QQuickWindow>
#include <QMessageBox>
#include <QCoreApplication>
#include "TopoDataItem.h"
#include "TopoColorMap.h"
#include "SharedConstants.h"
#include "SlopeShader.h"
#include "TopoDataItemSettings.h"
#include "SwathData.h"

using namespace mb_system;

vtkStandardNewMacro(TopoDataItem::Pipeline);

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

TopoDataItem::TopoDataItem() {
  dataFilename_ = QString("");
  verticalExagg_ = 1.f;
  showAxes_ = false;
  colormapScheme_ = TopoColorMap::Haxby;
  coloredScalar_ = ColoredScalar::Elevation;
  pointPicked_ = false;
  forceRender_ = false;
  surfaceRenderType_ = SurfaceRenderType::Polys;

  pickInteractorStyle_     = new PickInteractorStyle(this);
  lightingInteractorStyle_ = new LightingInteractorStyle(this);
  pointsSelectInteractorStyle_->setTopoDataItem(this);
  pointsSelectInteractorStyle_->setDrawingMode(
      MyRubberBandStyle::DrawingMode::Rectangle);
  drawInteractorStyle_->setTopoDataItem(this);
  drawInteractorStyle_->setDrawingMode(DrawInteractorStyle::DrawingMode::Line);
  testStyle_->setTopoDataItem(this);
  testStyle_->setDrawingMode(DrawInteractorStyle::DrawingMode::Line);
}

TopoDataItem::Pipeline::Pipeline() {

        firstRender_ = true;
        // One-time actor/mapper wiring
        surfaceActor_->SetMapper(surfaceMapper_);
        trackActor_->SetMapper(trackMapper_);

        contourActor_->SetMapper(contourMapper_);
        contourMapper_->SetInputConnection(contourFilter_->GetOutputPort());
        contourMapper_->ScalarVisibilityOff();
        contourMapper_->SetResolveCoincidentTopologyToPolygonOffset();
        contourMapper_->SetResolveCoincidentTopologyLineOffsetParameters(
                                                              -1.0, -1.0);
        contourActor_->GetProperty()->SetColor(0.0, 0.0, 0.0);
        contourActor_->GetProperty()->SetLineWidth(1.2);
        contourActor_->GetProperty()->LightingOff();

        // ── Contour-label sub-pipeline ────────────────────────────────────
        contourStripper_->SetInputConnection(contourFilter_->GetOutputPort());
        contourPointToCell_->SetInputConnection(
            contourStripper_->GetOutputPort());
        contourLabelPoints_->SetInputConnection(
            contourPointToCell_->GetOutputPort());
        contourLabelTransformFilter_->SetInputConnection(
            contourLabelPoints_->GetOutputPort());
        contourLabelTransformFilter_->SetTransform(contourLabelTransform_);

        contourLabelTextProperty_->SetColor(0.0, 0.0, 0.0);
        contourLabelTextProperty_->SetFontSize(16);
        contourLabelTextProperty_->SetBold(false);
        contourLabelTextProperty_->SetItalic(false);
        contourLabelTextProperty_->SetJustificationToCentered();
        contourLabelTextProperty_->SetVerticalJustificationToCentered();
        contourLabelTextProperty_->SetBackgroundColor(1.0, 1.0, 1.0);
        contourLabelTextProperty_->SetBackgroundOpacity(0.75);
        contourLabelTextProperty_->SetFrame(0);

        contourLabelMapper_->SetInputConnection(
            contourLabelTransformFilter_->GetOutputPort());
        contourLabelMapper_->SetLabelMode(VTK_LABEL_SCALARS);
        contourLabelMapper_->SetLabelFormat("%.0f");
        contourLabelMapper_->SetLabelTextProperty(contourLabelTextProperty_);

        contourLabelActor_->SetMapper(contourLabelMapper_);
}
// ─────────────────────────────────────────────────────────────────────────────
//  Dataset binding
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::setDataset(TopoDataset *dataset) {
  if (dataset_ == dataset) return;

  // Disconnect from any previous dataset
  if (dataset_) {
    disconnect(dataset_, nullptr, this, nullptr);
  }

  dataset_ = dataset;
  emit datasetChanged();

  if (!dataset_) return;

  // Forward error signal directly to QML
  connect(dataset_, &TopoDataset::errorOccurred,
          this,     &TopoDataItem::errorOccurred);

  // A new file load requires a full pipeline rebuild
  connect(dataset_, &TopoDataset::dataLoaded,
          this,     &TopoDataItem::onDatasetLoaded);

  // A quality change only needs a re-render (the polyData is already modified)
  connect(dataset_, &TopoDataset::qualityChanged,
          this,     &TopoDataItem::onQualityChanged);

  // If the dataset already has data (bound after load), rebuild now
  if (dataset_->isLoaded() && pipeline_) {
    onDatasetLoaded();
  }
}

// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::onDatasetLoaded() {
  if (!pipeline_) return;   // initializeVTK not yet called; assemblePipeline
                             // will call connectDataset when the window opens
  pipeline_->firstRender_ = true;
  // Keep displayed filename in sync with what the dataset loaded
  setDataFilename(dataset_->reader()->GetFileName()
                  ? QString(dataset_->reader()->GetFileName()) : QString());

  // If swath is loaded, set navigation track
  if (dataset_->reader()->getDataType() == TopoDataType::Swath) {
    SwathData *data = (SwathData *)dataset_->reader()->topoData();
    setNavigationTrack(data->navTrackPoints());
  }
  
  reassemblePipeline();
}

void TopoDataItem::onQualityChanged() {
  // polyData()->Modified() was already called by TopoDataset::setPointQuality.
  // A bare Render() is enough — VTK's demand-driven execution will propagate
  // the modified MTime through the pipeline automatically.
  dispatch_async([](vtkRenderWindow *rw, vtkUserData) {
    rw->Render();
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  QQuickVTKItem overrides
// ─────────────────────────────────────────────────────────────────────────────

QQuickVTKItem::vtkUserData
TopoDataItem::initializeVTK(vtkRenderWindow *renderWindow) {
  std::cerr << "initializeVTK()\n";
  renderWindow_ = renderWindow;
  pipeline_ = new TopoDataItem::Pipeline();
  renderWindow->AddRenderer(pipeline_->renderer_);
  pipeline_->interactorStyle_ = pickInteractorStyle_;
  assemblePipeline(pipeline_);
  setupLightSource();

  // Dataset may have been loaded before this item's VTK context existed
  // (e.g. edit window starts invisible). Assemble the pipeline now if so.
  if (dataset_ && dataset_->isLoaded()) {
    assemblePipeline(pipeline_);
  }
  
  return pipeline_;
}

void TopoDataItem::destroyingVTK(vtkRenderWindow *renderWindow,
                                  vtkUserData userData) {
  qInfo() << "TopoDataItem::destroyingVTK() not implemented";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Full assembly
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::assemblePipeline(TopoDataItem::Pipeline *pipeline) {
  qDebug() << "assemblePipeline() — full rebuild";

  pipeline->surfaceMapper_->RemoveAllInputConnections(0);
  pipeline->renderer_->RemoveAllViewProps();
  pipeline->renderer_->RemoveAllLights();

  auto *sp = pipeline->surfaceActor_->GetShaderProperty();
  sp->ClearAllVertexShaderReplacements();
  sp->ClearAllFragmentShaderReplacements();

  // Stage 1: wire the shared polyData into this pipeline.
  // connectDataset() returns false if the dataset is not yet loaded
  // (e.g. on first init before a file has been opened), in which case return.
  // The remaining stages are silently skipped; the
  // window renders an empty scene until a file is loaded.
  if (!connectDataset(pipeline)) {
    return;
  }

  // Stages 2–N: apply current display settings
  applyColormap(pipeline);
  applyColoredScalar(pipeline);   // also calls applyShadowSource() internally
  applyRenderType(pipeline);
  applyVerticalExagg(pipeline);
  applyAxes(pipeline);
  applyContours(pipeline);

  // Final actors / lights / interactor
  pipeline->renderer_->AddActor(pipeline->surfaceActor_);
  applyNavTrack(pipeline);
  pipeline->renderer_->AddLight(pipeline->lightSource_);
  pipeline->renderer_->SetBackground(
      pipeline->colors_->GetColor3d("White").GetData());

  pipeline->interactorStyle_->SetDefaultRenderer(pipeline->renderer_);
  pipeline->windowInteractor_->SetPicker(pipeline->areaPicker_);
  pipeline->windowInteractor_->SetInteractorStyle(pipeline->interactorStyle_);
  pipeline->windowInteractor_->SetRenderWindow(renderWindow_);

  if (pipeline->firstRender_) {
    pipeline->renderer_->ResetCamera();
  }
  pipeline->firstRender_ = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 1 — connect shared dataset (replaces loadDataPipeline)
// ─────────────────────────────────────────────────────────────────────────────

bool TopoDataItem::connectDataset(Pipeline *pipeline) {
  if (!dataset_ || !dataset_->isLoaded()) {
    qDebug() << "connectDataset(): no loaded dataset — skipping";
    return false;
  }

  // Wrap the shared polyData in a trivial producer so downstream filters can
  // use SetInputConnection() and VTK's MTime-based re-execution still works.
  pipeline->source_->SetOutput(dataset_->polyData());
  pipeline->polyData_ = dataset_->polyData();

  qDebug() << "connectDataset(): polyData has"
           << pipeline->polyData_->GetNumberOfPoints() << "points";
  return true;
}

void TopoDataItem::initializePipeline() {
  // Kept for compatibility; everything happens in assemblePipeline now.
}

// ─────────────────────────────────────────────────────────────────────────────
//  dataOutputPort — virtual; base returns the full shared dataset
// ─────────────────────────────────────────────────────────────────────────────

vtkAlgorithmOutput *TopoDataItem::dataOutputPort(Pipeline *pipeline) {
  return pipeline->source_->GetOutputPort();
}

void TopoDataItem::reassemblePipeline() {
  qDebug() << "reassemblePipeline() — full rebuild";
  dispatch_async([this](vtkRenderWindow *renderWindow, vtkUserData userData) {
    auto *pipeline = TopoDataItem::Pipeline::SafeDownCast(userData);
    assemblePipeline(pipeline);
    renderWindow_->Render();
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 2 — select colored scalar; route upstream port
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyColoredScalar(Pipeline *pipeline) {
  if (!dataset_ || !dataset_->isLoaded()) return;
  qDebug() << "applyColoredScalar(): " << coloredScalar_;

  switch (coloredScalar_) {
  case ColoredScalar::Elevation:
    // dataOutputPort() carries the "Elevation" scalar from the dataset
    // (base: source_ directly; EditDataItem: after the spatial clip filter)
    coloredOutputPort_ = dataOutputPort(pipeline);
    coloredMin_ = dataset_->elevMin();
    coloredMax_ = dataset_->elevMax();
    pipeline->surfaceMapper_->SetScalarModeToDefault();
    pipeline->surfaceMapper_->SelectColorArray("");
    break;

  case ColoredScalar::Slope: {
    pipeline->normalsFilter_->SetInputConnection(dataOutputPort(pipeline));
    pipeline->normalsFilter_->ComputePointNormalsOn();
    pipeline->normalsFilter_->ComputeCellNormalsOff();
    pipeline->normalsFilter_->SplittingOff();
    pipeline->normalsFilter_->Update();

    pipeline->slopeCalc_->SetInputConnection(
        pipeline->normalsFilter_->GetOutputPort());
    pipeline->slopeCalc_->SetAttributeTypeToPointData();
    pipeline->slopeCalc_->RemoveAllVariables();
    pipeline->slopeCalc_->AddVectorArrayName("Normals");
    pipeline->slopeCalc_->SetFunction("acos(Normals[2]) * 57.2957795");
    pipeline->slopeCalc_->SetResultArrayName("Slopes");
    pipeline->slopeCalc_->Update();

    coloredOutputPort_ = pipeline->slopeCalc_->GetOutputPort();

    vtkPolyData *slopeOutput =
        vtkPolyData::SafeDownCast(pipeline->slopeCalc_->GetOutput());
    if (slopeOutput) {
      vtkDataArray *slopesArray =
          slopeOutput->GetPointData()->GetArray("Slopes");
      if (slopesArray) {
        double range[2];
        slopesArray->GetRange(range);
        coloredMin_ = range[0];
        coloredMax_ = range[1];
      }
    }
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray("Slopes");
    pipeline->surfaceMapper_->SetScalarModeToUsePointFieldData();
    break;
  }

  case ColoredScalar::DataQuality:
    coloredOutputPort_ = dataOutputPort(pipeline);
    coloredMin_ = BAD_DATA;
    coloredMax_ = GOOD_DATA;
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray(DATA_QUALITY_NAME);
    pipeline->surfaceMapper_->SetScalarModeToUsePointFieldData();
    break;

  default:
    qWarning() << "applyColoredScalar(): unhandled value" << coloredScalar_;
    return;
  }

  applyShadowSource(pipeline);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 3 — configure mapper tail and lighting
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyShadowSource(Pipeline *pipeline) {
  if (!dataset_ || !dataset_->isLoaded() || !coloredOutputPort_) return;
  qDebug() << "applyShadowSource(): " << shadowSource_;

  auto *sp = pipeline->surfaceActor_->GetShaderProperty();
  sp->ClearAllVertexShaderReplacements();
  sp->ClearAllFragmentShaderReplacements();

  switch (shadowSource_) {
  case ShadowSource::Illumination: {
    pipeline->surfaceMapper_->SetInputConnection(coloredOutputPort_);
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
    pipeline->surfaceMapper_->SetScalarRange(coloredMin_, coloredMax_);
    pipeline->surfaceMapper_->SetColorModeToMapScalars();
    lightsEnabled_ = true;
    break;
  }
  case ShadowSource::NoShadows: {
    pipeline->surfaceMapper_->SetInputConnection(coloredOutputPort_);
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
    pipeline->surfaceMapper_->SetScalarRange(coloredMin_, coloredMax_);
    pipeline->surfaceMapper_->SetColorModeToMapScalars();
    lightsEnabled_ = false;
    break;
  }
  case ShadowSource::LocalSlope: {
    pipeline->shadeNormalsFilter_->SetInputConnection(coloredOutputPort_);
    pipeline->shadeNormalsFilter_->ComputePointNormalsOn();
    pipeline->shadeNormalsFilter_->ComputeCellNormalsOff();
    pipeline->shadeNormalsFilter_->SplittingOff();
    pipeline->shadeNormalsFilter_->ConsistencyOn();
    pipeline->shadeNormalsFilter_->AutoOrientNormalsOn();
    pipeline->shadeNormalsFilter_->Update();

    pipeline->slopeColorFilter_->SetInputConnection(coloredOutputPort_);
    if (!slopeCallbackData_) {
      slopeCallbackData_ = new SlopeShader::CallbackData{
          pipeline->slopeColorFilter_.Get(),
          pipeline->shadeNormalsFilter_.Get(),
          pipeline->elevLookupTable_,
          coloredMin_, coloredMax_,
          slopeGamma_,
          minBrightness_};
      pipeline->slopeColorFilter_->
          SetExecuteMethod(SlopeShader::execute, slopeCallbackData_);
      pipeline->slopeColorFilter_->
          SetExecuteMethodArgDelete(SlopeShader::deleteCallbackData);
    } else {
      slopeCallbackData_->elevMin       = coloredMin_;
      slopeCallbackData_->elevMax       = coloredMax_;
      slopeCallbackData_->slopeGamma    = slopeGamma_;
      slopeCallbackData_->minBrightness = minBrightness_;
    }
    pipeline->slopeColorFilter_->Modified();
    pipeline->surfaceMapper_->SetInputConnection(
        pipeline->slopeColorFilter_->GetOutputPort());
    pipeline->surfaceMapper_->SetColorModeToDirectScalars();
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    lightsEnabled_ = false;
    break;
  }
  case ShadowSource::LocalSlopeGpu: {
    pipeline->shadeNormalsFilter_->SetInputConnection(coloredOutputPort_);
    pipeline->shadeNormalsFilter_->ComputePointNormalsOn();
    pipeline->shadeNormalsFilter_->ComputeCellNormalsOff();
    pipeline->shadeNormalsFilter_->SplittingOff();
    pipeline->shadeNormalsFilter_->ConsistencyOn();
    pipeline->shadeNormalsFilter_->AutoOrientNormalsOn();
    pipeline->surfaceMapper_->SetInputConnection(
        pipeline->shadeNormalsFilter_->GetOutputPort());
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
    pipeline->surfaceMapper_->SetScalarRange(coloredMin_, coloredMax_);
    pipeline->surfaceMapper_->SetColorModeToMapScalars();
    SlopeShader::installGpuShader(pipeline->surfaceActor_,
                                  slopeGamma_, minBrightness_);
    lightsEnabled_ = false;
    break;
  }
  default:
    qWarning() << "applyShadowSource(): unhandled value" << shadowSource_;
    return;
  }

  pipeline->surfaceActor_->GetProperty()->SetLighting(lightsEnabled_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 4 — colormap
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyColormap(Pipeline *pipeline) {
  TopoColorMap::makeLUT(colormapScheme_, pipeline->elevLookupTable_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 5 — surface representation
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyRenderType(Pipeline *pipeline) {
  switch (surfaceRenderType_) {
  case SurfaceRenderType::Polys:
    pipeline->surfaceActor_->GetProperty()->SetRepresentationToSurface();
    break;
  case SurfaceRenderType::Wireframe:
    pipeline->surfaceActor_->GetProperty()->SetRepresentationToWireframe();
    break;
  case SurfaceRenderType::PointCloud:
    pipeline->surfaceActor_->GetProperty()->SetRepresentationToPoints();
    break;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 6 — axes visibility
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyAxes(Pipeline *pipeline) {
  pipeline->renderer_->RemoveActor(pipeline->axesActor_);
  if (!showAxes_ || !dataset_ || !dataset_->isLoaded()) return;

  double gb[6];
  dataset_->gridBounds(&gb[0], &gb[1], &gb[2], &gb[3], &gb[4], &gb[5]);
  const double zCenter = 0.5 * (gb[4] + gb[5]);

  pipeline->axesActor_->SetOrigin(0., 0., zCenter);
  pipeline->axesActor_->SetScale (1., 1., verticalExagg_);
  pipeline->axesActor_->SetCamera(pipeline->renderer_->GetActiveCamera());
  setupAxes(pipeline->axesActor_,
            pipeline->colors_,
            pipeline->surfaceMapper_->GetBounds(),
            gb,
            dataset_->reader()->xUnits(),
            dataset_->reader()->yUnits(),
            dataset_->reader()->zUnits(),
            dataset_->reader()->geographicCRS());
  pipeline->renderer_->AddActor(pipeline->axesActor_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 7 — vertical exaggeration
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyVerticalExagg(Pipeline *pipeline) {
  if (!dataset_ || !dataset_->isLoaded()) return;

  const double zCenter = 0.5 * (dataset_->elevMin() + dataset_->elevMax());

  pipeline->surfaceActor_->SetOrigin(0., 0., zCenter);
  pipeline->surfaceActor_->SetScale (1., 1., verticalExagg_);
  pipeline->contourActor_->SetOrigin(0., 0., zCenter);
  pipeline->contourActor_->SetScale (1., 1., verticalExagg_);
  pipeline->trackActor_->SetOrigin  (0., 0., zCenter);
  pipeline->trackActor_->SetScale   (1., 1., verticalExagg_);

  pipeline->contourLabelTransform_->Identity();
  pipeline->contourLabelTransform_->Translate(0., 0.,  zCenter);
  pipeline->contourLabelTransform_->Scale    (1., 1., verticalExagg_);
  pipeline->contourLabelTransform_->Translate(0., 0., -zCenter);
  pipeline->contourLabelTransformFilter_->Modified();

  pipeline->renderer_->ResetCameraClippingRange();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stage 8 — contour lines
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyContours(Pipeline *pipeline) {
  pipeline->renderer_->RemoveActor(pipeline->contourActor_);
  pipeline->renderer_->RemoveActor2D(pipeline->contourLabelActor_);
  if (!showContours_ || !dataset_ || !dataset_->isLoaded()) return;

  pipeline->contourFilter_->SetInputConnection(dataOutputPort(pipeline));
  pipeline->contourFilter_->SetInputArrayToProcess(
      0, 0, 0,
      vtkDataObject::FIELD_ASSOCIATION_POINTS, "Elevation");
  pipeline->contourFilter_->GenerateValues(
      contourCount_, dataset_->elevMin(), dataset_->elevMax());
  pipeline->contourFilter_->Modified();

  pipeline->renderer_->AddActor(pipeline->contourActor_);
  if (showContourLabels_) {
    pipeline->renderer_->AddActor2D(pipeline->contourLabelActor_);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Axes helper
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::setupAxes(vtkCubeAxesActor *axesActor,
                              vtkNamedColors *namedColors,
                              double *surfaceBounds,
                              double *gridBounds,
                              const char *xUnits, const char *yUnits,
                              const char *zUnits,
                              bool geographicCRS) {
  qDebug() << "setupAxes():"
           << " xMin:" << surfaceBounds[0] << "xMax:" << surfaceBounds[1]
           << " yMin:" << surfaceBounds[2] << "yMax:" << surfaceBounds[3]
           << " zMin:" << surfaceBounds[4] << "zMax:" << surfaceBounds[5];

  axesActor->SetBounds(surfaceBounds);
  axesActor->SetXAxisRange(gridBounds[0], gridBounds[1]);
  axesActor->SetYAxisRange(gridBounds[2], gridBounds[3]);
  axesActor->SetZAxisRange(gridBounds[4], gridBounds[5]);

  vtkColor3d axisColor = namedColors->GetColor3d("Black");
  for (int i = 0; i < 3; i++) {
    axesActor->GetTitleTextProperty(i)->SetColor(axisColor.GetData());
    axesActor->GetLabelTextProperty(i)->SetColor(axisColor.GetData());
  }
  axesActor->GetTitleTextProperty(0)->SetFontSize(100);
  axesActor->GetLabelTextProperty(0)->SetFontSize(30);
  axesActor->GetXAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetYAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetZAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->DrawXGridlinesOn();
  axesActor->DrawYGridlinesOn();
  axesActor->SetXTitle(xUnits);
  axesActor->SetYTitle(yUnits);
  axesActor->SetZTitle(zUnits);
  axesActor->SetGridLineLocation(axesActor->VTK_GRID_LINES_FURTHEST);
  axesActor->XAxisMinorTickVisibilityOff();
  axesActor->YAxisMinorTickVisibilityOff();
  axesActor->ZAxisMinorTickVisibilityOff();
  axesActor->SetLabelScaling(0, 0, 0, 0);
  axesActor->SetXLabelFormat(geographicCRS ? "%.2f" : "%.0f");
  axesActor->SetYLabelFormat(geographicCRS ? "%.2f" : "%.0f");
  axesActor->SetScreenSize(15.0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Q_INVOKABLE setters
// ─────────────────────────────────────────────────────────────────────────────

bool TopoDataItem::loadDatafile(QUrl fileUrl) {
  if (!dataset_) {
    qWarning() << "loadDatafile(): no dataset bound — call setDataset() first";
    emit errorOccurred("No dataset bound to this view.");
    return false;
  }
  QString path = fileUrl.toLocalFile();
  setDataFilename(path);        // update label immediately for responsiveness
  return dataset_->loadFile(path);
  // onDatasetLoaded() will fire via the dataLoaded signal and trigger rebuild
}

void TopoDataItem::setColoredScalar(ColoredScalar coloredScalar) {
  qDebug() << "setColoredScalar to " << coloredScalar;
  coloredScalar_ = coloredScalar;
  emit coloredScalarChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyColoredScalar(p);
    rw->Render();
  });
}

void TopoDataItem::setShadowSource(ShadowSource source) {
  qDebug() << "setShadowSource to " << source;
  shadowSource_ = source;
  emit shadowSourceChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyShadowSource(p);
    rw->Render();
  });
}

void TopoDataItem::setSurfaceRenderType(SurfaceRenderType renderType) {
  surfaceRenderType_ = renderType;
  emit surfaceRenderTypeChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyRenderType(p);
    rw->Render();
  });
}

void TopoDataItem::setVerticalExagg(float verticalExagg) {
  verticalExagg_ = verticalExagg;
  emit verticalExaggChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyVerticalExagg(p);
    applyAxes(p);
    rw->Render();
  });
}

void TopoDataItem::setSlopeGamma(double gamma) {
  slopeGamma_ = gamma;
  emit slopeGammaChanged();
  switch (shadowSource_) {
  case ShadowSource::LocalSlopeGpu:
    dispatch_async([this, gamma](vtkRenderWindow *rw, vtkUserData ud) {
      auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
      p->surfaceActor_->GetShaderProperty()->GetFragmentCustomUniforms()
          ->SetUniformf("slopeGamma", static_cast<float>(gamma));
      rw->Render();
    });
    break;
  case ShadowSource::LocalSlope:
    dispatch_async([this, gamma](vtkRenderWindow *rw, vtkUserData ud) {
      auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
      if (slopeCallbackData_) {
        slopeCallbackData_->slopeGamma = gamma;
        p->slopeColorFilter_->Modified();
      }
      rw->Render();
    });
    break;
  default:
    break;
  }
}

void TopoDataItem::setMinBrightness(double minBrightness) {
  minBrightness_ = minBrightness;
  emit minBrightnessChanged();
  switch (shadowSource_) {
  case ShadowSource::LocalSlopeGpu:
    dispatch_async([this, minBrightness](vtkRenderWindow *rw, vtkUserData ud) {
      auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
      p->surfaceActor_->GetShaderProperty()->GetFragmentCustomUniforms()
          ->SetUniformf("minBrightness", static_cast<float>(minBrightness));
      rw->Render();
    });
    break;
  case ShadowSource::LocalSlope:
    dispatch_async([this, minBrightness](vtkRenderWindow *rw, vtkUserData ud) {
      auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
      if (slopeCallbackData_) {
        slopeCallbackData_->minBrightness = minBrightness;
        p->slopeColorFilter_->Modified();
      }
      rw->Render();
    });
    break;
  default:
    break;
  }
}

bool TopoDataItem::setColormap(QString name) {
  QByteArray ba = name.toLocal8Bit();
  TopoColorMap::Scheme scheme = TopoColorMap::schemeFromName(ba.data());
  if (scheme == TopoColorMap::Unknown) return false;
  colormapScheme_ = scheme;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyColormap(p);
    rw->Render();
  });
  return true;
}

void TopoDataItem::setShowAxes(bool plotAxes) {
  qDebug() << "setShowAxes(): " << plotAxes;
  showAxes_ = plotAxes;
  emit showAxesChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyAxes(p);
    rw->Render();
  });
}

void TopoDataItem::setContours(bool showContours) {
  qDebug() << "setContours(): " << showContours;
  showContours_ = showContours;
  emit showContoursChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyContours(p);
    rw->Render();
  });
}

void TopoDataItem::setShowContourLabels(bool enabled) {
  showContourLabels_ = enabled;
  emit showContourLabelsChanged();
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyContours(p);
    rw->Render();
  });
}

void TopoDataItem::setContourInterval(double interval) {
  if (!dataset_ || !dataset_->isLoaded()) return;
  const double range = dataset_->elevMax() - dataset_->elevMin();
  if (interval <= 0.0 || range <= 0.0) return;
  setContourCount(static_cast<int>(range / interval));
}

void TopoDataItem::setContourCount(int n) {
  if (n < 1) n = 1;
  contourCount_ = n;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyContours(p);
    rw->Render();
  });
}

double TopoDataItem::getContourInterval() {
  if (!dataset_ || !dataset_->isLoaded() || contourCount_ < 1) return 0.0;
  return (dataset_->elevMax() - dataset_->elevMin()) / contourCount_;
}

void TopoDataItem::setDataFilename(const QString newName) {
  if (dataFilename_ == newName) return;
  dataFilename_ = newName;
  emit dataFilenameChanged(dataFilename_);
}

QString TopoDataItem::getDataFileName() const {
  return dataFilename_;
}

void TopoDataItem::setPickedPoint(double *worldCoords) {
  pointPicked_     = true;
  qDebug() << "setPickedPoint() " << worldCoords[0] << " " <<
    worldCoords[1] << " " << worldCoords[2];
  
  pickedCoords_[0] = worldCoords[0];
  pickedCoords_[1] = worldCoords[1];
  pickedCoords_[2] = worldCoords[2];
  forceRender_     = true;
}

bool TopoDataItem::setMouseMode(QString mouseMode) {
  qDebug() << "setMouseMode(): " << mouseMode;
  if (mouseMode == MousePanAndZoom)
    pipeline_->interactorStyle_ = pickInteractorStyle_;
  else if (mouseMode == MouseLighting)
    pipeline_->interactorStyle_ = lightingInteractorStyle_;
  else if (mouseMode == MouseDataSelect)
    pipeline_->interactorStyle_ = pointsSelectInteractorStyle_;
  else if (mouseMode == MouseElevProfile)
    pipeline_->interactorStyle_ = drawInteractorStyle_;
  else if (mouseMode == MouseTest)
    pipeline_->interactorStyle_ = testStyle_;
  else {
    qDebug() << "setMouseMode(): " << mouseMode << " not implemented";
    return false;
  }
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    p->interactorStyle_->SetDefaultRenderer(p->renderer_);
    p->windowInteractor_->SetInteractorStyle(p->interactorStyle_);
    rw->Render();
  });
  return true;
}

void TopoDataItem::setupLightSource() {
  vtkLight *light = pipeline_->lightSource_;
  light->SetColor(1.0, 1.0, 1.0);
  light->SetPosition(lightPosition_);
  light->SetFocalPoint(0.0, 0.0, 0.0);
  light->SetIntensity(lightIntensity_);
}

void TopoDataItem::setLight(bool lightsEnabled, float intensity,
                             double x, double y, double z) {
  lightsEnabled_   = lightsEnabled;
  lightIntensity_  = intensity;
  lightPosition_[0]= x;
  lightPosition_[1]= y;
  lightPosition_[2]= z;
  dispatch_async([this, lightsEnabled, intensity, x, y, z]
                 (vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    p->lightSource_->SetIntensity(intensity);
    p->lightSource_->SetPosition(x, y, z);
    p->surfaceActor_->GetProperty()->SetLighting(lightsEnabled);
    rw->Render();
  });
}

QVariantList TopoDataItem::getLightPosition() {
  return { lightPosition_[0], lightPosition_[1], lightPosition_[2] };
}

double TopoDataItem::getLightIntensity() {
  return lightIntensity_;
}

TopoDataReader *TopoDataItem::getDataReader() {
  return dataset_ ? dataset_->reader() : nullptr;
}

vtkPolyData *TopoDataItem::getPolyData() {
  return pipeline_ ? pipeline_->polyData_ : nullptr;
}

void TopoDataItem::resetCamera() {
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    p->renderer_->ResetCamera();
    rw->Render();
  });
}

void TopoDataItem::setPBR(double roughness, double metallic) {
  if (!pipeline_ || !pipeline_->surfaceActor_) return;
  vtkProperty *prop = pipeline_->surfaceActor_->GetProperty();
  prop->SetInterpolationToPBR();
  prop->SetRoughness(roughness);
  prop->SetMetallic(metallic);
}

void TopoDataItem::setOrthographicView() {
  pipeline_->renderer_->ResetCamera();
  double bounds[6];
  pipeline_->surfaceActor_->GetBounds(bounds);
  double cx = (bounds[0] + bounds[1]) / 2.0;
  double cy = (bounds[2] + bounds[3]) / 2.0;
  double cz = (bounds[4] + bounds[5]) / 2.0;
  double cameraHeight  = bounds[5] + 2000.0;
  double parallelScale = (bounds[3] - bounds[2]) / 2.0;
  auto camera = pipeline_->renderer_->GetActiveCamera();
  camera->ParallelProjectionOn();
  camera->SetPosition(cx, cy, cameraHeight);
  camera->SetFocalPoint(cx, cy, cz);
  camera->SetViewUp(0.0, 1.0, 0.0);
  camera->SetParallelScale(parallelScale);
  camera->SetClippingRange(1.0, cameraHeight - bounds[4] + 100.0);
  renderWindow_->Render();
}

bool TopoDataItem::saveSettings() {
  auto configPath = std::filesystem::path(getenv("HOME"))
    / ".config"
    / QCoreApplication::applicationName().toStdString()
    / "settings.toml";
  return TopoDataItemSettings::save(configPath, this);
}

bool TopoDataItem::loadSettings() {
  auto configPath = std::filesystem::path(getenv("HOME"))
    / ".config"
    / QCoreApplication::applicationName().toStdString()
    / "settings.toml";
  return TopoDataItemSettings::load(configPath, this);
}

void TopoDataItem::foo() {
  std::cout << "in TopoDataItem::foo()\n";
}

const char *TopoDataItem::getColormapScheme() {
  return TopoColorMap::schemeName(colormapScheme_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Navigation track
// ─────────────────────────────────────────────────────────────────────────────

bool TopoDataItem::setNavigationTrack(const std::vector<double> &x,
				      const std::vector<double> &y,
				      const std::vector<double> &z) {
  if (x.size() != y.size() || x.size() != z.size() || x.empty()) {
    qWarning() << "setNavigationTrack(): mismatched or empty coordinate arrays";
    return false;
  }

  // Build a single polyline cell connecting all points in order.
  vtkNew<vtkPoints>    pts;
  vtkNew<vtkCellArray> lines;

  const vtkIdType n = static_cast<vtkIdType>(x.size());
  pts->SetNumberOfPoints(n);
  lines->InsertNextCell(n);
  for (vtkIdType i = 0; i < n; ++i) {
    pts->SetPoint(i, x[i], y[i], z[i]);
    lines->InsertCellPoint(i);
  }

  trackPolyData_->SetPoints(pts);
  trackPolyData_->SetLines(lines);
  trackPolyData_->Modified();
  showNavTrack_ = true;

  qDebug() << "setNavigationTrack():" << n << "points";

  if (!pipeline_) return false;   // initializeVTK not yet called; applyNavTrack will
                             // run during the next assemblePipeline
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyNavTrack(p);
    rw->Render();
  });

  return true;
}



bool TopoDataItem::setNavigationTrack(vtkPoints *points) {

  // Build a single polyline cell connecting all points in order.
  vtkNew<vtkCellArray> lines;

  const vtkIdType n = points->GetNumberOfPoints();
  
  lines->InsertNextCell(n);
  for (vtkIdType i = 0; i < n; ++i) {
    lines->InsertCellPoint(i);
  }

  trackPolyData_->SetPoints(points);
  trackPolyData_->SetLines(lines);
  trackPolyData_->Modified();
  showNavTrack_ = true;

  qDebug() << "setNavigationTrack():" << n << "points";

  if (!pipeline_) return false;   // initializeVTK not yet called; applyNavTrack will
                                  // run during the next assemblePipeline
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyNavTrack(p);
    rw->Render();
  });

  return true;
}



void TopoDataItem::setShowNavTrack(bool show) {
  showNavTrack_ = show;
  qDebug() << "setShowNavTrack():" << show;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyNavTrack(p);
    rw->Render();
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  applyNavTrack — add/remove track actor; called from assemblePipeline and
//  whenever the track data or visibility changes.
// ─────────────────────────────────────────────────────────────────────────────

void TopoDataItem::applyNavTrack(Pipeline *pipeline) {
  // Always remove first so we start from a clean state (handles hide and
  // re-assembly without duplicating the actor).
  pipeline->renderer_->RemoveActor(pipeline->trackActor_);

  if (!showNavTrack_ || trackPolyData_->GetNumberOfPoints() == 0) return;

  pipeline->trackMapper_->SetInputData(trackPolyData_);
  pipeline->trackMapper_->ScalarVisibilityOff();

  pipeline->trackActor_->GetProperty()->SetColor(0.0, 0.0, 0.0);  // black
  pipeline->trackActor_->GetProperty()->SetLineWidth(2.0);
  pipeline->trackActor_->GetProperty()->LightingOff();

  // Apply the same vertical-exaggeration transform as the surface actor so
  // the track follows the rendered surface geometry.
  if (dataset_ && dataset_->isLoaded()) {
    const double zCenter = 0.5 * (dataset_->elevMin() + dataset_->elevMax());
    pipeline->trackActor_->SetOrigin(0., 0., zCenter);
    pipeline->trackActor_->SetScale (1., 1., verticalExagg_);
  }

  pipeline->renderer_->AddActor(pipeline->trackActor_);
}
