#ifndef SURFACEDATAITEM_H
#define SURFACEDATAITEM_H

#include "TopoDataItem.h"

namespace mb_system {

/**
   SurfaceDataItem is the main-window view.  It renders the full dataset as a
   shaded polygon surface (default SurfaceRenderType::Polys) and lets the user
   draw a rubber-band rectangle to define the volume that will appear in the
   companion EditDataItem.

   Selection flow
   ──────────────
   1. User switches to MouseDataSelect mode and draws a rectangle.
   2. The PointsSelectInteractorStyle / MyRubberBandStyle calls
      onRegionSelected(worldBounds) on this item via the TopoDataItem* pointer
      it already holds (virtual dispatch ensures the override runs).
   3. onRegionSelected() extends the XY selection bounds to the full dataset Z
      range (respecting vertical exaggeration) and emits editBoundsChanged().
   4. QML connects editBoundsChanged() to EditDataItem::setEditBounds() on the
      companion window.
*/
class SurfaceDataItem : public TopoDataItem {
  Q_OBJECT

public:
  SurfaceDataItem();

  /// Called by the region-selection interactor style when the user completes a
  /// rubber-band selection.  worldBounds = {xMin,xMax,yMin,yMax,zMin,zMax}
  /// of the selection's world-space bounding box projected onto the surface.
  /// Extends the Z extents to cover the full dataset (±margin for vertical
  /// exaggeration) and emits editBoundsChanged().
  void onRegionSelected(double worldBounds[6]) override;

signals:
  /// Emitted after onRegionSelected().  QML connects this to
  /// EditDataItem::setEditBounds() on the edit window.
  void editBoundsChanged(double xMin, double xMax,
                         double yMin, double yMax,
                         double zMin, double zMax);

protected:
  void assemblePipeline(Pipeline *pipeline) override;
};

} // namespace mb_system
#endif // SURFACEDATAITEM_H
