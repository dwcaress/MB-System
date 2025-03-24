#include <unistd.h>
#include <climits>
#include <array>
#include <vector>
#include <vtk/vtkProperty.h>
#include <vtk/vtkTextProperty.h>
#include <vtk/vtkErrorCode.h>
#include <vtk/vtkCellData.h>
#include <vtk/vtkPointData.h>
#include "TopoDataItem.h"
#include "TopoColorMap.h"

using namespace mb_system;

/// Define TopoDataItem::Pipeline::New() (factory method)
vtkStandardNewMacro(TopoDataItem::Pipeline);


TopoDataItem::TopoDataItem() {
  dataFilename_ = strdup("");
  verticalExagg_ = 1.;
  showAxes_ = false;
  scheme_ = TopoColorMap::Haxby;
  displayedSurface_ = DisplayedSurface::Elevation;
  pointPicked_ = false;
  forceRender_ = false;
}


QQuickVTKItem::vtkUserData TopoDataItem::initializeVTK(vtkRenderWindow *renderWindow) {
  qDebug() << "initializeVTK()";

  renderWindow_ = renderWindow;
  
  // Create pipeline elements
  pipeline_ = new TopoDataItem::Pipeline();

  renderWindow->AddRenderer(pipeline_->renderer_);
  
  // Assemble vtk pipeline
  assemblePipeline(pipeline_);

  return pipeline_;
}


void TopoDataItem::destroyingVTK(vtkRenderWindow
				 *renderWindow, vtkUserData userData) {
  qInfo() << "TopoDataItem::destroyingVTK() not implemented";
  return;
}


// vtkCubeAxesActor version
void TopoDataItem::setupAxes(vtkCubeAxesActor *axesActor,
                             vtkNamedColors *namedColors,                                                    double *surfaceBounds,
                             double *gridBounds,
                             const char *xUnits, const char *yUnits,
                             const char *zUnits,
			     bool geographicCRS) {

  qDebug() << "setupAxes(): " <<
    " xMin: " << surfaceBounds[0] << ", xMax: " << surfaceBounds[1] <<
    ", yMin: " << surfaceBounds[2] << ", yMax: " << surfaceBounds[3] <<
    ", zMin: " << surfaceBounds[4] << ", zMax: " << surfaceBounds[5];
  
  axesActor->SetBounds(surfaceBounds);
  
  axesActor->SetXAxisRange(gridBounds[0], gridBounds[1]);
  axesActor->SetYAxisRange(gridBounds[2], gridBounds[3]);  
  axesActor->SetZAxisRange(gridBounds[4], gridBounds[5]);

  vtkColor3d axisColor = namedColors->GetColor3d("Black");
  
  axesActor->GetTitleTextProperty(0)->SetColor(axisColor.GetData());
  axesActor->GetTitleTextProperty(0)->SetFontSize(48);
  axesActor->GetLabelTextProperty(0)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(1)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(1)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(2)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(2)->SetColor(axisColor.GetData());
  
  axesActor->GetXAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetYAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetZAxesLinesProperty()->SetColor(axisColor.GetData());    
  
  axesActor->DrawXGridlinesOn();
  axesActor->DrawYGridlinesOn();
  ///  axesActor->DrawZGridlinesOn();
  
  axesActor->SetXTitle(xUnits);
  axesActor->SetYTitle(yUnits);
  axesActor->SetZTitle(zUnits);

  axesActor->SetGridLineLocation(axesActor->VTK_GRID_LINES_FURTHEST);
  
  axesActor->XAxisMinorTickVisibilityOff();
  axesActor->YAxisMinorTickVisibilityOff();
  axesActor->ZAxisMinorTickVisibilityOff();

  axesActor->SetLabelScaling(0, 0, 0, 0);
  if (geographicCRS) {
    // Lat/lon in degrees
    axesActor->SetXLabelFormat("%.2f");
    axesActor->SetYLabelFormat("%.2f");
  }
  else {
    // Projected CRS, in meters
    axesActor->SetXLabelFormat("%.0f");
    axesActor->SetYLabelFormat("%.0f");    
  }

  // Calling this sometimes results in no z-labels at all
  // axesActor->SetZLabelFormat("%.0f");
  
}



bool TopoDataItem::loadDatafile(QUrl fileUrl) {

  char *filename = strdup(fileUrl.toLocalFile().toLatin1().data());
  qDebug() << "loadGridfile " << filename;

  // Set name of grid file to access from pipeline
  setDataFilename(filename);

  pipeline_->firstRender_ = true;
  
  // Set function to run in QT render thread
  /// dispatch_async(&(this->reassemblePipeline));  // HOW???

  reassemblePipeline();
  
  return true;
}


void TopoDataItem::reassemblePipeline() {
  // Dispatch lambda function to run in QT render thread 
  dispatch_async([this](vtkRenderWindow *renderWindow, vtkUserData userData) {
    auto *pipeline = TopoDataItem::Pipeline::SafeDownCast(userData);
    assemblePipeline(pipeline);
    return;
  });

  // Schedule update on the vtkRenderWindow
  scheduleRender();

  return;
}


void TopoDataItem::assemblePipeline(TopoDataItem::Pipeline *pipeline) {

  qDebug() << "assemblePipeline()";
  
  // Check that input file exists and is readable
  if (access(dataFilename_, R_OK) == -1) {
    qWarning() << "Can't access input file " << dataFilename_;
    return;
  }
  
  qDebug() << "set filename to " << dataFilename_;
  pipeline->topoReader_->SetFileName(dataFilename_);

  unsigned long errorCode;
  if ((errorCode = pipeline->topoReader_->GetErrorCode()) != 0) {
    qWarning() << "grid reader error during SetFileName(): "
	       << errorCode;

    qWarning() << dataFilename_ << ": " <<
      vtkErrorCode::GetStringFromErrorCode(errorCode);

    return;
  }

  // Clear mapper connections
  pipeline->surfaceMapper_->RemoveAllInputConnections(0);
    
  // Clear actor list
  pipeline->renderer_->RemoveAllViewProps();  

  // Determine grid type
  TopoDataType gridType =
    TopoDataReader::getDataType(dataFilename_);

  pipeline->topoReader_->setDataType(gridType);

  // Update TopoDataReader
  qDebug() << "call topoReader_->Update()";
  pipeline->topoReader_->Update();

  if ((errorCode = pipeline->topoReader_->GetErrorCode()) != 0) {
    qWarning() << "grid reader error during Update(): "
	       << errorCode;
    
    qWarning() << dataFilename_ << ": " <<
      vtkErrorCode::GetStringFromErrorCode(errorCode);
    
    return;
  }  

  /// DEBUG ////
  vtkPolyData *polyData = pipeline->topoReader_->GetOutput();
  vtkPoints *points = polyData->GetPoints();
  vtkCellArray *cells = polyData->GetPolys();
  if (points) {
    std::cerr << "topoReader output #points: " <<
      points->GetNumberOfPoints() << "\n";
  }
  else {
    std::cerr << "topoReader output has no points\n";
  }
  if (cells) {
    std::cerr << "topoReader output #cells: " << cells->GetNumberOfCells() <<
      "\n";
  }
  else {
    std::cerr << "topoReader output has no cells\n";
  }  
  ////////////////////
  
  // Read grid bounds
  double gridBounds[6];
  pipeline->topoReader_->gridBounds(&gridBounds[0], &gridBounds[1],
				    &gridBounds[2], &gridBounds[3],
				    &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];
  //////////////////////////////////////////


  pipeline->elevFilter_->SetInputConnection(pipeline->topoReader_->
					      GetOutputPort());
  
  pipeline->elevFilter_->SetLowPoint(0, 0, gridBounds[4]);
  pipeline->elevFilter_->SetHighPoint(0, 0, gridBounds[5]);
  // Preserve scalar values (keep minZ/maxZ range)
  pipeline->elevFilter_->SetScalarRange(gridBounds[4], gridBounds[5]);    

  double minVal = gridBounds[4];
  double maxVal = gridBounds[5];
  
  /// DEBUG ///
  printPolyDataOutput(pipeline->elevFilter_, "elevFilter");


  if (displayedSurface_ == TopoDataItem::DisplayedSurface::Gradient) {
    qDebug() << "set slopeFilter input to topoReader output port:";
    pipeline->slopeFilter_->SetInputConnection(pipeline->elevFilter_->
					       GetOutputPort());

    qDebug() << "connected slopeFilter input to elevFilter output port";
    
    printPolyDataOutput(pipeline->slopeFilter_, "slopeFilter");
    
    pipeline->surfaceMapper_->SetInputConnection(pipeline->slopeFilter_->
						 GetOutputPort());
    
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray("Slopes");
    
    std::cerr << "now mapper->GetArrayName(): " <<
      pipeline->surfaceMapper_->GetArrayName() << "\n";

    std::cerr << "surfaceMapper: ";
    pipeline->surfaceMapper_->Print(std::cerr);
    
    // Bogus min/max gradient values
    minVal = 0.;
    maxVal = RAND_MAX;  // TEST TEST TEST
  }
  else {
    std::cerr << "connect surfaceMapper to elevFilter output port\n";
    pipeline->surfaceMapper_->SetInputConnection(pipeline->elevFilter_->
						 GetOutputPort());
  }

  // Make lookup table
  TopoColorMap::makeLUT(scheme_,
			pipeline->elevLookupTable_);
    
  // Use scalar data to color objects
  pipeline->surfaceMapper_->ScalarVisibilityOn();
  // Scalar values range from min to max z (depth)
  pipeline->surfaceMapper_->SetScalarRange(minVal, maxVal);

  pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
  
  // Assign surfaceMapper to actor
  pipeline->surfaceActor_->SetMapper(pipeline->surfaceMapper_);
    
  // Add actor to renderer
  pipeline->renderer_->AddActor(pipeline->surfaceActor_);

  pipeline->renderer_->SetBackground(pipeline->colors_->GetColor3d("White").
				     GetData());
  
  if (showAxes_) {
    // Set up axes
    setupAxes(pipeline->axesActor_,
	      pipeline->colors_,
	      pipeline->surfaceMapper_->GetBounds(),
	      gridBounds,
	      pipeline->topoReader_->xUnits(),
	      pipeline->topoReader_->yUnits(),
	      pipeline->topoReader_->zUnits(),
	      pipeline->topoReader_->geographicCRS());


    pipeline->axesActor_->SetCamera(pipeline->renderer_->GetActiveCamera());

    pipeline->renderer_->AddActor(pipeline->axesActor_);    
    pipeline->axesActor_->SetScale(1., 1., verticalExagg_);
  }
    
  pipeline->surfaceActor_->SetScale(1., 1., verticalExagg_);  // NEW!

  pipeline->interactorStyle_->initialize(this, pipeline->windowInteractor_);
  pipeline->interactorStyle_->SetDefaultRenderer(pipeline->renderer_);
  pipeline->interactorStyle_->polyData_ =
    pipeline->topoReader_->GetOutput();
  

  pipeline->windowInteractor_->SetInteractorStyle(pipeline->interactorStyle_);
  pipeline->windowInteractor_->SetRenderWindow(renderWindow_);
  
  if (pipeline->firstRender_) {
    pipeline->renderer_->ResetCamera();
  }
  pipeline->firstRender_ = false;


  /// TEST : get vertical profile across the map
  
}



bool TopoDataItem::setColormap(QString name) {

  QByteArray ba = name.toLocal8Bit();
  char *cname = ba.data();

  TopoColorMap::Scheme scheme = TopoColorMap::schemeFromName(cname);
  if (scheme == TopoColorMap::Unknown) {
    return false;
  }
  scheme_ = scheme;

  reassemblePipeline();

  return true;
}


void TopoDataItem::showAxes(bool plotAxes) {
  qDebug() << "showAxes(): " << plotAxes;
  showAxes_ = plotAxes;

  reassemblePipeline();

  return;
}


void TopoDataItem::printPolyDataOutput(vtkDataSetAlgorithm *algorithm,
				 const char *outputName) {

  std::cerr << "---- printPolyDataOutput() for " << outputName << "\n";
    
  algorithm->Update();

  algorithm->Print(std::cerr);

  vtkPolyData *polyData = algorithm->GetPolyDataOutput();
  vtkPoints *points = polyData->GetPoints();
  vtkCellArray *cells = polyData->GetPolys();

  double bounds[6];
  if (points) {
    std::cerr << "#points in " << outputName << " output: " <<
      points->GetNumberOfPoints() << "\n";

    // Compute x, y, z bounds
    std::cerr << "Compute " << outputName << " bounds...";
    points->ComputeBounds();
    std::cerr << "Done\n";

    points->GetBounds(bounds);
    std::cerr << outputName << " bounds: " <<
      " xmin=" << bounds[0] << " xmax=" << bounds[1] <<
      " ymin=" << bounds[2] << " ymax=" << bounds[3] <<
      " zmin=" << bounds[4] << " zmax=" << bounds[5] << "\n";
      
  }
  else {
    std::cerr << "no points in " << outputName << " output\n";    
  }
  
  if (cells) {
    std::cerr << "#cells in " << outputName << " output: " <<
      cells->GetNumberOfCells() << "\n";
  }
  else {
    std::cerr << "no cells in " << outputName << " output\n";
  }

  vtkDataArray *dataArray = nullptr;
  vtkDataSet* dataSet = algorithm->GetOutput();
  vtkPointData *pointData = dataSet->GetPointData();
  
  if (pointData) {
    std::cerr << outputName << " pointData:\n";
    pointData->Print(std::cerr);
    dataArray = pointData->GetScalars();
    if (dataArray) {
      dataArray->Print(std::cerr);
    }
    else {
      std::cerr << outputName << " has no point data scalars\n";
    }
  }
  else {
    std::cerr << outputName << " has no pointData\n";
  }
  
  vtkCellData* cellData = dataSet->GetCellData();
  if (cellData) {
    std::cerr << outputName << " cellData:\n";
    cellData->Print(std::cerr);
    dataArray = cellData->GetScalars();
    if (dataArray) {
      dataArray->Print(std::cerr);
    }
    else {
      std::cerr << outputName << " has no cell data scalars\n";
    }
  }
  std::cerr << "---- printPolyDataOutput() done\n";
}

void TopoDataItem::setPickedPoint(double *worldCoords) {

  pointPicked_ = true;

  pickedCoords_[0] = worldCoords[0];
  pickedCoords_[1] = worldCoords[1];
  pickedCoords_[2] = worldCoords[2];

  /// Force render on next update
  forceRender_ = true;
}


TopoDataReader *TopoDataItem::getDataReader() {
  return pipeline_->topoReader_;
}


QList<QVector2D> TopoDataItem::getElevProfile(int row1, int col1,
					      int row2,  int col2,
					      int nPieces) {


  auto *profile = new std::vector<std::array<double, 2>>;

  qDebug() << "TopoDataItem::getElevProfile() TEST TEST TEST row/col values";
  row1 = col1 = 0;

  row2 = pipeline_->topoReader_->topoData()->nRows();
  col2 = pipeline_->topoReader_->topoData()->nColumns();
  
  bool ok = pipeline_->topoReader_->topoData()->getElevProfile(row1, col1,
							       row2 - 1,
							       col2 - 1,
							       nPieces,
							       profile);
  QList<QVector2D> qProfile;
  
  if (!ok) {
    // Return 0-length profile
    return qProfile;
  }

  // Print profile
  for (int i = 0; i < profile->size(); i++) {
    std::array<double, 2> point = profile->at(i);
    qDebug() << "distance: " << point[0] << ", z: " << point[1];
  }

  // Transfer std::vector profile data to QList of QVector2D objects
  QVector2D qPoint;  
  for (int i = 0; i < profile->size(); i++) {
    std::array<double, 2> point = profile->at(i);
    qPoint.setX((float )point[0]);
    qPoint.setY((float )point[1]);
    qProfile.append(qPoint);
  }

  // print out list values
  qDebug() << "getElevProfile() output:";
  for (int i = 0; i < qProfile.size(); i++) {
    const QVector2D p = qProfile.at(i);
    qDebug() << "p.x(): " << p.x() << ", p.y(): " << p.y();
  }

  delete profile;
  
  return qProfile;
}


QList<QVector2D> TopoDataItem::runTest2(void) {
  // Get elevation profile
  int nRows = pipeline_->topoReader_->topoData()->nRows();
  int nCols = pipeline_->topoReader_->topoData()->nColumns();

  qDebug() << "nRows: " << nRows << ", nCols: " << nCols;

  // Vector holds elevation profile
  auto *profile = new std::vector<std::array<double, 2>>;

  qDebug() << "runTest(): get elevation profile";
  int nPieces = 10;
  bool ok = pipeline_->topoReader_->topoData()->getElevProfile(0, 0,
							       nRows-1,
							       nCols-1,
							       nPieces,
							       profile);
  QList<QVector2D> qProfile;
  
  if (!ok) {
    return qProfile;
  }

  // Print profile
  for (int i = 0; i < profile->size(); i++) {
    std::array<double, 2> point = profile->at(i);
    qDebug() << "distance: " << point[0] << ", z: " << point[1];
  }

  // Transfer std::vector profile data to QList of QVector2D objects
  QVector2D qPoint;  
  for (int i = 0; i < profile->size(); i++) {
    std::array<double, 2> point = profile->at(i);
    qPoint.setX((float )point[0]);
    qPoint.setY((float )point[1]);
    qProfile.append(qPoint);
  }

  // print out list values
  qDebug() << "getZProfile() output:";
  for (int i = 0; i < qProfile.size(); i++) {
    const QVector2D p = qProfile.at(i);
    qDebug() << "p.x(): " << p.x() << ", p.y(): " << p.y();
  }

  delete profile;
  
  return qProfile;
}

