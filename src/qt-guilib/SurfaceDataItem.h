#ifndef SURFACEDATAITEM_H
#define SURFACEDATAITEM_H

#include "TopoDataItem.h"
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

namespace mb_system {

/**
   SurfaceDataItem — main-window view of the full bathymetric dataset.

   Rendering: polygon surface (SurfaceRenderType::Polys), elevation coloring.

   Bad-point hiding
   ────────────────
   The surface view maintains its own render-only copy of the dataset geometry
   (surfacePolyData_) that contains only cells whose vertices are all GOOD.
   A companion vtkTrivialProducer (qualitySource_) wraps that copy and is
   returned by dataOutputPort() so the mapper, normals filter, contour filter,
   etc. see the filtered geometry.

   connectDataset() is overridden ONLY to initialise surfacePolyData_ and
   qualitySource_; it does NOT change pipeline_->source_ or pipeline_->polyData_,
   so the rubber-band area picker (PointsSelectInteractorStyle) continues to
   operate on the full shared dataset and can select bad points for editing.

   When EditDataItem marks a point Bad/Good, TopoDataset emits qualityChanged().
   onQualityChanged() dispatches rebuildSurfacePolyData() to the render thread,
   which rebuilds the cell array to exclude bad-vertex cells, marks the copy
   Modified, and re-renders.

   Selection flow
   ──────────────
   1. User alt-drags a rubber band → PointsSelectInteractorStyle picks from
      pipeline_->polyData_ (= dataset_->polyData(), unchanged).
   2. onRegionSelected(worldBounds) emits editBoundsChanged().
   3. QML shows the EditDataItem window and calls setEditBounds().
*/
class SurfaceDataItem : public TopoDataItem {
  Q_OBJECT

public:
  SurfaceDataItem();

  void onRegionSelected(double worldBounds[6]) override;

signals:
  void editBoundsChanged(double xMin, double xMax,
                         double yMin, double yMax,
                         double zMin, double zMax);

protected:
  void assemblePipeline(Pipeline *pipeline) override;

  /// Extends base connectDataset(): after the base wires pipeline_->source_
  /// and pipeline_->polyData_ to dataset_->polyData() (unchanged), initialises
  /// surfacePolyData_ with good-only geometry and wires qualitySource_ to it.
  bool connectDataset(Pipeline *pipeline) override;

  /// Returns qualitySource_->GetOutputPort() so all downstream rendering
  /// stages (mapper, normals, contours) consume the good-only copy.
  /// pipeline_->polyData_ is never touched — rubber-band works against the
  /// full dataset as before.
  vtkAlgorithmOutput *dataOutputPort(Pipeline *pipeline) override;

protected slots:
  /// Rebuilds surfacePolyData_ (good cells only) on the render thread and
  /// triggers a re-render of the surface view.
  void onQualityChanged() override;

private:
  /// Called on the render thread.  Shallow-copies dataset_->polyData() into
  /// surfacePolyData_, then replaces the polys array with a new cell array
  /// that omits any cell containing at least one BAD_DATA vertex.
  void rebuildSurfacePolyData();

  /// Render-only geometry: good cells only.  Points array is shared with
  /// dataset_->polyData() via ShallowCopy; the polys array is rebuilt on
  /// every quality change.
  vtkNew<vtkPolyData>        surfacePolyData_;

  /// Wraps surfacePolyData_ as a VTK pipeline source so downstream filters
  /// can use SetInputConnection().
  vtkNew<vtkTrivialProducer> qualitySource_;

  /// Flat 4-edge rectangle showing the most recent rubber-band selection.
  /// Corners are all at zMax (topmost world Z in the selection) so it lies
  /// on the surface rather than forming a 3-D box.  Invisible until the
  /// first selection; updated by onRegionSelected().
  vtkNew<vtkPolyData>       selectionRectPolyData_;
  vtkNew<vtkPolyDataMapper> selectionOutlineMapper_;
  vtkNew<vtkActor>          selectionOutlineActor_;
};

} // namespace mb_system
#endif // SURFACEDATAITEM_H
