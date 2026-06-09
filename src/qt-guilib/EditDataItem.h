#ifndef EDITDATAITEM_H
#define EDITDATAITEM_H

#include <vtkExtractPolyDataGeometry.h>
#include <vtkBox.h>
#include <vtkStaticPointLocator.h>
#include "TopoDataItem.h"

namespace mb_system {

/**
   EditDataItem is the edit-window view.  It renders a spatial subset of the
   shared TopoDataset as a point cloud, coloured by data quality, and lets the
   user pick individual points to flag them as bad or good.

   Pipeline additions
   ──────────────────
   Between the shared source_ and the base-class rendering stages, EditDataItem
   interposes a vtkExtractPolyDataGeometry / vtkBox clip filter.  The override
   of dataOutputPort() makes all base-class apply*() methods automatically read
   from the clip filter's output instead of the raw source.

   Updating the clip volume (setEditBounds) only re-runs the extraction filter
   and issues a Render — no full pipeline rebuild is needed.

   Point flagging
   ──────────────
   The existing PickInteractorStyle calls setPickedPoint(worldCoords) on the
   item.  EditDataItem overrides that method to:
     1. Find the closest point in the *extracted* polyData using a static KD-tree
        locator rebuilt after each setEditBounds() call.
     2. Read the ORIGINAL_IDS array to get the stable index into the shared
        quality array.
     3. Call dataset_->setPointQuality(originalId, flagValue_), which modifies
        the quality array in place and emits qualityChanged() — causing both
        the edit and main windows to re-render.

   Bad-point visibility
   ────────────────────
   setShowBadPoints(false) makes the bad-data entry in the quality LUT
   transparent (alpha = 0) so flagged points disappear visually without any
   pipeline rebuild.  setShowBadPoints(true) restores full opacity.
*/
class EditDataItem : public TopoDataItem {
  Q_OBJECT

public:
  EditDataItem();

  // ── Edit volume ────────────────────────────────────────────────────────────

  /// Set the world-space bounding box of the edit volume and re-render.
  /// Typically connected from SurfaceDataItem::editBoundsChanged() in QML.
  Q_INVOKABLE void setEditBounds(double xMin, double xMax,
                                  double yMin, double yMax,
                                  double zMin, double zMax);

  // ── Point flagging ─────────────────────────────────────────────────────────

  /// Set the quality value applied when the user clicks a point.
  /// Pass BAD_DATA (0) to flag as bad, GOOD_DATA (1) to unflag.  Default: BAD_DATA.
  Q_INVOKABLE void setFlagValue(int quality);

  /// Override: maps world coordinates to the nearest point in the extracted
  /// subset, looks up its ORIGINAL_IDS value, and calls
  /// dataset_->setPointQuality(originalId, flagValue_).
  void setPickedPoint(double *worldCoords) override;

  // ── Display ────────────────────────────────────────────────────────────────

  /// Show or hide points flagged as bad.
  ///   true  (default): bad points shown in a distinct colour (red LUT entry)
  ///   false           : bad-data LUT entry alpha set to 0 (transparent)
  /// Takes effect immediately without a pipeline rebuild.
  Q_INVOKABLE void setShowBadPoints(bool show);

protected:
  // ── TopoDataItem overrides ─────────────────────────────────────────────────

  /// Returns clipFilter_->GetOutputPort() so all base-class apply*() methods
  /// operate on the spatially clipped subset.
  vtkAlgorithmOutput *dataOutputPort(Pipeline *pipeline) override;

  /// Extends the base implementation to wire clipFilter_ between source_ and
  /// the rest of the pipeline, and to rebuild the point locator.
  bool connectDataset(Pipeline *pipeline) override;

  /// Extends the base implementation to apply edit-window defaults:
  ///   - SurfaceRenderType::PointCloud
  ///   - ColoredScalar::DataQuality
  /// and to build the two-entry quality LUT.
  void assemblePipeline(Pipeline *pipeline) override;

protected slots:
  /// Override: re-syncs the quality array in the clip output (workaround for
  /// vtkExtractPolyDataGeometry not copying vtkIntArray values correctly),
  /// then dispatches a re-render.
  void onQualityChanged() override;

private:
  /// Build (or rebuild) the two-entry lookup table used for DataQuality
  /// coloring in this view.  Bad-data alpha is controlled by showBadPoints_.
  void buildQualityLut();

  /// Rebuild the static point locator on the *extracted* polyData so that
  /// setPickedPoint() can quickly find the nearest point by world coordinate.
  /// Must be called (on the render thread) after every setEditBounds().
  void rebuildLocator();

  /// Workaround for vtkExtractPolyDataGeometry silently zeroing vtkIntArray
  /// values in its output.  Manually copies quality values from the shared
  /// dataset quality array into the clip output using ORIGINAL_IDS for mapping.
  /// Must be called (on the render thread) after every clipFilter_->Update().
  void syncQualityArray();

  // ── Clip filter (spatial subset) ──────────────────────────────────────────
  vtkNew<vtkExtractPolyDataGeometry> clipFilter_;
  vtkNew<vtkBox>                     clipBox_;

  // ── Point locator for pick → point-ID mapping ─────────────────────────────
  vtkNew<vtkStaticPointLocator>      pointLocator_;

  // ── Two-entry quality lookup table ────────────────────────────────────────
  /// Separate from the base-class elevLookupTable_; used when coloredScalar_
  /// == DataQuality.  Entries: index 0 = BAD_DATA (red / transparent),
  ///                            index 1 = GOOD_DATA (blue-green).
  vtkNew<vtkLookupTable>             qualityLut_;

  // ── State ──────────────────────────────────────────────────────────────────
  double editBounds_[6]  = {0, 0, 0, 0, 0, 0};
  bool   boundsSet_      = false;
  bool   showBadPoints_  = true;
  int    flagValue_      = BAD_DATA;
};

} // namespace mb_system
#endif // EDITDATAITEM_H
