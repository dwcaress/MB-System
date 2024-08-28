#include <unistd.h>
#include <vtk/vtkProperty.h>
#include <vtk/vtkTextProperty.h>

#include "TopoGridItem.h"
#include "TopoColorMap.h"

using namespace mb_system;

// Define TopoGridItem::Pipeline::New()
vtkStandardNewMacro(TopoGridItem::Pipeline);


TopoGridItem::TopoGridItem() {
  gridFilename_ = strdup("");
  verticalExagg_ = 1.;
  plotAxes_ = true;
}


QQuickVTKItem::vtkUserData TopoGridItem::initializeVTK(vtkRenderWindow *renderWindow) {
  qDebug() << "initializeVTK()";

  renderWindow_ = renderWindow;
  
  // Create pipeline elements
  vtkNew<TopoGridItem::Pipeline> pipeline_;

  renderWindow->AddRenderer(pipeline_->renderer_);
  
  // Assemble vtk pipeline
  assemblePipeline(pipeline_);

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

  return pipeline_;
}


void TopoGridItem::destroyingVTK(vtkRenderWindow
				 *renderWindow, vtkUserData userData) {
  qInfo() << "TopoGridItem::destroyingVTK() not implemented";
  return;
}


// vtkCubeAxesActor version
void TopoGridItem::setupAxes(vtkCubeAxesActor *axesActor,
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


bool TopoGridItem::setColorMapScheme(const char *colorMapName) {
  qDebug() << "setColormap() " << colorMapName;
  
  // Check for valid colorMap name

  TopoColorMap::Scheme scheme = TopoColorMap::schemeFromName(colorMapName);
  if (scheme == TopoColorMap::Scheme::Unknown) {
    return false;
  }

  scheme_ = scheme;
  
  return true;

}

bool TopoGridItem::loadGridfile(QUrl fileUrl) {

  char *filename = strdup(fileUrl.toLocalFile().toLatin1().data());
  qDebug() << "loadGridfile " << filename;
  setGridFilename(filename);

  // Dispatch pipeline commands to run in render thread
  dispatch_async([this](vtkRenderWindow *renderWindow, vtkUserData userData) {

    auto *pipeline = TopoGridItem::Pipeline::SafeDownCast(userData);
    assemblePipeline(pipeline);
    return;

  });
  
  scheduleRender();
  return true;
}




void TopoGridItem::assemblePipeline(TopoGridItem::Pipeline *pipeline) {
    // Set TopoGridReader file name
    qDebug() << "set filename to " << gridFilename_;
    pipeline->gridReader_->SetFileName(gridFilename_);

    if (pipeline->gridReader_->GetErrorCode() != 0) {
      qWarning() << "grid reader error during SetFileNae(): "
		 << pipeline->gridReader_->GetErrorCode();
      return;
    }  

    // Determine grid type
    TopoGridType gridType =
      TopoGridReader::getGridType(gridFilename_);

    pipeline->gridReader_->setGridType(gridType);

    // Update TopoGridReader
    qDebug() << "call gridReader_->Update()";
    pipeline->gridReader_->Update();

    if (pipeline->gridReader_->GetErrorCode() != 0) {
      qWarning() << "grid reader error during Update(): "
		 << pipeline->gridReader_->GetErrorCode();
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

    pipeline->renderer_->SetBackground(pipeline->colors_->GetColor3d("White").
				       GetData());
  

    // Set up axes
    setupAxes(pipeline->axesActor_,
	      pipeline->colors_,
	      pipeline->surfaceMapper_->GetBounds(),
	      gridBounds,
	      pipeline->gridReader_->xUnits(),
	      pipeline->gridReader_->yUnits(),
	      pipeline->gridReader_->zUnits(),
	      pipeline->gridReader_->geographicCRS());


    pipeline->axesActor_->SetCamera(pipeline->renderer_->GetActiveCamera());

    pipeline->renderer_->AddActor(pipeline->axesActor_);    


    /* ***
    ///
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
  ////
  *** */
    
    pipeline->firstRender_ = true;  /// TEST
    
    if (pipeline->firstRender_) {
      pipeline->renderer_->ResetCamera();
    }
    pipeline->firstRender_ = false;
}
