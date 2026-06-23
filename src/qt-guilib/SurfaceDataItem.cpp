#include "SurfaceDataItem.h"
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QEvent>
#include <QQuickWindow>

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

    // Persistent selection rectangle — flat 4-edge outline, invisible until
    // the first rubber-band pick.
    // SetInputData so the mapper reads selectionRectPolyData_ directly; we
    // rebuild the points/lines in-place in onRegionSelected() and just call
    // Modified().
    selectionOutlineMapper_->SetInputData(selectionRectPolyData_);
    selectionOutlineActor_->SetMapper(selectionOutlineMapper_);
    selectionOutlineActor_->GetProperty()->SetColor(0.0, 0.0, 0.0); // black
    selectionOutlineActor_->GetProperty()->SetLineWidth(2.5);
    selectionOutlineActor_->GetProperty()->LightingOff();
    selectionOutlineActor_->VisibilityOff();
    // SetUseBounds(false): exclude this actor from ResetCamera() /
    // ResetCameraClippingRange() / ComputeVisiblePropBounds().
    // Without this, once VisibilityOn() is called (after the first rubber-band
    // selection), the outline's bounds participate in clipping-plane calculation.
    // The outline is in model/unscaled coordinates while the surface actor has a
    // vertical-exaggeration scale transform, so the two actors' world-space bounds
    // diverge.  That mismatch shifts the near/far clip planes and pushes the
    // surface out of view on the next camera interaction.
    // SetUseBounds(false) keeps the actor fully visible but never lets it affect
    // camera geometry.
    selectionOutlineActor_->SetUseBounds(false);
    // RemoveActor before AddActor: connectDataset() is called on every pipeline
    // reassembly; without this guard, duplicate actor entries corrupt VTK's
    // ResetCameraClippingRange() and push the camera out of view.
    pipeline->renderer_->RemoveActor(selectionOutlineActor_);
    pipeline->renderer_->AddActor(selectionOutlineActor_);

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
    // On Ubuntu/X11 qualityChanged fires from the VTK render thread.
    //
    // QQuickItem::update() and QQuickWindow::update() are BOTH rejected from
    // the render thread ("Updates can only be scheduled from GUI thread or from
    // QQuickItem::updatePaintNode()"), so dispatch_async's internal update()
    // call is silently dropped and the callback never fires.
    //
    // QMetaObject::invokeMethod(..., Qt::QueuedConnection) posts to the main
    // thread but that event is never drained in Qt Quick's threaded render loop.
    //
    // What DOES work: QCoreApplication::postEvent with QEvent::UpdateRequest.
    // This is always thread-safe and routes through Qt Quick's own render-
    // scheduling machinery rather than the general event queue, so the render
    // loop picks it up and calls updatePaintNode() — which runs the dispatch_async
    // callback.
    //
    // On macOS qualityChanged fires from the main thread so dispatch_async alone
    // is sufficient; the postEvent is harmless there.

    bool onMainThread = (QThread::currentThread() == thread());
    qDebug() << "SurfaceDataItem::onQualityChanged()"
             << "onMainThread=" << onMainThread
             << "window=" << (window() ? "ok" : "NULL");

    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "SurfaceDataItem::onQualityChanged() dispatch_async callback fired";
        if (!dataset_ || !dataset_->isLoaded()) {
            rw->Render();
            return;
        }
        rebuildSurfacePolyData();
        rw->Render();
    });

    // Trigger a render of this window via UpdateRequest, which bypasses the
    // render-thread restriction on update() and uses Qt Quick's own scheduling.
    if (QQuickWindow *w = window()) {
        qDebug() << "  -> posting QEvent::UpdateRequest";
        QCoreApplication::postEvent(w,
            new QEvent(QEvent::UpdateRequest),
            Qt::HighEventPriority);
    }
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
    qDebug() << "SurfaceDataItem::rebuildSurfacePolyData() ENTERED";
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

    // Update the persistent selection rectangle only when the rubber-band covers
    // a real area.  A plain left-click produces worldBounds where xMin≈xMax
    // and/or yMin≈yMax.  The threshold is 0.01% of each dataset axis — small
    // enough to pass any real rubber-band but large enough to exclude single-click
    // noise.
    if (pipeline_) {
        double *db = dataset_->polyData()->GetBounds(); // [xMin,xMax,yMin,yMax,zMin,zMax]
        const double dsXRange = db[1] - db[0];
        const double dsYRange = db[3] - db[2];
        const bool realSelection =
            (xMax - xMin) > 1e-4 * dsXRange &&
            (yMax - yMin) > 1e-4 * dsYRange;

        if (realSelection) {
            // Build a flat 4-edge rectangle at zMax (topmost world Z in the
            // selection).  Using worldBounds[5] (the actual world-space max Z
            // returned by the area picker) places the rectangle at the surface
            // level regardless of vertical exaggeration, since worldBounds are
            // already in world/camera space rather than model space.
            vtkNew<vtkPoints> pts;
            pts->SetNumberOfPoints(4);
            pts->SetPoint(0, xMin, yMin, zMax);
            pts->SetPoint(1, xMax, yMin, zMax);
            pts->SetPoint(2, xMax, yMax, zMax);
            pts->SetPoint(3, xMin, yMax, zMax);

            vtkNew<vtkCellArray> edges;
            auto addEdge = [&](vtkIdType a, vtkIdType b) {
                vtkIdType ids[2] = {a, b};
                edges->InsertNextCell(2, ids);
            };
            addEdge(0, 1);
            addEdge(1, 2);
            addEdge(2, 3);
            addEdge(3, 0);

            selectionRectPolyData_->SetPoints(pts);
            selectionRectPolyData_->SetLines(edges);
            selectionRectPolyData_->Modified();
            selectionOutlineActor_->VisibilityOn();

            // We're inside SurfaceDataItem's own updatePaintNode() here, so
            // dispatch_async is explicitly allowed.
            dispatch_async([](vtkRenderWindow *rw, vtkUserData) {
                rw->Render();
            });
        } else {
            qDebug() << "SurfaceDataItem::onRegionSelected(): degenerate/trivial bounds,"
                        " skipping outline update";
        }
    }

    qDebug() << "SurfaceDataItem::onRegionSelected(): emitting editBoundsChanged";
    emit editBoundsChanged(xMin, xMax, yMin, yMax, zMin, zMax);
}
