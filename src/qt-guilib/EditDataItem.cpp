#include "EditDataItem.h"
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkIdList.h>
#include <vtkShaderProperty.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
// GL_PROGRAM_POINT_SIZE may not be exposed by VTK's transitive headers on all
// platforms; define the raw enum value directly (OpenGL 3.2 core, §14.4).
#ifndef GL_PROGRAM_POINT_SIZE
#  define GL_PROGRAM_POINT_SIZE 0x8642
#endif
#include <QDebug>
#include <QMetaObject>
#include <QGuiApplication>
#include <QScreen>

using namespace mb_system;

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

EditDataItem::EditDataItem()
    : TopoDataItem()
{
    // Edit view defaults
    surfaceRenderType_ = SurfaceRenderType::PointCloud;
    coloredScalar_     = ColoredScalar::DataQuality;

    // Box starts collapsed; expanded by the first setEditBounds() call
    clipBox_->SetBounds(0, 0, 0, 0, 0, 0);
    clipFilter_->SetImplicitFunction(clipBox_);
    clipFilter_->ExtractInsideOn();
    clipFilter_->ExtractBoundaryCellsOn();

    buildQualityLut();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Quality LUT — two entries: BAD_DATA (red/transparent) and GOOD_DATA (blue)
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::buildQualityLut() {
    qualityLut_->SetNumberOfTableValues(2);
    qualityLut_->SetTableRange(BAD_DATA, GOOD_DATA);

    // GOOD_DATA: bright yellow — high luminance, very visible on dark background
    qualityLut_->SetTableValue(GOOD_DATA, 1.0, 1.0, 0.0, 1.0);

    // BAD_DATA: bright red; alpha is controlled by showBadPoints_
    const double badAlpha = showBadPoints_ ? 1.0 : 0.0;
    qualityLut_->SetTableValue(BAD_DATA, 1.0, 0.1, 0.1, badAlpha);

    qualityLut_->Build();
}

// ─────────────────────────────────────────────────────────────────────────────
//  dataOutputPort override — clip filter interposes before render stages
// ─────────────────────────────────────────────────────────────────────────────

vtkAlgorithmOutput *EditDataItem::dataOutputPort(Pipeline * /*pipeline*/) {
    // All base-class apply*() methods will now read from the extracted subset.
    return clipFilter_->GetOutputPort();
}

// ─────────────────────────────────────────────────────────────────────────────
//  connectDataset override — wire clip filter, set mapper to quality LUT
// ─────────────────────────────────────────────────────────────────────────────

bool EditDataItem::connectDataset(Pipeline *pipeline) {
    if (!TopoDataItem::connectDataset(pipeline)) return false;

    vtkPolyData *src = dataset_->polyData();

    // Ensure ORIGINAL_IDS exists so setPickedPoint() can map clip-output
    // indices back to the shared quality array.
    // vtkExtractPolyDataGeometry copies existing point-data arrays through to
    // its output, so a subset of these values will appear after Update().
    if (!src->GetPointData()->GetArray(ORIGINAL_IDS)) {
        vtkNew<vtkIdTypeArray> ids;
        ids->SetName(ORIGINAL_IDS);
        ids->SetNumberOfTuples(src->GetNumberOfPoints());
        for (vtkIdType i = 0; i < src->GetNumberOfPoints(); i++)
            ids->SetValue(i, i);
        src->GetPointData()->AddArray(ids);
    }

    auto *srcQA = vtkIntArray::SafeDownCast(
        src->GetPointData()->GetArray(DATA_QUALITY_NAME));
    qDebug() << "source quality before clip:"
             << (srcQA ? srcQA->GetValue(0) : -999);

    // Feed the clip filter from the shared polyData (not from source_, because
    // vtkExtractPolyDataGeometry needs SetInputData for a stable pointer).
    clipFilter_->SetInputData(src);
    if (boundsSet_) {
        clipBox_->SetBounds(editBounds_);
    }
    clipFilter_->Update();

    // Workaround: vtkExtractPolyDataGeometry does not correctly copy
    // vtkIntArray values into its output (values arrive as 0).  Manually sync.
    syncQualityArray();

    auto *qa = vtkIntArray::SafeDownCast(
        clipFilter_->GetOutput()->GetPointData()->GetArray(DATA_QUALITY_NAME));
    qDebug() << "quality in clip output:" << (qa != nullptr);
    if (qa && qa->GetNumberOfTuples() > 0)
        qDebug() << "clip quality[0]:" << qa->GetValue(0);

    // Point locator runs on the extracted result for fast pick → ID mapping
    rebuildLocator();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  assemblePipeline override — apply edit defaults and install quality LUT
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::assemblePipeline(Pipeline *pipeline) {
    qDebug() << "EditDataItem::assemblePipeline"
             << "dataset=" << (bool)dataset_
             << "loaded=" << (dataset_ && dataset_->isLoaded())
             << "boundsSet=" << boundsSet_;

    // Defaults for edit view
    surfaceRenderType_ = SurfaceRenderType::PointCloud;
    coloredScalar_     = ColoredScalar::DataQuality;

    // Base class builds the pipeline (calls connectDataset, applyColoredScalar,
    // applyRenderType, etc.)
    TopoDataItem::assemblePipeline(pipeline);

    // Background and interactor style are edit-view constants — set them
    // regardless of whether the dataset is loaded yet, so the window looks
    // correct even before the first reassembly after load.
    pipeline->renderer_->SetBackground(0.08, 0.08, 0.12);   // very dark blue-gray
    // SetBackground() sets RGB but leaves alpha at 0 (transparent) by default.
    // On Ubuntu + Mesa, Qt Quick composites VTK's FBO over the window background
    // (white), so a transparent VTK background shows as white.  macOS defaults to
    // opaque and is not affected.  Force alpha=1 so the colour is actually visible.
    pipeline->renderer_->SetBackgroundAlpha(1.0);
    if (renderWindow_ && renderWindow_->GetInteractor())
        renderWindow_->GetInteractor()->SetInteractorStyle(pickInteractorStyle_);

    if (!dataset_ || !dataset_->isLoaded()) return;

    // Override: install the quality LUT on the mapper in place of the
    // elevation LUT that applyColoredScalar / applyShadowSource installed.
    pipeline->surfaceMapper_->SetLookupTable(qualityLut_);
    pipeline->surfaceMapper_->SetScalarRange(BAD_DATA, GOOD_DATA);
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray(DATA_QUALITY_NAME);
    pipeline->surfaceMapper_->SetScalarModeToUsePointFieldData();
    pipeline->surfaceMapper_->SetColorModeToMapScalars();

    // Point cloud rendering.
    //
    // Strategy for cross-platform point size on OpenGL core profile:
    //
    // VTK sets a "pointSize" uniform in its built-in vertex shader
    // (//VTK::PointSizeUniform::Impl) from vtkProperty::GetPointSize(), and
    // writes that value to gl_PointSize — but only when GL_PROGRAM_POINT_SIZE is
    // enabled.  glPointSize() is a no-op in core profile.
    //
    // On macOS, Qt Quick / VTK enables GL_PROGRAM_POINT_SIZE automatically.
    // On some Mesa (Ubuntu) configurations it is not enabled by default, so we
    // do it explicitly here via the VTK state tracker so VTK's own context
    // management does not accidentally undo it.
    //
    // gl_PointSize / SetPointSize() is in *physical* (framebuffer) pixels.
    // QQuickVTKItem renders into a HiDPI FBO at device-pixel resolution, so on
    // a 2× Retina display SetPointSize(5) gives 5 device pixels ≈ 2.5 logical
    // pixels.  Multiply by devicePixelRatio so the apparent size matches on
    // every platform.
    {
        auto *primaryScreen = QGuiApplication::primaryScreen();
        const float dpr = primaryScreen
                              ? static_cast<float>(primaryScreen->devicePixelRatio())
                              : 1.0f;

        const float physPtSize = 2.0f * dpr;

        pipeline->surfaceActor_->GetProperty()->SetRepresentationToPoints();
        pipeline->surfaceActor_->GetProperty()->SetPointSize(physPtSize);

        // Belt-and-suspenders: also enable GL_PROGRAM_POINT_SIZE via the VTK
        // state wrapper so the GL capability is on even if VTK would otherwise
        // skip it (e.g. after applyShadowSource() clears the shader property).
        auto *oglrw = vtkOpenGLRenderWindow::SafeDownCast(renderWindow_);
        if (oglrw) {
            oglrw->GetState()->vtkglEnable(GL_PROGRAM_POINT_SIZE);
        }

        // Inject gl_PointSize into the vertex shader explicitly.  Relying on
        // VTK's built-in //VTK::PointSizeUniform::Impl is unreliable here
        // because applyShadowSource() clears all user shader replacements
        // earlier in the same assembly pass, and VTK's own injection may not
        // fire after that.  applyShadowSource() (Illumination case) adds no
        // replacements of its own, so there is nothing to conflict with.
        //
        // Use snprintf("%.1f") not std::to_string(): the latter produces
        // "70.000000" which some GLSL compilers (Mesa) reject as a double
        // literal when assigned to the float gl_PointSize built-in.
        char ptSizeLiteral[32];
        snprintf(ptSizeLiteral, sizeof(ptSizeLiteral), "%.1f", physPtSize);
        const std::string glsl =
            "//VTK::PositionVC::Impl\n"
            "  gl_PointSize = " + std::string(ptSizeLiteral) + ";\n";

        auto *sp = pipeline->surfaceActor_->GetShaderProperty();
        sp->AddVertexShaderReplacement(
            "//VTK::PositionVC::Impl",
            true,
            glsl,
            false);

        qDebug() << "EditDataItem: dpr=" << dpr << " physPtSize=" << physPtSize
                 << " gl_PointSize literal=" << ptSizeLiteral;
    }

    // No lighting needed for quality-colour point cloud
    pipeline->surfaceActor_->GetProperty()->LightingOff();
    pipeline->surfaceActor_->GetProperty()->SetAmbient(1.0);
    pipeline->surfaceActor_->GetProperty()->SetDiffuse(0.0);
    pipeline->surfaceActor_->GetProperty()->SetSpecular(0.0);

    qDebug() << "EditDataItem::assemblePipeline done: actors="
             << pipeline->renderer_->GetActors()->GetNumberOfItems()
             << " clipOutputPts="
             << clipFilter_->GetOutput()->GetNumberOfPoints();
}

// ─────────────────────────────────────────────────────────────────────────────
//  setEditBounds — update clip box and re-render
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setEditBounds(double xMin, double xMax,
                                  double yMin, double yMax,
                                  double zMin, double zMax) {
    editBounds_[0] = xMin;  editBounds_[1] = xMax;
    editBounds_[2] = yMin;  editBounds_[3] = yMax;
    editBounds_[4] = zMin;  editBounds_[5] = zMax;
    boundsSet_ = true;

    qDebug() << "EditDataItem::setEditBounds()"
             << "x[" << xMin << "," << xMax << "]"
             << "y[" << yMin << "," << yMax << "]"
             << "z[" << zMin << "," << zMax << "]";

    // Capture by value: the lambda runs on the render thread
    dispatch_async([this, xMin, xMax, yMin, yMax, zMin, zMax]
                   (vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "setEditBounds() dispatched";
        clipBox_->SetBounds(xMin, xMax, yMin, yMax, zMin, zMax);
        clipFilter_->Modified();
        clipFilter_->Update();
        qDebug() << "clip output points:"
                 << clipFilter_->GetOutput()->GetNumberOfPoints();
        syncQualityArray();
        rebuildLocator();
        rw->GetRenderers()->GetFirstRenderer()->ResetCamera();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  onQualityChanged override — re-sync quality array and re-render
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::onQualityChanged() {
    // The shared polyData's quality array was modified in-place.
    // Re-copy values into the clip output — they won't propagate automatically
    // because vtkExtractPolyDataGeometry cached them incorrectly on Update().
    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        syncQualityArray();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setFlagValue — toggle between BAD_DATA and GOOD_DATA
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setFlagValue(int quality) {
    flagValue_ = quality;
    qDebug() << "EditDataItem::setFlagValue()" << flagValue_;
}

// ─────────────────────────────────────────────────────────────────────────────
//  setShowBadPoints — adjust quality LUT alpha without pipeline rebuild
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setShowBadPoints(bool show) {
    showBadPoints_ = show;
    qDebug() << "EditDataItem::setShowBadPoints()" << show;

    dispatch_async([this, show](vtkRenderWindow *rw, vtkUserData) {
        // Adjust only the alpha of the BAD_DATA entry in the quality LUT
        double rgba[4];
        qualityLut_->GetTableValue(BAD_DATA, rgba);
        rgba[3] = show ? 1.0 : 0.0;
        qualityLut_->SetTableValue(BAD_DATA, rgba);
        qualityLut_->Modified();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setPickedPoint override — world coords → originalId → setPointQuality
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setPickedPoint(double *worldCoords) {
    if (!dataset_ || !dataset_->isLoaded() || !pipeline_) return;

    // Find the nearest point in the *extracted* polyData.
    // clipFilter_->GetOutput() is the same vtkPolyData that pointLocator_ was
    // built on; FindClosestPoint() runs in O(log n) via the KD-tree.
    vtkPolyData *extracted =
        vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
    if (!extracted || extracted->GetNumberOfPoints() == 0) return;

    vtkIdType localId =
        pointLocator_->FindClosestPoint(worldCoords);
    if (localId < 0) return;

    // Map localId (index in the extracted subset) back to the stable
    // ORIGINAL_IDS index used by the shared quality array.
    vtkDataArray *origIds =
        extracted->GetPointData()->GetArray(ORIGINAL_IDS);
    if (!origIds) {
        qWarning() << "EditDataItem::setPickedPoint(): "
                      "ORIGINAL_IDS array not found in extracted polyData";
        return;
    }

    vtkIdType originalId = static_cast<vtkIdType>(origIds->GetTuple1(localId));
    qDebug() << "EditDataItem::setPickedPoint(): localId=" << localId
             << " originalId=" << originalId
             << " flagValue=" << flagValue_;

    // dataset_->setPointQuality() modifies the quality array in the shared
    // polyData, marks it Modified, and emits qualityChanged().  Both the edit
    // and main windows receive the signal and dispatch a re-render.
    QMetaObject::invokeMethod(this, [this, originalId]() {
        dataset_->setPointQuality(originalId, flagValue_);
    }, Qt::QueuedConnection);
}

// ─────────────────────────────────────────────────────────────────────────────
//  rebuildLocator (private, render thread)
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::rebuildLocator() {
    vtkPolyData *extracted =
        vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
    if (!extracted || extracted->GetNumberOfPoints() == 0) return;

    pointLocator_->SetDataSet(extracted);
    pointLocator_->BuildLocator();
    qDebug() << "EditDataItem::rebuildLocator():"
             << extracted->GetNumberOfPoints() << "points indexed";
}

// ─────────────────────────────────────────────────────────────────────────────
//  syncQualityArray (private, render thread)
//  Workaround: vtkExtractPolyDataGeometry does not correctly copy vtkIntArray
//  values into its output (all values arrive as 0).  Manually copy from the
//  shared quality array using ORIGINAL_IDS for the index mapping.
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::syncQualityArray() {
    vtkPolyData *out = clipFilter_->GetOutput();
    if (!out || out->GetNumberOfPoints() == 0) return;

    auto *origIds = vtkIdTypeArray::SafeDownCast(
        out->GetPointData()->GetArray(ORIGINAL_IDS));
    auto *outQA   = vtkIntArray::SafeDownCast(
        out->GetPointData()->GetArray(DATA_QUALITY_NAME));
    vtkIntArray *srcQA = dataset_ ? dataset_->qualityArray() : nullptr;

    if (!origIds || !outQA || !srcQA) {
        qWarning() << "syncQualityArray(): missing array(s)"
                   << "origIds=" << (bool)origIds
                   << "outQA="   << (bool)outQA
                   << "srcQA="   << (bool)srcQA;
        return;
    }

    for (vtkIdType i = 0; i < out->GetNumberOfPoints(); i++)
        outQA->SetValue(i, srcQA->GetValue(origIds->GetValue(i)));
    outQA->Modified();
}
