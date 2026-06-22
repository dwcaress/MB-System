#include "SurfaceDataItem.h"
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <QDebug>

using namespace mb_system;

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

SurfaceDataItem::SurfaceDataItem()
    : TopoDataItem()
{
    surfaceRenderType_ = SurfaceRenderType::Polys;
    coloredScalar_     = ColoredScalar::Elevation;
}

// ─────────────────────────────────────────────────────────────────────────────
//  assemblePipeline — surface defaults, then base-class assembly
// ─────────────────────────────────────────────────────────────────────────────

void SurfaceDataItem::assemblePipeline(Pipeline *pipeline) {
    surfaceRenderType_ = SurfaceRenderType::Polys;
    coloredScalar_     = ColoredScalar::Elevation;
    // Base class calls connectDataset() → applyColoredScalar() → applyRenderType()
    // etc., in that order.  connectDataset() is overridden below and will
    // initialise qualitySource_ before applyColoredScalar() calls
    // dataOutputPort() (also overridden below).
    TopoDataItem::assemblePipeline(pipeline);
}

// ─────────────────────────────────────────────────────────────────────────────
//  connectDataset — wire quality source AFTER base wires the shared polyData
//
//  IMPORTANT: does NOT change pipeline->source_ or pipeline->polyData_.
//  The rubber-band area picker uses pipeline_->polyData_ directly; keeping it
//  pointed at dataset_->polyData() ensures extraction of all points (good and
//  bad) for the EditDataItem to edit.
// ─────────────────────────────────────────────────────────────────────────────

bool SurfaceDataItem::connectDataset(Pipeline *pipeline) {
    // Base: sets pipeline->source_->SetOutput(dataset_->polyData())
    //       and pipeline->polyData_ = dataset_->polyData().
    //       Returns false if dataset is not yet loaded.
    if (!TopoDataItem::connectDataset(pipeline)) return false;

    // Build the initial good-only geometry (at load time every point is GOOD,
    // so this is a full copy) and wire qualitySource_ to it.
    rebuildSurfacePolyData();
    qualitySource_->SetOutput(surfacePolyData_);

    qDebug() << "SurfaceDataItem::connectDataset(): qualitySource_ wired,"
             << surfacePolyData_->GetNumberOfCells() << "cells in render copy";

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  dataOutputPort — serve the good-only copy to the rendering stages
// ─────────────────────────────────────────────────────────────────────────────

vtkAlgorithmOutput *SurfaceDataItem::dataOutputPort(Pipeline *pipeline) {
    (void)pipeline;
    return qualitySource_->GetOutputPort();
}

// ─────────────────────────────────────────────────────────────────────────────
//  onQualityChanged — rebuild the good-only surface and re-render
// ─────────────────────────────────────────────────────────────────────────────

void SurfaceDataItem::onQualityChanged() {
    // qualityChanged() is emitted on the render thread; this slot is connected
    // with Qt::QueuedConnection so it runs on the main thread first, then we
    // dispatch VTK work back to the render thread.
    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        if (!dataset_ || !dataset_->isLoaded()) {
            rw->Render();
            return;
        }
        rebuildSurfacePolyData();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  rebuildSurfacePolyData — core quality-hiding logic (render thread only)
//
//  Shallow-copies the shared polyData so that points, point data, and cell
//  data arrays are shared (no data duplication).  Then replaces the polys
//  array with a new vtkCellArray that omits every cell whose vertices include
//  at least one BAD_DATA point.  The result is a geometrically consistent
//  surface with holes where bad soundings were removed.
// ─────────────────────────────────────────────────────────────────────────────

void SurfaceDataItem::rebuildSurfacePolyData() {
    vtkPolyData *src = dataset_->polyData();

    // ShallowCopy: new container object, shared underlying array references.
    // We immediately replace the polys array, so the source polys are not
    // aliased after this function returns.
    surfacePolyData_->ShallowCopy(src);

    vtkIntArray *qa = vtkIntArray::SafeDownCast(
        src->GetPointData()->GetArray(DATA_QUALITY_NAME));

    if (!qa) {
        // No quality array yet — nothing to hide, leave the full surface.
        surfacePolyData_->Modified();
        return;
    }

    // Walk source cells; include only those with ALL good vertices.
    vtkNew<vtkCellArray> goodPolys;
    vtkNew<vtkIdList>    ptIds;

    const vtkIdType nCells   = src->GetNumberOfCells();
    vtkIdType       nRemoved = 0;

    for (vtkIdType ci = 0; ci < nCells; ++ci) {
        src->GetCellPoints(ci, ptIds);

        bool anyBad = false;
        for (vtkIdType pi = 0; pi < ptIds->GetNumberOfIds(); ++pi) {
            if (qa->GetValue(ptIds->GetId(pi)) == BAD_DATA) {
                anyBad = true;
                break;
            }
        }

        if (!anyBad) {
            goodPolys->InsertNextCell(ptIds);
        } else {
            ++nRemoved;
        }
    }

    surfacePolyData_->SetPolys(goodPolys);
    surfacePolyData_->Modified();

    qDebug() << "SurfaceDataItem::rebuildSurfacePolyData():"
             << nCells - nRemoved << "/" << nCells << "cells kept,"
             << nRemoved << "removed (bad vertices)";
}

// ─────────────────────────────────────────────────────────────────────────────
//  onRegionSelected — emit editBoundsChanged with the rubber-band XY extent
// ─────────────────────────────────────────────────────────────────────────────

void SurfaceDataItem::onRegionSelected(double worldBounds[6]) {
    qDebug() << "SurfaceDataItem::onRegionSelected() CALLED"
             << "x[" << worldBounds[0] << "," << worldBounds[1] << "]"
             << "y[" << worldBounds[2] << "," << worldBounds[3] << "]"
             << "z[" << worldBounds[4] << "," << worldBounds[5] << "]";

    if (!dataset_) {
        qWarning() << "SurfaceDataItem::onRegionSelected(): dataset_ is null";
        return;
    }
    if (!dataset_->isLoaded()) {
        qWarning() << "SurfaceDataItem::onRegionSelected(): dataset not loaded";
        return;
    }

    const double xMin = worldBounds[0], xMax = worldBounds[1];
    const double yMin = worldBounds[2], yMax = worldBounds[3];
    const double zMin = worldBounds[4], zMax = worldBounds[5];

    qDebug() << "SurfaceDataItem::onRegionSelected(): emitting editBoundsChanged";
    emit editBoundsChanged(xMin, xMax, yMin, yMax, zMin, zMax);
}
