/// #define QT_NO_DEBUG_OUTPUT

#include <unistd.h>
#include <climits>
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
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <QQuickWindow>
#include <QMessageBox>
#include "TopoDataItem.h"
#include "TopoColorMap.h"
#include "SharedConstants.h"
#include "SlopeShader.h"

using namespace mb_system;

/// Define TopoDataItem::Pipeline::New() (factory method)
vtkStandardNewMacro(TopoDataItem::Pipeline);


TopoDataItem::TopoDataItem() {
  dataFilename_ = strdup("");
  verticalExagg_ = 1.;
  showAxes_ = false;
  scheme_ = TopoColorMap::Haxby;
  coloredScalar_ = ColoredScalar::Elevation;
  pointPicked_ = false;
  forceRender_ = false;

  // Instantiate interactor styles
  pickInteractorStyle_ = new PickInteractorStyle(this);
  lightingInteractorStyle_ = new LightingInteractorStyle(this);

  pointsSelectInteractorStyle_->setTopoDataItem(this);
  pointsSelectInteractorStyle_->
    setDrawingMode(MyRubberBandStyle::DrawingMode::Rectangle);

  drawInteractorStyle_->setTopoDataItem(this);
  drawInteractorStyle_->setDrawingMode(DrawInteractorStyle::DrawingMode::Line);

  testStyle_->setTopoDataItem(this);
  testStyle_->setDrawingMode(DrawInteractorStyle::DrawingMode::Line);
}


QQuickVTKItem::vtkUserData
TopoDataItem::initializeVTK(vtkRenderWindow *renderWindow) {
  std::cerr << "initializeVTK()\n";
  renderWindow_ = renderWindow;
  pipeline_ = new TopoDataItem::Pipeline();
  renderWindow->AddRenderer(pipeline_->renderer_);
  pipeline_->interactorStyle_ = pickInteractorStyle_;

  // Full assembly (will be a no-op for the data load until a file is set)
  assemblePipeline(pipeline_);
  setupLightSource();
  return pipeline_;
}


void TopoDataItem::destroyingVTK(vtkRenderWindow *renderWindow,
                                 vtkUserData userData) {
  qInfo() << "TopoDataItem::destroyingVTK() not implemented";
  return;
}


// vtkCubeAxesActor version
void TopoDataItem::setupAxes(vtkCubeAxesActor *axesActor,
                             vtkNamedColors *namedColors,
                             double *surfaceBounds,
                             double *gridBounds,
                             const char *xUnits, const char *yUnits,
                             const char *zUnits,
                             bool geographicCRS) {

  qDebug() << "setupAxes(): " <<
    " xMin: " << surfaceBounds[0] << ", xMax: " << surfaceBounds[1] <<
    ", yMin: " << surfaceBounds[2] << ", yMax: " << surfaceBounds[3] <<
    ", zMin: " << surfaceBounds[4] << ", zMax: " << surfaceBounds[5];

  axesActor->SetBounds(surfaceBounds);
  axesActor->SetXAxisRange(gridBounds[0], gridBounds[1]);
  axesActor->SetYAxisRange(gridBounds[2], gridBounds[3]);
  axesActor->SetZAxisRange(gridBounds[4], gridBounds[5]);

  vtkColor3d axisColor = namedColors->GetColor3d("Black");

  axesActor->GetTitleTextProperty(0)->SetColor(axisColor.GetData());
  axesActor->GetTitleTextProperty(0)->SetFontSize(100);
  axesActor->GetLabelTextProperty(0)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(0)->SetFontSize(30);

  axesActor->GetTitleTextProperty(1)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(1)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(2)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(2)->SetColor(axisColor.GetData());

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
  if (geographicCRS) {
    axesActor->SetXLabelFormat("%.2f");
    axesActor->SetYLabelFormat("%.2f");
  }
  else {
    axesActor->SetXLabelFormat("%.0f");
    axesActor->SetYLabelFormat("%.0f");
  }

  axesActor->SetScreenSize(15.0);
}


void TopoDataItem::initializePipeline() {
  // Kept for compatibility; everything happens in assemblePipeline now.
}


bool TopoDataItem::loadDatafile(QUrl fileUrl) {
  char *filename = strdup(fileUrl.toLocalFile().toLatin1().data());
  qDebug() << "loadGridfile " << filename;
  setDataFilename(filename);

  pipeline_->firstRender_ = true;

  // New data → full rebuild
  reassemblePipeline();
  return true;
}


void TopoDataItem::reassemblePipeline() {
  qDebug() << "reassemblePipeline() — full rebuild";
  dispatch_async([this](vtkRenderWindow *renderWindow, vtkUserData userData) {
    auto *pipeline = TopoDataItem::Pipeline::SafeDownCast(userData);
    assemblePipeline(pipeline);
    renderWindow_->Render();
  });
}


// ═════════════════════════════════════════════════════════════════════════════
//  Full assembly — called on init and on data file load
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::assemblePipeline(TopoDataItem::Pipeline *pipeline) {

  qDebug() << "assemblePipeline() — full rebuild";

  // Clear stale state from previous runs
  pipeline->surfaceMapper_->RemoveAllInputConnections(0);
  pipeline->renderer_->RemoveAllViewProps();
  pipeline->renderer_->RemoveAllLights();

  // Drop any GPU shader replacements left over from a previous pass
  auto *sp = pipeline->surfaceActor_->GetShaderProperty();
  sp->ClearAllVertexShaderReplacements();
  sp->ClearAllFragmentShaderReplacements();

  // Stage 1: load data
  if (!loadDataPipeline(pipeline)) {
    return;
  }

  // Stages 2–N: apply current settings to the pipeline tail
  applyColormap(pipeline);
  applyColoredScalar(pipeline);    // also calls applyShadowSource() internally
  applyRenderType(pipeline);
  applyVerticalExagg(pipeline);
  applyAxes(pipeline);
  applyContours(pipeline);

  // Final assembly: actors / lights / interactor
  pipeline->renderer_->AddActor(pipeline->surfaceActor_);
  pipeline->renderer_->AddLight(pipeline->lightSource_);
  pipeline->renderer_->SetBackground(
      pipeline->colors_->GetColor3d("White").GetData());

  qDebug() << "assemblePipeline(): GetLightIntensity="
           << pipeline->lightSource_->GetIntensity();

  pipeline->interactorStyle_->SetDefaultRenderer(pipeline->renderer_);
  pipeline->windowInteractor_->SetPicker(pipeline->areaPicker_);
  pipeline->windowInteractor_->SetInteractorStyle(pipeline->interactorStyle_);
  pipeline->windowInteractor_->SetRenderWindow(renderWindow_);

  if (pipeline->firstRender_) {
    pipeline->renderer_->ResetCamera();
  }
  pipeline->firstRender_ = false;

  vtkActorCollection* actors = pipeline->renderer_->GetActors();
  std::cerr << "TOTAL actors: " << actors->GetNumberOfItems() << "\n";
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 1 — read data file, cache bounds, build quality array
// ═════════════════════════════════════════════════════════════════════════════
bool TopoDataItem::loadDataPipeline(Pipeline *pipeline) {
  qDebug() << "loadDataPipeline()";

  if (access(dataFilename_, R_OK) == -1) {
    qWarning() << "Can't access input file " << dataFilename_;
    emit errorOccurred(QString("Cannot access input file ") + dataFilename_);
    dataLoaded_ = false;
    return false;
  }

  qDebug() << "set filename to " << dataFilename_;
  pipeline->topoReader_->SetFileName(dataFilename_);

  TopoDataType dataType = TopoDataReader::getDataType(dataFilename_);
  pipeline->topoReader_->setDataType(dataType);

  pipeline->topoReader_->Modified();
  pipeline->topoReader_->UpdateInformation();
  pipeline->topoReader_->Update();

  unsigned long errorCode = pipeline->topoReader_->GetErrorCode();
  if (errorCode != 0) {
    qWarning() << "grid reader error during Update(): " << errorCode;
    qWarning() << dataFilename_ << ": "
               << vtkErrorCode::GetStringFromErrorCode(errorCode);
    emit errorOccurred(QString("Cannot access input file ") + dataFilename_ +
                       "\n" + vtkErrorCode::GetStringFromErrorCode(errorCode));
    dataLoaded_ = false;
    return false;
  }

  // Tag points/cells with original IDs
  pipeline->idFilter_->SetInputData(pipeline->topoReader_->GetOutput());
  pipeline->idFilter_->SetCellIdsArrayName(ORIGINAL_IDS);
  pipeline->idFilter_->SetPointIdsArrayName(ORIGINAL_IDS);
  pipeline->idFilter_->Update();

  // Cache bounds
  pipeline->topoReader_->gridBounds(&gridBounds_[0], &gridBounds_[1],
                                    &gridBounds_[2], &gridBounds_[3],
                                    &gridBounds_[4], &gridBounds_[5]);
  elevMin_ = gridBounds_[4];
  elevMax_ = gridBounds_[5];

  qDebug() << "xMin: " << gridBounds_[0] << ", xMax: " << gridBounds_[1]
           << " yMin: " << gridBounds_[2] << ", yMax: " << gridBounds_[3]
           << " zMin: " << gridBounds_[4] << ", zMax: " << gridBounds_[5];

  // Elevation filter
  pipeline->elevFilter_->SetInputConnection(pipeline->idFilter_->
                                            GetOutputPort());
  pipeline->elevFilter_->SetLowPoint (0, 0, elevMin_);
  pipeline->elevFilter_->SetHighPoint(0, 0, elevMax_);
  pipeline->elevFilter_->SetScalarRange(elevMin_, elevMax_);
  pipeline->elevFilter_->Update();

  pipeline->polyData_ =
      vtkPolyData::SafeDownCast(pipeline->elevFilter_->GetOutput());

  // Quality array (initially all GOOD_DATA)
  pipeline->quality_->SetName(DATA_QUALITY_NAME);
  pipeline->quality_->SetNumberOfTuples(
      pipeline->polyData_->GetNumberOfPoints());
  for (vtkIdType i = 0; i < pipeline->polyData_->GetNumberOfPoints(); i++) {
    pipeline->quality_->SetValue(i, GOOD_DATA);
  }
  pipeline->polyData_->GetPointData()->AddArray(pipeline->quality_);

  dataLoaded_ = true;
  return true;
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 2 — select colored scalar; route upstream port
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyColoredScalar(Pipeline *pipeline) {
  if (!dataLoaded_) return;

  qDebug() << "applyColoredScalar(): " << coloredScalar_;

  switch (coloredScalar_) {

  case ColoredScalar::Elevation:
    coloredOutputPort_ = pipeline->elevFilter_->GetOutputPort();
    coloredMin_ = elevMin_;
    coloredMax_ = elevMax_;
    pipeline->surfaceMapper_->SetScalarModeToDefault();
    pipeline->surfaceMapper_->SelectColorArray("");
    break;

  case ColoredScalar::Slope: {
    // Compute per-vertex normals then derive a "Slopes" point-data array
    pipeline->normalsFilter_->SetInputConnection(
        pipeline->elevFilter_->GetOutputPort());
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
    coloredOutputPort_ = pipeline->elevFilter_->GetOutputPort();
    coloredMin_ = BAD_DATA;
    coloredMax_ = GOOD_DATA;
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray(DATA_QUALITY_NAME);
    pipeline->surfaceMapper_->SetScalarModeToUsePointFieldData();
    break;

  default:
    qWarning() << "Unhandled coloredScalar_: " << coloredScalar_;
    return;
  }

  // The shadow source consumes coloredOutputPort_, so re-apply it.
  applyShadowSource(pipeline);
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 3 — configure mapper tail and lighting based on shadowSource_
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyShadowSource(Pipeline *pipeline) {
  if (!dataLoaded_ || !coloredOutputPort_) return;

  qDebug() << "applyShadowSource(): " << shadowSource_;

  // Always start with a clean shader state — cheap; just empties a map.
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

  case ShadowSource::None: {
    pipeline->surfaceMapper_->SetInputConnection(coloredOutputPort_);
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
    pipeline->surfaceMapper_->SetScalarRange(coloredMin_, coloredMax_);
    pipeline->surfaceMapper_->SetColorModeToMapScalars();
    lightsEnabled_ = false;
    break;
  }

  case ShadowSource::LocalSlope: {
    // CPU path: programmable filter writes RGBA per vertex
    pipeline->shadeNormalsFilter_->SetInputConnection(coloredOutputPort_);
    pipeline->shadeNormalsFilter_->ComputePointNormalsOn();
    pipeline->shadeNormalsFilter_->ComputeCellNormalsOff();
    pipeline->shadeNormalsFilter_->SplittingOff();
    pipeline->shadeNormalsFilter_->ConsistencyOn();
    pipeline->shadeNormalsFilter_->AutoOrientNormalsOn();
    pipeline->shadeNormalsFilter_->Update();

    pipeline->slopeColorFilter_->SetInputConnection(coloredOutputPort_);

    if (!slopeCallbackData_) {
      // First time entering CPU slope mode — allocate and register.
      // Pipeline owns it from here via SetExecuteMethodArgDelete.
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
      // Reuse: just update parameters (pointer stays registered).
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
    // GPU path: normals in series, mapper does LUT, fragment shader darkens
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
    qWarning() << "Unhandled shadowSource: " << shadowSource_;
    return;
  }

  pipeline->surfaceActor_->GetProperty()->SetLighting(lightsEnabled_);
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 4 — colormap
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyColormap(Pipeline *pipeline) {
  TopoColorMap::makeLUT(scheme_, pipeline->elevLookupTable_);
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 5 — surface representation
// ═════════════════════════════════════════════════════════════════════════════
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


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 6 — axes visibility
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyAxes(Pipeline *pipeline) {
  // Idempotent: remove first, then add back if needed
  pipeline->renderer_->RemoveActor(pipeline->axesActor_);

  if (showAxes_ && dataLoaded_) {
    // Pivot Z-scaling about the data's vertical centre so the axes box
    // tracks the surface actor (which uses the same pivot in
    // applyVerticalExagg) instead of swinging around world Z=0.
    const double zCenter = 0.5 * (gridBounds_[4] + gridBounds_[5]);
    pipeline->axesActor_->SetOrigin(0., 0., zCenter);
    pipeline->axesActor_->SetScale (1., 1., verticalExagg_);

    pipeline->axesActor_->SetCamera(pipeline->renderer_->GetActiveCamera());
    setupAxes(pipeline->axesActor_,
              pipeline->colors_,
              pipeline->surfaceMapper_->GetBounds(),
              gridBounds_,
              pipeline->topoReader_->xUnits(),
              pipeline->topoReader_->yUnits(),
              pipeline->topoReader_->zUnits(),
              pipeline->topoReader_->geographicCRS());
    pipeline->renderer_->AddActor(pipeline->axesActor_);
  }
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 7 — vertical exaggeration
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyVerticalExagg(Pipeline *pipeline) {
  // Pivot Z-scaling about the data's vertical centre so the surface
  // grows/shrinks symmetrically around its midplane instead of swinging
  // around world Z=0.
  const double zCenter = 0.5 * (gridBounds_[4] + gridBounds_[5]);

  pipeline->surfaceActor_->SetOrigin(0., 0., zCenter);
  pipeline->surfaceActor_->SetScale (1., 1., verticalExagg_);

  pipeline->contourActor_->SetOrigin(0., 0., zCenter);
  pipeline->contourActor_->SetScale (1., 1., verticalExagg_);

  pipeline->renderer_->ResetCameraClippingRange();
}


// ═════════════════════════════════════════════════════════════════════════════
//  Stage 8 — contour lines
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::applyContours(Pipeline *pipeline) {
  // Idempotent: always remove first.
  pipeline->renderer_->RemoveActor(pipeline->contourActor_);

  if (!contoursEnabled_ || !dataLoaded_) {
    return;
  }

  // Feed contours from the elevation scalar.  Note we don't go through the
  // colored port: contours should reflect elevation regardless of whether
  // the surface is currently colored by Slope or DataQuality.
  pipeline->contourFilter_->SetInputConnection(
      pipeline->elevFilter_->GetOutputPort());

  // Tell vtkContourFilter which scalar drives the iso-extraction.
  pipeline->contourFilter_->SetInputArrayToProcess(
      0, 0, 0,
      vtkDataObject::FIELD_ASSOCIATION_POINTS, "Elevation");

  pipeline->contourFilter_->GenerateValues(contourCount_, elevMin_, elevMax_);
  pipeline->contourFilter_->Modified();

  pipeline->renderer_->AddActor(pipeline->contourActor_);
}


// ═════════════════════════════════════════════════════════════════════════════
//  Q_INVOKABLE setters — dispatch only the narrow stage they affect
// ═════════════════════════════════════════════════════════════════════════════
void TopoDataItem::setColoredScalar(ColoredScalar coloredScalar) {
  qDebug() << "setColoredScalar to " << coloredScalar;
  coloredScalar_ = coloredScalar;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyColoredScalar(p);     // calls applyShadowSource internally
    rw->Render();
  });
}


void TopoDataItem::setShadowSource(ShadowSource source) {
  qDebug() << "setShadowSource to " << source;
  shadowSource_ = source;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyShadowSource(p);
    rw->Render();
  });
}


void TopoDataItem::setSurfaceRenderType(SurfaceRenderType renderType) {
  surfaceRenderType_ = renderType;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyRenderType(p);
    rw->Render();
  });
}


void TopoDataItem::setVerticalExagg(float verticalExagg) {
  verticalExagg_ = verticalExagg;
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyVerticalExagg(p);
    applyAxes(p);  // axes scale with terrain
    rw->Render();
  });
}


void TopoDataItem::setSlopeGamma(double gamma) {
  slopeGamma_ = gamma;
  qDebug() << "setSlopeGamma(" << gamma << ")";

  switch (shadowSource_) {
  case ShadowSource::LocalSlopeGpu:
    // Cheapest path: just write the uniform and render.
    dispatch_async([this, gamma](vtkRenderWindow *rw, vtkUserData ud) {
      auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
      p->surfaceActor_->GetShaderProperty()->GetFragmentCustomUniforms()
          ->SetUniformf("slopeGamma", static_cast<float>(gamma));
      rw->Render();
    });
    break;
  case ShadowSource::LocalSlope:
    // CPU path: update callback data, mark filter dirty, render.
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
    // Gamma has no effect in Illumination / None modes; nothing to do.
    break;
  }
}


void TopoDataItem::setMinBrightness(double minBrightness) {
  minBrightness_ = minBrightness;
  qDebug() << "setMinBrightness(" << minBrightness << ")";

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
  char *cname = ba.data();

  TopoColorMap::Scheme scheme = TopoColorMap::schemeFromName(cname);
  if (scheme == TopoColorMap::Unknown) {
    return false;
  }
  scheme_ = scheme;

  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyColormap(p);
    rw->Render();
  });
  return true;
}


void TopoDataItem::showAxes(bool plotAxes) {
  qDebug() << "showAxes(): " << plotAxes;
  showAxes_ = plotAxes;

  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyAxes(p);
    rw->Render();
  });
}


void TopoDataItem::setContours(bool enabled) {
  qDebug() << "setContours(): " << enabled;
  contoursEnabled_ = enabled;

  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyContours(p);
    rw->Render();
  });
}


void TopoDataItem::setContourCount(int n) {
  qDebug() << "setContourCount(): " << n;
  if (n < 1) n = 1;
  contourCount_ = n;

  // Only meaningful when contours are on; otherwise applyContours is a no-op.
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    applyContours(p);
    rw->Render();
  });
}


void TopoDataItem::setPickedPoint(double *worldCoords) {
  pointPicked_ = true;
  pickedCoords_[0] = worldCoords[0];
  pickedCoords_[1] = worldCoords[1];
  pickedCoords_[2] = worldCoords[2];
  forceRender_ = true;
}


TopoDataReader *TopoDataItem::getDataReader() {
  return pipeline_->topoReader_;
}


bool TopoDataItem::setMouseMode(QString mouseMode) {
  qDebug() << "setMouseMode(): " << mouseMode;

  if (mouseMode == MousePanAndZoom) {
    pipeline_->interactorStyle_ = pickInteractorStyle_;
  }
  else if (mouseMode == MouseLighting) {
    pipeline_->interactorStyle_ = lightingInteractorStyle_;
  }
  else if (mouseMode == MouseDataSelect) {
    pipeline_->interactorStyle_ = pointsSelectInteractorStyle_;
  }
  else if (mouseMode == MouseElevProfile) {
    pipeline_->interactorStyle_ = drawInteractorStyle_;
  }
  else if (mouseMode == MouseTest) {
    pipeline_->interactorStyle_ = testStyle_;
  }
  else {
    qDebug() << "setMouseMode(): " << mouseMode << " not yet implemented";
    return false;
  }

  // Swap the interactor style on the existing interactor — no rebuild.
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    p->interactorStyle_->SetDefaultRenderer(p->renderer_);
    p->windowInteractor_->SetInteractorStyle(p->interactorStyle_);
    rw->Render();
  });
  return true;
}


void TopoDataItem::setupLightSource() {
  qDebug() << "setupLightSource()";
  vtkLight *light = pipeline_->lightSource_;
  light->SetColor(1.0, 1.0, 1.0);
  light->SetPosition(-0.03, 0.24, 0.50);
  light->SetFocalPoint(0.0, 0.0, 0.0);
  light->SetIntensity(1.0);
}


void TopoDataItem::setLight(bool lightsEnabled, float intensity,
                            double x, double y, double z) {
  qDebug() << "setLight()";
  lightsEnabled_ = lightsEnabled;

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
  double position[3];
  pipeline_->lightSource_->GetPosition(position);
  QVariantList result;
  for (int i = 0; i < 3; i++) {
    qDebug() << "getLightPosition(): " << i << ": " << position[i];
    result.append(position[i]);
  }
  return result;
}


double TopoDataItem::getLightIntensity() {
  return pipeline_->lightSource_->GetIntensity();
}


vtkPolyData *TopoDataItem::getPolyData() {
  return pipeline_->polyData_;
}


void TopoDataItem::resetCamera() {
  qDebug() << "resetCamera()";
  dispatch_async([this](vtkRenderWindow *rw, vtkUserData ud) {
    auto *p = TopoDataItem::Pipeline::SafeDownCast(ud);
    p->renderer_->ResetCamera();
    rw->Render();
  });
}


void TopoDataItem::setPBR(double roughness, double metallic) {
  qDebug() << "setPBR(): check pipeline_ pointer";
  if (!pipeline_) {
    qDebug() << "setPBR(): pipeline_ is null";
    return;
  }
  if (!pipeline_->surfaceActor_) {
    qDebug() << "setPBR(): surfaceActor_ is null";
    return;
  }

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
  double cameraHeight = bounds[5] + 2000.0;
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
