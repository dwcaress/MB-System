#include "EditDataItem.h"
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkIdList.h>
#include <vtkShaderProperty.h>
#include <QDebug>
#include <QMetaObject>

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

    if (!dataset_ || !dataset_->isLoaded()) return;

    // Install pick style directly on the render window interactor.
    // The edit window has no Mouse menu, so this is the only install point.
    if (renderWindow_ && renderWindow_->GetInteractor())
        renderWindow_->GetInteractor()->SetInteractorStyle(pickInteractorStyle_);

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
    pipeline->surfaceActor_->GetProperty()->SetRepresentationToPoints();
    pipeline->surfaceActor_->GetProperty()->SetPointSize(5.0);

    // On OpenGL core profile (required on macOS and Linux), glPointSize() is a
    // no-op — gl_PointSize must be written in the vertex shader.  Inject a
    // replacement that sets it after the PositionVC block.
    {
        auto *sp = pipeline->surfaceActor_->GetShaderProperty();
        sp->ClearAllVertexShaderReplacements();
        sp->AddVertexShaderReplacement(
            "//VTK::PositionVC::Impl",
            true,
            "//VTK::PositionVC::Impl\n"
            "  gl_PointSize = 5.0;\n",
            false);
    }

    // No lighting needed for quality-colour point cloud
    pipeline->surfaceActor_->GetProperty()->LightingOff();
    pipeline->surfaceActor_->GetProperty()->SetAmbient(1.0);
    pipeline->surfaceActor_->GetProperty()->SetDiffuse(0.0);
    pipeline->surfaceActor_->GetProperty()->SetSpecular(0.0);
    // Very dark blue-gray background
    pipeline->renderer_->SetBackground(0.08, 0.08, 0.12);
    qDebug() << "renderer actors:"
             << pipeline->renderer_->GetActors()->GetNumberOfItems();
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
