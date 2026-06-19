#ifndef EDITDATAITEM_H
#define EDITDATAITEM_H

#include <vtkExtractPolyDataGeometry.h>
#include <vtkBox.h>
#include <vtkStaticPointLocator.h>
#include "TopoDataItem.h"

namespace mb_system {

// Defined in EditDataItem.cpp; installed as the interactor style for this view.
class EditPickInteractorStyle;

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
   EditPickInteractorStyle (a private helper class in EditDataItem.cpp) handles
   mouse events.  On a plain left-click (≤ 4 px drag) it calls
   setPickedPoint(worldCoords) on this item.  EditDataItem overrides that
   virtual method to:
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
  ~EditDataItem() override;

  /// Returns the current clip-filter output (extracted point subset).
  /// Called by EditPickInteractorStyle for screen-space point picking.
  vtkPolyData *clipOutput();

  /// Returns the polyData that the surface mapper is actually rendering.
  /// This is the output of the last filter in the pipeline before the mapper
  /// (which may differ from clipOutput() if the base class added intermediate
  /// filters such as a normals or glyph filter).  EditPickInteractorStyle
  /// should iterate THIS polyData for screen-space picking so that the point
  /// indices it finds match the indices the mapper uses to read scalar colours.
  /// Returns nullptr if the pipeline has not been assembled yet.
  vtkPolyData *mapperInputData();

  /// Returns the surface actor's composite model-to-world matrix (position ×
  /// origin × scale × rotation × -origin).  EditPickInteractorStyle must apply
  /// this before calling WorldToDisplay() so that vertical exaggeration and any
  /// other actor-level transforms are reflected in the projected screen position.
  /// Returns nullptr if the pipeline has not been assembled yet.
  vtkMatrix4x4 *surfaceActorMatrix();

  // ── Edit volume ────────────────────────────────────────────────────────────

  /// Set the world-space bounding box of the edit volume and re-render.
  /// Typically connected from SurfaceDataItem::editBoundsChanged() in QML.
  Q_INVOKABLE void setEditBounds(double xMin, double xMax,
                                  double yMin, double yMax,
                                  double zMin, double zMax);

  /// Reset the camera to fit the current clip-filter output and re-render.
  /// Called by the "Reset view" toolbar button in the edit window.
  Q_INVOKABLE void resetCamera();

  // ── Point flagging ─────────────────────────────────────────────────────────

  /// Set the quality value applied when the user clicks a point.
  /// Pass BAD_DATA (0) to flag as bad, GOOD_DATA (1) to unflag.  Default: BAD_DATA.
  Q_INVOKABLE void setFlagValue(int quality);

  /// Called by EditPickInteractorStyle with the local index (into the clip
  /// output) of the screen-space nearest point.  Looks up ORIGINAL_IDS and
  /// calls dataset_->setPointQuality() directly, avoiding any 3D-locator
  /// round-trip that can select a different point when vertical exaggeration
  /// displaces the visual position from the object-space position.
  void setPickedLocalId(vtkIdType localId);

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

  // ── Interactor style ───────────────────────────────────────────────────────
  /// Screen-space point-flagging interactor (defined in EditDataItem.cpp).
  /// Replaces the generic PickInteractorStyle for this view because:
  ///  (a) PickInteractorStyle's zero-tolerance drag detection misses clicks on
  ///      HiDPI displays; and
  ///  (b) vtkPointPicker is unreliable on point clouds with no mesh cells.
  EditPickInteractorStyle *editPickStyle_ = nullptr;

  // ── State ──────────────────────────────────────────────────────────────────
  double editBounds_[6]  = {0, 0, 0, 0, 0, 0};
  bool   boundsSet_      = false;
  bool   showBadPoints_  = true;
  int    flagValue_      = BAD_DATA;
};

} // namespace mb_system
#endif // EDITDATAITEM_H
