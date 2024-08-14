#include "TopoGridItem.h"
#include "TopoColorMap.h"

// Define TopoGridItem::Pipeline::New()
vtkStandardNewMacro(TopoGridItem::Pipeline);

using namespace mb_system;

TopoGridItem::TopoGridItem() {
  gridFilename_ = strdup("");
  verticalExagg_ = 1.;
}


QQuickVTKItem::vtkUserData TopoGridItem::initializeVTK(vtkRenderWindow *renderWindow) {
  qDebug() << "initializeVTK()";

  // Create pipeline elements
  vtkNew<TopoGridItem::Pipeline> pipeline;

  renderWindow->AddRenderer(pipeline->renderer_);
  
  // Assemble vtk pipeline
  pipeline->gridReader_->SetFileName(gridFilename_);

  if (pipeline->gridReader_->GetErrorCode() != 0) {
    qWarning() << "grid reader error during SetFileNae(): "
	       << pipeline->gridReader_->GetErrorCode();
    return pipeline;
  }

  // Determine grid type
  TopoGridType gridType =
    TopoGridReader::getGridType(gridFilename_);

  pipeline->gridReader_->setGridType(gridType);
  
  pipeline->gridReader_->Update();
  if (pipeline->gridReader_->GetErrorCode() != 0) {
    qWarning() << "grid reader error during Update(): "
	       << pipeline->gridReader_->GetErrorCode();
    return pipeline;
  }

  
  // Read grid bounds
  double gridBounds[6];
  pipeline->gridReader_->gridBounds(&gridBounds[0], &gridBounds[1],
				    &gridBounds[2], &gridBounds[3],
				    &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];


  double *dBounds = pipeline->gridReader_->GetOutput()->GetBounds();
  
  qDebug() << "GetBounds() - xMin: " << dBounds[0] << ", xMax: " <<
    dBounds[1] <<
    "yMin: " << dBounds[2] << ", yMax: " << dBounds[3] <<
    "zMin: " << dBounds[4] << ", zMax: " << dBounds[5];

  pipeline->elevColorizer_->SetInputConnection(pipeline->gridReader_
					       ->GetOutputPort());
  
  pipeline->elevColorizer_->SetLowPoint(0, 0, gridBounds[4]);
  pipeline->elevColorizer_->SetHighPoint(0, 0, gridBounds[5]);

  /// Scale z axis based on vertical exaggeration
  float zScale = verticalExagg_ * pipeline->gridReader_->zScaleLatLon();

  pipeline->transform_->Scale(1., 1., zScale);
  pipeline->transformFilter_->SetTransform(pipeline->transform_);
  pipeline->transformFilter_->SetInputConnection(pipeline->elevColorizer_->
						 GetOutputPort());

  pipeline->surfaceMapper_->SetInputConnection(pipeline->transformFilter_->
					       GetOutputPort());

  pipeline->elevColorizer_->SetScalarRange(dBounds[4], dBounds[5]);
  TopoColorMap::makeLUT(TopoColorMap::Haxby,
                        pipeline->elevLookupTable_);
    
  pipeline->surfaceMapper_->SetScalarRange(dBounds[4], dBounds[5]);
  pipeline->surfaceMapper_->ScalarVisibilityOn();
  pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
  
  // Assign surfaceMapper to actor
  qDebug() << "assign surfaceMapper to actor";
  pipeline->surfaceActor_->SetMapper(pipeline->surfaceMapper_);
  
  // Add actor to renderer
  pipeline->renderer_->AddActor(pipeline->surfaceActor_);

  /* ***
  if (displayProperties_->siteFile()) {
    /// TEST TEST TEST SITE READER
    vtkNew<vtkParticleReader> siteReader;
    // siteReader->SetFileName("test-site.ste");
    qDebug() << "open particle reader source file " <<
      displayProperties_->siteFile();
    
    siteReader->SetFileName(displayProperties_->siteFile());
    siteReader->Update();

    vtkNew<vtkPolyDataMapper> siteMapper;
    siteMapper->SetInputConnection(siteReader->GetOutputPort());
    vtkNew<vtkActor> siteActor;
    siteActor->SetMapper(siteMapper);
    siteActor->GetProperty()->SetPointSize(25);
    renderer_->AddActor(siteActor);
  }
  *** */

  /* ***
  qDebug() << "ready to check pointPicked_";
  qDebug() << "pointPicked_: " << pointPicked_;
  if (pointPicked_) {
    std::cerr << "add picked point to scene" << std::endl;

    int *size = renderWindow_->GetSize();
    std::cerr << "window w: " << size[0] << "  h: " << size[1] << "\n";

    vtkNew<vtkCoordinate> coords;
    coords->SetCoordinateSystemToDisplay();
    coords->SetValue(size[0], size[1], 0.);
    double *worldSize = coords->GetComputedWorldValue(renderer_);
    std::cerr << "world x: " << worldSize[0] << "  world y: " <<
      worldSize[1] << "  world z: " << worldSize[2] << "\n";
    
    vtkSmartPointer<vtkSphereSource>pickedPoint =
      vtkSmartPointer<vtkSphereSource>::New();

    double radius = worldSize[0] / 30000;
    pickedPoint->SetRadius(radius);
    pickedPoint->SetCenter(pickedCoords_[0], pickedCoords_[1],
			   pickedCoords_[2]);
    
    vtkSmartPointer<vtkPolyDataMapper> pickedPointMapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    
    pickedPointMapper->SetInputConnection(pickedPoint->GetOutputPort());

    vtkSmartPointer<vtkActor> pickedPointActor =
      vtkSmartPointer<vtkActor>::New();
    
    pickedPointActor->SetMapper(pickedPointMapper);

    renderer_->AddActor(pickedPointActor);

  }
  *** */

  return pipeline;
}


void TopoGridItem::destroyingVTK(vtkRenderWindow
				 *renderWindow, vtkUserData userData) {
  qInfo() << "TopoGridItem::destroyingVTK() not implemented";
  return;
}


  void dispatch_async(std::function<void(vtkRenderWindow *renderWindow,
					 QQuickVTKItem::vtkUserData data)>f) {
    qInfo() << "TopoGridItem::dispatch_async() not implemented";
    return;
  }
