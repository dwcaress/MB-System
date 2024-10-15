#include <unistd.h>
#include <vtk/vtkProperty.h>
#include <vtk/vtkTextProperty.h>
#include <vtk/vtkErrorCode.h>

#include "TopoGridItem.h"
#include "TopoColorMap.h"

using namespace mb_system;

/// Define TopoGridItem::Pipeline::New() (factory method)
vtkStandardNewMacro(TopoGridItem::Pipeline);


TopoGridItem::TopoGridItem() {
  gridFilename_ = strdup("");
  verticalExagg_ = 1.;
  plotAxes_ = false;
  scheme_ = TopoColorMap::Haxby;
}


QQuickVTKItem::vtkUserData TopoGridItem::initializeVTK(vtkRenderWindow *renderWindow) {
  qDebug() << "initializeVTK()";

  renderWindow_ = renderWindow;
  
  // Create pipeline elements
  vtkNew<TopoGridItem::Pipeline> pipeline_;

  renderWindow->AddRenderer(pipeline_->renderer_);
  
  // Assemble vtk pipeline
  assemblePipeline(pipeline_);

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



bool TopoGridItem::loadGridfile(QUrl fileUrl) {

  char *filename = strdup(fileUrl.toLocalFile().toLatin1().data());
  qDebug() << "loadGridfile " << filename;

  // Set name of grid file to access from pipeline
  setGridFilename(filename);

  // Set function to run in QT render thread
  /// dispatch_async(&(this->reassemblePipeline));  // HOW???

  reassemblePipeline();
  
  return true;
}


void TopoGridItem::reassemblePipeline() {
  // Dispatch lambda function to run in QT render thread 
  dispatch_async([this](vtkRenderWindow *renderWindow, vtkUserData userData) {
    auto *pipeline = TopoGridItem::Pipeline::SafeDownCast(userData);
    assemblePipeline(pipeline);
    return;
  });

  // Schedule update on the vtkRenderWindow
  scheduleRender();

  return;
}


void TopoGridItem::assemblePipeline(TopoGridItem::Pipeline *pipeline) {

  qDebug() << "assemblePipeline()";
  
  // Check that input file exists and is readable
  if (access(gridFilename_, R_OK) == -1) {
    qWarning() << "Can't access input file " << gridFilename_;
    return;
  }
  
  qDebug() << "set filename to " << gridFilename_;
  pipeline->gridReader_->SetFileName(gridFilename_);

  unsigned long errorCode;
  if ((errorCode = pipeline->gridReader_->GetErrorCode()) != 0) {
    qWarning() << "grid reader error during SetFileName(): "
	       << errorCode;

    qWarning() << gridFilename_ << ": " <<
      vtkErrorCode::GetStringFromErrorCode(errorCode);

    return;
  }
  
  // Clear actor list
  pipeline->renderer_->RemoveAllViewProps();  

  // Determine grid type
  TopoGridType gridType =
    TopoGridReader::getGridType(gridFilename_);

  pipeline->gridReader_->setGridType(gridType);

  // Update TopoGridReader
  qDebug() << "call gridReader_->Update()";
  pipeline->gridReader_->Update();

  if ((errorCode = pipeline->gridReader_->GetErrorCode()) != 0) {
    qWarning() << "grid reader error during Update(): "
	       << errorCode;
    
    qWarning() << gridFilename_ << ": " <<
      vtkErrorCode::GetStringFromErrorCode(errorCode);
    
    return;
  }  

  // Read grid bounds
  double gridBounds[6];
  pipeline->gridReader_->gridBounds(&gridBounds[0], &gridBounds[1],
				    &gridBounds[2], &gridBounds[3],
				    &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];

  pipeline->elevFilter_->SetInputConnection(pipeline->gridReader_->
					    GetOutputPort());
  
  pipeline->elevFilter_->SetLowPoint(0, 0, gridBounds[4]);
  pipeline->elevFilter_->SetHighPoint(0, 0, gridBounds[5]);
  // Preserve scalar values (keep minZ/maxZ range)
  pipeline->elevFilter_->SetScalarRange(gridBounds[4], gridBounds[5]);    

  pipeline->surfaceMapper_->SetInputConnection(pipeline->elevFilter_->
					       GetOutputPort());

  // Make lookup table
  TopoColorMap::makeLUT(scheme_,
			pipeline->elevLookupTable_);
    
  // Use scalar data to color objects
  pipeline->surfaceMapper_->ScalarVisibilityOn();
  // Scalar values range from min to max z (depth)
  pipeline->surfaceMapper_->SetScalarRange(gridBounds[4],
					   gridBounds[5]);    

  pipeline->surfaceMapper_->SetLookupTable(pipeline->elevLookupTable_);
  
  // Assign surfaceMapper to actor
  pipeline->surfaceActor_->SetMapper(pipeline->surfaceMapper_);
    
  // Add actor to renderer
  pipeline->renderer_->AddActor(pipeline->surfaceActor_);

  pipeline->renderer_->SetBackground(pipeline->colors_->GetColor3d("White").
				     GetData());
  

  if (plotAxes_) {
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
    pipeline->axesActor_->SetScale(1., 1., verticalExagg_);
  }
    
  pipeline->surfaceActor_->SetScale(1., 1., verticalExagg_);  // NEW!

  if (pipeline->firstRender_) {
    pipeline->renderer_->ResetCamera();
  }
  pipeline->firstRender_ = false;
}



bool TopoGridItem::setColormap(QString name) {

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


void TopoGridItem::showAxes(bool plotAxes) {
  qDebug() << "showAxes(): " << plotAxes;
  plotAxes_ = plotAxes;

  reassemblePipeline();

  return;
}
