#define QT_NO_DEBUG_OUTPUT

#include <unistd.h>
#include <climits>
#include <array>
#include <vector>
#include <thread>
#include <vtkErrorCode.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkLightCollection.h>
#include <vtkIdTypeArray.h>
#include "TopoDataItem.h"
#include "TopoColorMap.h"
#include "SharedConstants.h"


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
  

  // Instantiate interactor styles
  pickInteractorStyle_ = new PickInteractorStyle(this);
  lightingInteractorStyle_ = new LightingInteractorStyle(this);

  pointsSelectInteractorStyle_->setTopoDataItem(this);
  pointsSelectInteractorStyle_->setDrawingMode(MyRubberBandStyle::DrawingMode::Rectangle);

  // testStyle_->SetMinimumZ(-1000.);
  testStyle_->setTopoDataItem(this);
  testStyle_->setDrawingMode(DrawInteractorStyle::DrawingMode::Line);
}


QQuickVTKItem::vtkUserData TopoDataItem::initializeVTK(vtkRenderWindow
						       *renderWindow) {
  qDebug() << "initializeVTK()";

  renderWindow_ = renderWindow;
  
  // Create pipeline elements
  pipeline_ = new TopoDataItem::Pipeline();

  renderWindow->AddRenderer(pipeline_->renderer_);

  // Set interactor style
  pipeline_->interactorStyle_ = pickInteractorStyle_;
  
  // Assemble vtk pipeline
  assemblePipeline(pipeline_);
  setupLightSource();

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
  axesActor->GetTitleTextProperty(0)->SetFontSize(100);
  axesActor->GetLabelTextProperty(0)->SetColor(axisColor.GetData());

  axesActor->GetLabelTextProperty(0)->SetFontSize(30);

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

  axesActor->SetScreenSize(15.0);
  
  // Calling this sometimes results in no z-labels at all
  // axesActor->SetZLabelFormat("%.0f");
  
}



bool TopoDataItem::loadDatafile(QUrl fileUrl) {

  char *filename = strdup(fileUrl.toLocalFile().toLatin1().data());
  qDebug() << "loadGridfile " << filename;

  // Set name of grid file to access from pipeline
  setDataFilename(filename);

  pipeline_->firstRender_ = true;
  
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

  vtkLightCollection *lights = pipeline->renderer_->GetLights();

  // Clear all lights
  pipeline->renderer_->RemoveAllLights();
  
  lights = pipeline->renderer_->GetLights();
  
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

  // Associate cell and point id's with original polyData
  pipeline->idFilter_->SetInputData(pipeline->topoReader_->GetOutput());

  // Specify name by which to retrieve original id's
  pipeline->idFilter_->SetCellIdsArrayName(ORIGINAL_IDS);
  pipeline->idFilter_->SetPointIdsArrayName(ORIGINAL_IDS);
  pipeline->idFilter_->Update();


  // Read grid bounds
  double gridBounds[6];
  pipeline->topoReader_->gridBounds(&gridBounds[0], &gridBounds[1],
				    &gridBounds[2], &gridBounds[3],
				    &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];
  //////////////////////////////////////////


  //pipeline->elevFilter_->SetInputConnection(pipeline->topoReader_->/
  //					      GetOutputPort());
  pipeline->elevFilter_->SetInputConnection(pipeline->idFilter_->
					      GetOutputPort());
  
  pipeline->elevFilter_->SetLowPoint(0, 0, gridBounds[4]);
  pipeline->elevFilter_->SetHighPoint(0, 0, gridBounds[5]);
  // Preserve scalar values (keep minZ/maxZ range)
  pipeline->elevFilter_->SetScalarRange(gridBounds[4], gridBounds[5]);    

  double minVal = gridBounds[4];
  double maxVal = gridBounds[5];
  
  /// DEBUG ///
  printPolyDataOutput(pipeline->elevFilter_, "elevFilter");

  pipeline->polyData_ =
    vtkPolyData::SafeDownCast(pipeline->elevFilter_->GetOutput());

  pipeline->quality_->SetName(DATA_QUALITY_NAME);
  pipeline->quality_->SetNumberOfTuples(pipeline->polyData_->
					GetNumberOfPoints());
    
  // First assume all points are good
  for (int i = 0; i < pipeline->polyData_->GetNumberOfPoints(); i++) {
    pipeline->quality_->SetValue(i, GOOD_DATA);
  }

  // Associate quality array with original poly data
  pipeline->polyData_->GetPointData()->AddArray(pipeline->quality_);
  
  /// TEST TEST TEST
  // Try to get subsetted original point IDs
  vtkIdTypeArray *ids =
    vtkIdTypeArray::SafeDownCast(pipeline->polyData_->GetPointData()->
				 GetArray(ORIGINAL_IDS));
  if (ids) {
    qDebug() << "assemblePipeline(): FOUND id array";
  }
  else {
    qDebug() << "assemblePipeline(): COULD NOT FIND id array";    
  }
  
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
    
    qDebug() << "now mapper->GetArrayName(): " <<
      pipeline->surfaceMapper_->GetArrayName();

    qDebug() << "surfaceMapper: ";
    pipeline->surfaceMapper_->Print(std::cerr);
    
    // Bogus min/max gradient values
    minVal = 0.;
    maxVal = RAND_MAX;  // TEST TEST TEST
  }
  else {
    qDebug() << "connect surfaceMapper to elevFilter output port\n";
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

  //// DEBUG - count actors
  vtkActorCollection* actors = pipeline->renderer_->GetActors();
  std::cerr << "TOTAL actors: " << actors->GetNumberOfItems() << "\n";

  // Add any additional actors
  // Add extra actors
  for (vtkActor *actor: pipeline->addedActors_) {
    pipeline->renderer_->AddActor(actor);
  }
  
  pipeline->renderer_->SetBackground(pipeline->colors_->GetColor3d("White").
				     GetData());

  // Need to add the light again here
  qDebug() << "assemblePipeline(): GetLightIntensity=" <<
    pipeline->lightSource_->GetIntensity();

  pipeline->renderer_->AddLight(pipeline->lightSource_);

  lights = pipeline->renderer_->GetLights();
  
  if (showAxes_) {
    // Set up axes
    pipeline->axesActor_->SetCamera(pipeline->renderer_->GetActiveCamera());
    pipeline->axesActor_->SetScale(1., 1., verticalExagg_);
    setupAxes(pipeline->axesActor_,
	      pipeline->colors_,
	      pipeline->surfaceMapper_->GetBounds(),
	      gridBounds,
	      pipeline->topoReader_->xUnits(),
	      pipeline->topoReader_->yUnits(),
	      pipeline->topoReader_->zUnits(),
	      pipeline->topoReader_->geographicCRS());



    pipeline->renderer_->AddActor(pipeline->axesActor_);    
  }
    
  pipeline->surfaceActor_->SetScale(1., 1., verticalExagg_);  // NEW!
  pipeline->interactorStyle_->SetDefaultRenderer(pipeline->renderer_);

  pipeline->windowInteractor_->SetPicker(pipeline->areaPicker_);
  pipeline->windowInteractor_->SetInteractorStyle(pipeline->interactorStyle_);
  pipeline->windowInteractor_->SetRenderWindow(renderWindow_);
  
  if (pipeline->firstRender_) {
    pipeline->renderer_->ResetCamera();
  }
  pipeline->firstRender_ = false;

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

  qDebug() << "---- printPolyDataOutput() for " << outputName;
    
  algorithm->Update();

  algorithm->Print(std::cerr);

  vtkPolyData *polyData = algorithm->GetPolyDataOutput();
  vtkPoints *points = polyData->GetPoints();
  vtkCellArray *cells = polyData->GetPolys();

  double bounds[6];
  if (points) {
    qDebug() << "#points in " << outputName << " output: " <<
      points->GetNumberOfPoints();

    // Compute x, y, z bounds
    qDebug() << "Compute " << outputName << " bounds...";
    points->ComputeBounds();
    qDebug() << "Done";

    points->GetBounds(bounds);
    qDebug() << outputName << " bounds: " <<
      " xmin=" << bounds[0] << " xmax=" << bounds[1] <<
      " ymin=" << bounds[2] << " ymax=" << bounds[3] <<
      " zmin=" << bounds[4] << " zmax=" << bounds[5];
      
  }
  else {
    qDebug() << "no points in " << outputName << " output";    
  }
  
  if (cells) {
    qDebug() << "#cells in " << outputName << " output: " <<
      cells->GetNumberOfCells();
  }
  else {
    qDebug() << "no cells in " << outputName << " output";
  }

  vtkDataArray *dataArray = nullptr;
  vtkDataSet* dataSet = algorithm->GetOutput();
  vtkPointData *pointData = dataSet->GetPointData();
  
  if (pointData) {
    qDebug() << outputName << " pointData:";
    pointData->Print(std::cerr);
    dataArray = pointData->GetScalars();
    if (dataArray) {
      dataArray->Print(std::cerr);
    }
    else {
      qDebug() << outputName << " has no point data scalars";
    }
  }
  else {
    qDebug() << outputName << " has no pointData";
  }
  
  vtkCellData* cellData = dataSet->GetCellData();
  if (cellData) {
    qDebug() << outputName << " cellData:";
    cellData->Print(std::cerr);
    dataArray = cellData->GetScalars();
    if (dataArray) {
      dataArray->Print(std::cerr);
    }
    else {
      qDebug() << outputName << " has no cell data scalars";
    }
  }
  qDebug() << "---- printPolyDataOutput() done";
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


bool TopoDataItem::setMouseMode(QString mouseMode) {
  qDebug() << "setMouseMode(): " << mouseMode;

  if (mouseMode == MousePanAndZoom) {
    qDebug() << "setMouseMode(): set pickInteractorStyle_";
    pipeline_->interactorStyle_ = pickInteractorStyle_;    
  }
  else if (mouseMode == MouseLighting) {
    qDebug() << "setMouseMode(): set lightingInteractorStyle_";
    pipeline_->interactorStyle_ = lightingInteractorStyle_;
  }
  else if (mouseMode == MouseDataSelect) {
    qDebug() << "setMouseMode(): set pointsSelectInteractorStyle_";
    pipeline_->interactorStyle_ = pointsSelectInteractorStyle_;
  }
  else if (mouseMode == MouseTest) {
    qDebug() << "setMouseMode():: TEST!!";
    pipeline_->interactorStyle_ = testStyle_;
  }
  else {
    qDebug() << "setMouseMode(): " << mouseMode << " not yet implemented";
    return false;
  }

  reassemblePipeline();
  
  return true;
}


void TopoDataItem::setupLightSource() {

  qDebug() << "setupLightSource()";
  
  vtkLight *light = pipeline_->lightSource_;
  light->SetColor(1.0, 1.0, 1.0);
  
  // Position light above the midde of the topo surface
  double x = -0.03;
  double y = 0.24;
  double z = 0.50;
  
  light->SetPosition(x, y, z);
  
  light->SetFocalPoint(0.0, 0.0, 0.0);
  light->SetIntensity(1.0);
}


void TopoDataItem::setLight(float intensity, double x, double y, double z) {

  qDebug() << "setLight()";
  
  pipeline_->lightSource_->SetIntensity(intensity);

  
  pipeline_->lightSource_->SetPosition(x, y, z);
  
  // Render scene
  reassemblePipeline();
}


QVariantList TopoDataItem::getLightPosition() {

  double position[3];
  pipeline_->lightSource_->GetPosition(position);
  QVariantList result;
  
  for (int i = 0; i < 3; i++) {
    qDebug() << "getLightPosition(): " << i << ": " << position[i];
    result.append(position[i]);
  }

  return result;
}


double TopoDataItem::getLightIntensity() {
  return pipeline_->lightSource_->GetIntensity();
}


vtkPolyData *TopoDataItem::getPolyData() {
  return pipeline_->polyData_;
}
