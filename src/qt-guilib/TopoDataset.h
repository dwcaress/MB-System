#ifndef TOPODATASET_H
#define TOPODATASET_H

#include <QObject>
#include <QString>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkIntArray.h>
#include <vtkElevationFilter.h>
#include <vtkIdFilter.h>
#include "TopoDataReader.h"

/// Point-data array name for the data-quality flag
#define DATA_QUALITY_NAME "dataQuality"

/// Point-data array name for dataset's original point indices
/// (written by vtkIdFilter)
#define ORIGINAL_IDS      "originalIDs"

/// Quality value for a flagged-bad point
#define BAD_DATA  0

/// Quality value for an unflagged point
#define GOOD_DATA 1

namespace mb_system {

/**
   TopoDataset owns the shared topographic data that can be displayed by
   one or more TopoDataItem views.  It wraps TopoDataReader and the early
   pipeline stages (id-tagging, elevation scalar) that are about data rather
   than rendering, and exposes the resulting vtkPolyData to any number of
   view items without duplicating file I/O or the quality array.

   Separating data ownership from rendering fixes two problems that arise
   when multiple views display the same dataset:
     - A single quality_ array is mutated in place; all views re-render on
       the qualityChanged() signal without needing to know about each other.
     - reassemblePipeline() in a view no longer discards quality edits,
       because the quality array lives here, outside any view's pipeline.
*/
class TopoDataset : public QObject {
  Q_OBJECT

public:
  explicit TopoDataset(QObject *parent = nullptr);

  // ── Data loading ──────────────────────────────────────────────────────────

  /// Read the specified file.  Emits dataLoaded() on success,
  /// errorOccurred() on failure.  Returns false on failure.
  bool loadFile(const QString &path);

  // ── Shared data access ────────────────────────────────────────────────────

  /// The shared vtkPolyData produced by the elevation filter.
  /// TopoDataItem and subclasses (SurfaceDataItem, EditDataItem...) wire
  /// their pipeline tails to this.
  /// The polyData already carries the "Elevation" scalar and the
  /// DATA_QUALITY_NAME point-data array.
  vtkPolyData *polyData();

  // ── Quality editing ───────────────────────────────────────────────────────

  /// Set the quality flag for the point whose ORIGINAL_IDS value equals
  /// originalId.  Modifies the shared quality array in place, marks the
  /// polyData modified, and emits qualityChanged().  Thread-safe to call
  /// from the main Qt thread; view items dispatch their own re-renders in
  /// response to the signal.
  void setPointQuality(vtkIdType originalId, int quality);

  // ── Metadata accessors ────────────────────────────────────────────────────
  /// Minimum elevation in dataset
  double elevMin()  const { return elevMin_; }
  /// Maximum elevation in dataset
  double elevMax()  const { return elevMax_; }

  /// Fill caller's six doubles with xMin,xMax,yMin,yMax,zMin,zMax
  void gridBounds(double *xMin, double *xMax,
                  double *yMin, double *yMax,
                  double *zMin, double *zMax) const;

  /// Returns true if dataset has been loaded
  bool isLoaded() const { return dataLoaded_; }

  /// Direct access to the reader for units, CRS queries (e.g. in setupAxes).
  TopoDataReader *reader() { return reader_; }

  /// Quality array
  vtkIntArray *qualityArray() { return quality_; }
  
signals:
  /// Emitted after a successful loadFile().  Connected TopoDataItems can
  /// rebuild their rendering pipelines in response.
  void dataLoaded();

  /// Emitted after setPointQuality().  Connected TopoDataItems can dispatch a
  /// re-render in response.
  void qualityChanged();

  /// Emitted when loadFile() fails.  QML error dialog can connect to this.
  void errorOccurred(QString message);

private:
  // Early pipeline stages — reading, filtering data, not rendering
  vtkNew<TopoDataReader>     reader_;
  vtkNew<vtkIdFilter>        idFilter_;
  vtkNew<vtkElevationFilter> elevFilter_;

  /// Per-point quality flags; lives in polyData's PointData as DATA_QUALITY_NAME
  vtkNew<vtkIntArray>        quality_;

  // Cached after loadFile()
  double elevMin_        = 0.0;
  double elevMax_        = 0.0;
  double gridBounds_[6]  = {0, 0, 0, 0, 0, 0};
  bool   dataLoaded_     = false;
};

} // namespace mb_system
#endif // TOPODATASET_H
