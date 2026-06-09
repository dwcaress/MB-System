#include "SurfaceDataItem.h"
#include <QDebug>
#include <QMetaObject>

using namespace mb_system;

// ─────────────────────────────────────────────────────────────────────────────

SurfaceDataItem::SurfaceDataItem()
    : TopoDataItem()
{
    // Surface view defaults: polygon surface, elevation coloring
    surfaceRenderType_ = SurfaceRenderType::Polys;
    coloredScalar_     = ColoredScalar::Elevation;
}

// ─────────────────────────────────────────────────────────────────────────────
//  assemblePipeline override — apply surface-specific defaults
// ─────────────────────────────────────────────────────────────────────────────
void SurfaceDataItem::assemblePipeline(Pipeline *pipeline) {
    // Ensure surface defaults are set before the base class builds the pipeline
    surfaceRenderType_ = SurfaceRenderType::Polys;
    coloredScalar_     = ColoredScalar::Elevation;
    TopoDataItem::assemblePipeline(pipeline);
}

// ─────────────────────────────────────────────────────────────────────────────
//  onRegionSelected — extend selection to full Z range, emit signal
// ─────────────────────────────────────────────────────────────────────────────
void SurfaceDataItem::onRegionSelected(double worldBounds[6]) {

  qDebug() << "SurfaceDataItem::onRegionSelected(): dataset_=" << dataset_;

  if (!dataset_ || !dataset_->isLoaded()) return;
    const double xMin = worldBounds[0];
    const double xMax = worldBounds[1];
    const double yMin = worldBounds[2];
    const double yMax = worldBounds[3];

    // Extend Z to cover the full dataset elevation range, scaled by the
    // current vertical exaggeration so the clip volume matches what the user
    // sees.  A small outward margin avoids clipping surface-level points.
    const double zCenter = 0.5 * (dataset_->elevMin() + dataset_->elevMax());
    const double zHalf   = 0.5 * (dataset_->elevMax() - dataset_->elevMin())
                           * verticalExagg_;
    const double margin  = zHalf * 0.05;   // 5% margin

    const double zMin = zCenter - zHalf - margin;
    const double zMax = zCenter + zHalf + margin;

    qDebug() << "SurfaceDataItem::onRegionSelected():"
             << "x[" << xMin << "," << xMax << "]"
             << "y[" << yMin << "," << yMax << "]"
             << "z[" << zMin << "," << zMax << "]";

    qDebug() << "emit editBoundsChanged()";
    emit editBoundsChanged(xMin, xMax, yMin, yMax, zMin, zMax);
}
