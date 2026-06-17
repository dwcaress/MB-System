#include "TopoDataset.h"
#include "SwathData.h"
#include <unistd.h>
#include <vtkErrorCode.h>
#include <vtkPointData.h>
#include <QDebug>

using namespace mb_system;

// ─────────────────────────────────────────────────────────────────────────────

TopoDataset::TopoDataset(QObject *parent)
    : QObject(parent)
{
    quality_->SetName(DATA_QUALITY_NAME);
}

// ─────────────────────────────────────────────────────────────────────────────
//  loadFile — was TopoDataItem::loadDataPipeline()
// ─────────────────────────────────────────────────────────────────────────────

bool TopoDataset::loadFile(const QString &path) {
    QByteArray ba = path.toUtf8();
    const char *filename = ba.constData();

    if (access(filename, R_OK) == -1) {
        qWarning() << "TopoDataset::loadFile: cannot access" << path;
        emit errorOccurred(QString("Cannot access input file ") + path);
        dataLoaded_ = false;
        return false;
    }

    // Configure and run the reader
    TopoDataType dataType = TopoDataReader::getDataType(filename);
    reader_->setDataType(dataType);
    reader_->SetFileName(filename);
    reader_->Modified();
    reader_->UpdateInformation();
    reader_->Update();

    unsigned long errorCode = reader_->GetErrorCode();
    if (errorCode != 0) {
        qWarning() << "TopoDataset::loadFile: reader error" << errorCode;
        emit errorOccurred(QString("Cannot read file ") + path + "\n" +
                           vtkErrorCode::GetStringFromErrorCode(errorCode));
        dataLoaded_ = false;
        return false;
    }

    // Tag every point with its original index so point-flag operations can
    // map a picked render-pipeline ID back to the quality array slot.
    idFilter_->SetInputData(reader_->GetOutput());
    idFilter_->SetCellIdsArrayName(ORIGINAL_IDS);
    idFilter_->SetPointIdsArrayName(ORIGINAL_IDS);
    idFilter_->Update();

    // Cache grid bounds
    reader_->gridBounds(&gridBounds_[0], &gridBounds_[1],
                        &gridBounds_[2], &gridBounds_[3],
                        &gridBounds_[4], &gridBounds_[5]);
    elevMin_ = gridBounds_[4];
    elevMax_ = gridBounds_[5];

    // Compute "Elevation" point-data scalar (0–1 mapped to elevMin–elevMax).
    // This scalar is what all downstream color-mapping stages read.
    elevFilter_->SetInputConnection(idFilter_->GetOutputPort());
    elevFilter_->SetLowPoint (0, 0, elevMin_);
    elevFilter_->SetHighPoint(0, 0, elevMax_);
    elevFilter_->SetScalarRange(elevMin_, elevMax_);
    elevFilter_->Update();

    // Build the quality array — one entry per point, all GOOD_DATA initially.
    // AddArray() is idempotent: on a reload it replaces the old array in place.
    vtkPolyData *pd = polyData();
    quality_->SetNumberOfTuples(pd->GetNumberOfPoints());
    for (vtkIdType i = 0; i < pd->GetNumberOfPoints(); i++) {
        quality_->SetValue(i, GOOD_DATA);
    }
    pd->GetPointData()->AddArray(quality_);

    dataLoaded_ = true;
    emit dataLoaded();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────

vtkPolyData *TopoDataset::polyData() {
    return vtkPolyData::SafeDownCast(elevFilter_->GetOutput());
}

// ─────────────────────────────────────────────────────────────────────────────

void TopoDataset::setPointQuality(vtkIdType originalId, int quality) {
    if (!dataLoaded_) {
        qWarning() << "TopoDataset::setPointQuality: data not loaded";
        return;
    }
    quality_->SetValue(originalId, quality);

    // Mark the array and the polyData modified so that all downstream VTK
    // filters (in every connected TopoDataItem pipeline) re-execute on the
    // next render triggered by qualityChanged().
    quality_->Modified();
    polyData()->Modified();

    emit qualityChanged();
}

// ─────────────────────────────────────────────────────────────────────────────

vtkPoints *TopoDataset::navTrackPoints() {
    if (!dataLoaded_) {
        qDebug() << "navTrackPoints(): data not loaded";
        return nullptr;
    }
    if (reader_->getDataType() != TopoDataType::Swath) {
        qDebug() << "navTrackPoints(): not a swath file, dataType="
                 << reader_->getDataType();
        return nullptr;
    }
    TopoData *td = reader_->topoData();
    qDebug() << "navTrackPoints(): topoData()=" << (void*)td;
    SwathData *sd = dynamic_cast<SwathData *>(td);
    qDebug() << "navTrackPoints(): dynamic_cast<SwathData*>=" << (void*)sd;
    if (!sd) return nullptr;
    vtkPoints *pts = sd->navTrackPoints();
    qDebug() << "navTrackPoints(): points=" << (pts ? pts->GetNumberOfPoints() : -1);
    return pts;
}

// ─────────────────────────────────────────────────────────────────────────────

void TopoDataset::gridBounds(double *xMin, double *xMax,
                              double *yMin, double *yMax,
                              double *zMin, double *zMax) const {
    *xMin = gridBounds_[0];  *xMax = gridBounds_[1];
    *yMin = gridBounds_[2];  *yMax = gridBounds_[3];
    *zMin = gridBounds_[4];  *zMax = gridBounds_[5];
}
