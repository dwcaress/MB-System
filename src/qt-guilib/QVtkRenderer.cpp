#include <string.h>
#include <QQuickWindow>
#include <QOpenGLFramebufferObjectFormat>
#include <QDebug>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkColor.h>
#include <vtkStringArray.h>
#include <vtkVectorText.h>
#include <vtkAxes.h>
#include <vtkFollower.h>
#include <vtkParticleReader.h>
#include <vtkAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkSphereSource.h>
#include <vtkCoordinate.h>
#include "QVtkRenderer.h"
#include "QVtkItem.h"
#include "TopoColorMap.h"

using namespace mb_system;

QVtkRenderer::QVtkRenderer() :
  displayProperties_(nullptr),
  item_(nullptr),
  gridFilename_(nullptr),
  wheelEvent_(nullptr),
  mouseButtonEvent_(nullptr),
  mouseMoveEvent_(nullptr),
  pointPicked_(false),
  forceRender_(false),
  firstRender_(true) {
  
    worker_ = new LoadFileWorker(*this);

    // Handle things when worker thread finishes
    connect(worker_, &QThread::finished, this, &QVtkRenderer::handleFileLoaded,
            Qt::QueuedConnection);
  
}

QOpenGLFramebufferObject *QVtkRenderer::createFramebufferObject(const QSize &size) {

  // qDebug() << "QVtkRenderer::createFrameBufferObject";
  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

  // optionally enable multisampling by doing format.setSamples(4);
  return new QOpenGLFramebufferObject(size, format);
}


void QVtkRenderer::render() {

  if (!worker_->okToRender()) {
    qDebug() << "QVtkRenderer::render() do not render yet";
    return;
  }

  if (!renderWindow_ || !windowInteractor_) {
    qDebug() << "renderWindow/windowInteractor not yet defined";
    return;
  }

  qDebug() << "QVtkRenderer::render()";

  item_->window()->beginExternalCommands();


  renderWindow_->PushState(); 
  
  /// renderWindow_->OpenGLInitState();
  bool show = displayProperties_->showAxes();
  
  axesActor_->SetVisibility(displayProperties_->showAxes());

  if (displayProperties_->changed() || forceRender_) {
    // Some property changed, or force re-render - rebuild pipeline
    qDebug() <<
      "QVtkRenderer::render(), assemblePipeline()";
    assemblePipeline();
    forceRender_ = false;
    item_->clearPropertyChangedFlag();
  }

  if (wheelEvent_ && !wheelEvent_->isAccepted()) {
    qDebug() << "render(): handle wheelEvent";
    if (wheelEvent_->angleDelta().y() > 0) {
      qDebug() << "forwardWheel";
      windowInteractor_->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
    }
    else {
      qDebug() << "backwardWheel";      
      windowInteractor_->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
    }
    wheelEvent_->accept();
  }
  
  if (mouseButtonEvent_ && !mouseButtonEvent_->isAccepted()) {
    qDebug() << "render(): handle mouseButtonEvent";


    if (mouseButtonEvent_->type() == QEvent::MouseButtonPress) {

      bool cntrlKey =
        (mouseButtonEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0;
      
      bool shiftKey =
        (mouseButtonEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0;
      
      bool dblClick =
        mouseButtonEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0;
        
      if (mouseButtonEvent_->buttons() & Qt::LeftButton) {
        // qDebug() << "QVtkRenderer() - got left button";
        qDebug() << "x: " << mouseButtonEvent_->x() <<
          " y: " << mouseButtonEvent_->y();
        
        windowInteractor_->SetEventInformation(mouseButtonEvent_->x(),
                                               mouseButtonEvent_->y(),
                                               cntrlKey,
                                               shiftKey,
                                               dblClick);

        windowInteractor_->InvokeEvent(vtkCommand::LeftButtonPressEvent);
      }
      else if (mouseButtonEvent_->buttons() & Qt::RightButton) {
        // qDebug() << "QVtkRenderer() - got right button";
        windowInteractor_->SetEventInformation(mouseButtonEvent_->x(),
                                               mouseButtonEvent_->y(),
                                               cntrlKey,
                                               shiftKey,
                                               dblClick);
        
        windowInteractor_->InvokeEvent(vtkCommand::RightButtonPressEvent);
      }
      else if (mouseButtonEvent_->buttons() & Qt::MiddleButton) {
        // qDebug() << "QVtkRenderer() - got middle button";
        windowInteractor_->InvokeEvent(vtkCommand::MiddleButtonPressEvent);        
      }      
    }
    else if (mouseButtonEvent_->type() == QEvent::MouseButtonRelease) {
      // qDebug() << "QVtkRenderer: mouse button release";
      if (mouseButtonEvent_->button() == Qt::LeftButton) {
        // qDebug() << "QVtkRenderer: left mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::LeftButtonReleaseEvent);
      }
      else if (mouseButtonEvent_->button() == Qt::RightButton) {
        // qDebug() << "QVtkRenderer: right mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::RightButtonReleaseEvent);
      }
      else if (mouseButtonEvent_->button() == Qt::MiddleButton) {
        // qDebug() << "QVtkRenderer: right mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent);
      }      
    }
    mouseButtonEvent_->accept();
  }

  if (mouseMoveEvent_ && !mouseMoveEvent_->isAccepted()) {
    // qDebug() << "QVtkRenderer::render() - mouse move event";
    
    bool cntrlKey =
      (mouseButtonEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0;
      
    bool shiftKey =
      (mouseButtonEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0;
      
    bool dblClick =
      mouseButtonEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0;
    
    if (mouseMoveEvent_->type() == QEvent::MouseMove) {
      // Got left-button mouse-drag
      // qDebug() << "QVtkRenderer::render(): command mouse move; x=" <<
      // mouseMoveEvent_->x() << ", y=" <<
      // mouseMoveEvent_->y();

      windowInteractor_->SetEventInformation(mouseMoveEvent_->x(),
                                             mouseMoveEvent_->y(),
                                             cntrlKey,
                                             shiftKey,
                                             dblClick);

      windowInteractor_->InvokeEvent(vtkCommand::MouseMoveEvent);
      mouseMoveEvent_->accept();
    }
  }

  int *rendererSize = renderWindow_->GetSize();

  if (item_->width() != rendererSize[0] || item_->height() != rendererSize[1]) {
    // Resize render window to fit within its QVtkItem 
    qDebug() << "QVtkRenderer::render() resize renderWindow " << \
      item_->width() << " x " << item_->height();
    renderWindow_->SetSize(item_->width(), item_->height());
    qDebug() << "Resized";
  }

  qDebug() << "call renderWindow->Render()";
  renderWindow_->Render();
  qDebug() << "back from renderWindow->Render() - reset opengl state";
  
  // Done with render. Reset OpenGL state
  renderWindow_->PopState();
  qDebug() << "back from PopState()";

  item_->window()->endExternalCommands();

  return;
}


// Copy data from item to this renderer
void QVtkRenderer::synchronize(QQuickFramebufferObject *item) {
  qDebug() << "QVtkRenderer::synchronize()";
  if (!item_) {
    // The item argument is the QVtkItem associated with this renderer;
    // keep a copy as item_ member
    item_ = static_cast<QVtkItem *>(item);
  }

  // Copy pointer to display properties
  displayProperties_ = item_->displayProperties();
  
  if (gridFilenameChanged(item_->getGridFilename())) {

    gridReader_ = vtkSmartPointer<TopoDataReader>::New();

    qDebug() << "synchronize(): change busy state to true";
    item_->setAppBusy(true);

    qDebug() << "synchronize(): start worker thread";
    worker_->start();
    qDebug() << "synchronize(): worker started!";
  }
  
  // Mouse wheel moved
  if (item_->latestWheelEvent() &&
      !item_->latestWheelEvent()->isAccepted()) {
    // Copy and accept latest wheel event
    // qDebug() << "synchronize() - copy wheelEvent";
    // Get latest wheel event generated by the QVtkItem
    wheelEvent_ = item_->latestWheelEvent();
    item_->latestWheelEvent()->accept();
  }

  // Mouse button pressed/released
  if (item_->latestMouseButtonEvent() &&
      !item_->latestMouseButtonEvent()->isAccepted()) {
    // qDebug() << "synchronize() - copy mouseButtonEvent";
    // Get latest mouse button event generated by the QVtkItem
    mouseButtonEvent_ = item_->latestMouseButtonEvent();
    item_->latestMouseButtonEvent()->accept();
  }

  // Mouse moved
  if (item_->latestMouseMoveEvent() &&
      !item_->latestMouseMoveEvent()->isAccepted()) {
    // qDebug() << "synchronize() - copy mouseMoveEvent";
    // Get latest mouse move event generated by the QVtkItem    
    mouseMoveEvent_ = item_->latestMouseMoveEvent();
    item_->latestMouseMoveEvent()->accept();
  }
}


bool QVtkRenderer::initializePipeline(const char *gridFilename) {
  qDebug() << "QVtkRenderer::initializePipeline() " << gridFilename;
  
  item_->window()->beginExternalCommands();

  // Colors for axes
  namedColors_ = vtkSmartPointer<vtkNamedColors>::New();

  // Color data points based on z-value
  elevColorizer_ =
    vtkSmartPointer<vtkElevationFilter>::New();

  // Lookup table colorizing topo surface
  elevLookupTable_ =
    vtkSmartPointer<vtkLookupTable>::New();

  // Create VTK renderer (not the same as QT renderer)
  qDebug() << "create vtk renderer";
  renderer_ =
    vtkSmartPointer<vtkRenderer>::New();

  // Create rotation transform and filter
  transform_ = vtkSmartPointer<vtkTransform>::New();
  transformFilter_ = vtkSmartPointer<vtkTransformFilter>::New();
  
  // Create mapper
  qDebug() << "create vtk mapper";
  surfaceMapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();

  // Create actor for grid surface
  qDebug() << "create vtk actor";
  surfaceActor_ = vtkSmartPointer<vtkActor>::New();

  // Create vtk renderWindow
  qDebug() << "create renderWindow";

  renderWindow_ =
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

  renderWindow_->OpenGLInitState();

  // Create vtk windowInteractor
  qDebug() << "create windowInteractor";
  windowInteractor_ =
    vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();

  // Create interactor style
  interactorStyle_ =
    vtkSmartPointer<PickerInteractorStyle>::New();  

  interactorStyle_->initialize(this, windowInteractor_);

  // Axes actor
  //  axesActor_ = vtkSmartPointer<vtkCubeAxesActor2D>::New();
  axesActor_ = vtkSmartPointer<vtkCubeAxesActor>::New();  

  // Invoke callback when renderWindow_ is made current
  renderWindow_->AddObserver(vtkCommand::WindowMakeCurrentEvent,
                             this, &QVtkRenderer::makeCurrentCallback);

  firstRender_ = true;

  item_->window()->beginExternalCommands();
  
  return assemblePipeline();
}

// vtkCubeAxesActor2D version
void QVtkRenderer::setupAxes(vtkCubeAxesActor2D *axesActor,
                             vtkNamedColors *namedColors,
                             double *surfaceBounds,
                             double *gridBounds,
                             const char *xUnits, const char *yUnits,
                             const char *zUnits) {

  qDebug() << "setupAxes(): " <<
    " xMin: " << surfaceBounds[0] << ", xMax: " << surfaceBounds[1] <<
    ", yMin: " << surfaceBounds[2] << ", yMax: " << surfaceBounds[3] <<
    ", zMin: " << surfaceBounds[4] << ", zMax: " << surfaceBounds[5];


  vtkNew<vtkTextProperty> text;
  text->SetColor(namedColors->GetColor3d("Black").GetData());
  axesActor->SetAxisTitleTextProperty(text.GetPointer());
  axesActor->SetAxisLabelTextProperty(text.GetPointer());

  axesActor->GetProperty()->SetColor(0., 0., 0.);
  
  axesActor->SetBounds(surfaceBounds);

  qDebug() << "setupAxes(): set X axis range "
           << gridBounds[0] << " - " << gridBounds[1];
  
  axesActor->GetXAxisActor2D()->SetRange(gridBounds[0], gridBounds[1]);
  axesActor->GetYAxisActor2D()->SetRange(gridBounds[2], gridBounds[3]);  
  axesActor->GetZAxisActor2D()->SetRange(gridBounds[4], gridBounds[5]);

  axesActor->SetXLabel(xUnits);  
  axesActor->SetYLabel(yUnits);  
  axesActor->SetZLabel(zUnits);

  axesActor->SetLabelFormat("%.0f");
}


// vtkCubeAxesActor version
void QVtkRenderer::setupAxes(vtkCubeAxesActor *axesActor,
                             vtkNamedColors *namedColors,                                                    double *surfaceBounds,
                             double *gridBounds,
                             const char *xUnits, const char *yUnits,
                             const char *zUnits) {

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
  if (gridReader_->geographicCRS()) {
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

bool QVtkRenderer::assemblePipeline() {

  qDebug() << "QVtkRenderer::assemblePipeline() for " << gridFilename_;

  if (gridReader_->GetErrorCode() != 0) {
    std::cerr << "gridReader_ error code: " << gridReader_->GetErrorCode() <<
      "\n";
    return false;
  }
  
  // Clear actor list
  renderer_->RemoveAllViewProps();
  
  qDebug() << "renderer_ has " << renderer_->GetActors()->GetNumberOfItems() << " actors";

  // Read grid bounds
  double gridBounds[6];
  gridReader_->gridBounds(&gridBounds[0], &gridBounds[1],
                          &gridBounds[2], &gridBounds[3],
                          &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];


  double *dBounds = gridReader_->GetOutput()->GetBounds();
  
  qDebug() << "GetBounds() - xMin: " << dBounds[0] << ", xMax: " <<
    dBounds[1] <<
    "yMin: " << dBounds[2] << ", yMax: " << dBounds[3] <<
    "zMin: " << dBounds[4] << ", zMax: " << dBounds[5];

  elevColorizer_->SetInputConnection(gridReader_->GetOutputPort());  
  elevColorizer_->SetLowPoint(0, 0, gridBounds[4]);
  elevColorizer_->SetHighPoint(0, 0, gridBounds[5]);

  /// Scale z axis based on vertical exaggeration
  float zScale = displayProperties_->verticalExagg() *
    gridReader_->zScaleLatLon();
  ;
  transform_ = vtkSmartPointer<vtkTransform>::New();
  transform_->Scale(1., 1., zScale);
  transformFilter_->SetTransform(transform_);
  transformFilter_->SetInputConnection(elevColorizer_->GetOutputPort());
  surfaceMapper_->SetInputConnection(transformFilter_->GetOutputPort());

  elevColorizer_->SetScalarRange(dBounds[4], dBounds[5]);
  TopoColorMap::makeLUT(displayProperties_->colorMapScheme(),
                        elevLookupTable_);
    
  surfaceMapper_->SetScalarRange(dBounds[4], dBounds[5]);
  surfaceMapper_->ScalarVisibilityOn();
  surfaceMapper_->SetLookupTable(elevLookupTable_);
  
  // Assign surfaceMapper to actor
  qDebug() << "assign surfaceMapper to actor";
  surfaceActor_->SetMapper(surfaceMapper_);
  
  // Add actor to renderer
  renderer_->AddActor(surfaceActor_);

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

  // Add renderer to the renderWindow
  qDebug() << "add renderer to renderWindow";
  renderWindow_->AddRenderer(renderer_);

  interactorStyle_->SetDefaultRenderer(renderer_);
  interactorStyle_->polyData_ = gridReader_->GetOutput();

  windowInteractor_->SetInteractorStyle(interactorStyle_);
  windowInteractor_->SetRenderWindow(renderWindow_);
  
  // Per QtVTK example
  windowInteractor_->EnableRenderOff();
  vtkColor3d axisColor = namedColors_->GetColor3d("black");

  // Set up axes
  setupAxes(axesActor_,
            namedColors_,
            surfaceMapper_->GetBounds(),
            gridBounds,
            gridReader_->xUnits(),
            gridReader_->yUnits(),
            gridReader_->zUnits());

  axesActor_->SetCamera(renderer_->GetActiveCamera());

  renderer_->AddActor(axesActor_);    

  if (firstRender_) {
    // Reset on first rendering of this grid
    renderer_->ResetCamera();
  }

  // Don't reset on subsequent rendering of this grid
  firstRender_ = false;
  


  // Initialize displayed picked coordinates to blank
  // QString msg("");
  // item_->setPickedPoint(msg);
  
  qDebug() << "pipeline assembled";  
  return true;

}



bool QVtkRenderer::gridFilenameChanged(char *filename) {
  bool changed = false;

  if (!filename && gridFilename_) {
    // Item now has no gridFilename, free renderer's member and set to null
    free((void *)gridFilename_);
    gridFilename_ = nullptr;
    changed = true;
  }
  else if (filename && !gridFilename_) {
    // Item now has gridFilename, renderer doesn't - copy it
    gridFilename_ = strdup(filename);
    changed = true;
  }
  else if (filename && gridFilename_) {
    if (strcmp(filename, gridFilename_)) {
      // Item filename differs from renderer's - copy it
      changed = true;
      free((void *)gridFilename_);
      gridFilename_ = strdup(filename);
    }
  }
  
  return changed;
}



void QVtkRenderer::handleFileLoaded() {
  // Called when worker thread is finished


  qDebug() << "handleFileLoaded() current thread: ";
  qDebug() << QThread::currentThread();

  // Render the FBO again
  update();

  // Initialize the OpenGL context for the renderer
  qDebug() << "initialize OpenGL context for renderer";
  /// renderWindow_->OpenGLInitContext();
  
  qDebug() << "handleFileLoaded(): change busy state to false";
  item_->setAppBusy(false);

  
}



QVtkRenderer::LoadFileWorker::LoadFileWorker(QVtkRenderer &parent) :
  parent_(parent) {}
  

void QVtkRenderer::LoadFileWorker::run() {
  okToRender_ = true;
  qDebug() << "QVtkRenderer::LoadFileLoader::run()";

  qDebug() << "before gridReader->SetFileName(): ErrorCode: " <<
    parent_.gridReader_->GetErrorCode();
  
  parent_.gridReader_->SetFileName(parent_.gridFilename_);

  qDebug() << "after gridReader->SetFileName(): ErrorCode: " <<
    parent_.gridReader_->GetErrorCode();
  
  TopoDataType gridType =
    TopoDataReader::getDataType(parent_.gridFilename_);

  parent_.gridReader_->setDataType(gridType);

  qDebug() << "before gridReader->Update(): ErrorCode: " <<
    parent_.gridReader_->GetErrorCode();
  
  parent_.gridReader_->Update();

  if (parent_.gridReader_->GetErrorCode()) {
    std::cerr << "Error during gridReader Update(): " <<
      parent_.gridReader_->GetErrorCode() << std::endl;

    parent_.gridReader_->SetFileName("");
  }

  // Critical region - don't render during this phase
  okToRender_ = false;
  qDebug() << "**** handleFileLoaded() - initialize pipeline";
  // Grid file is loaded - initialize pipeline
  parent_.initializePipeline(parent_.gridFilename_);
  qDebug() << "**** handleFileLoaded() - pipeline ready";    

  // All done - ok to render
  okToRender_ = true;  
  qDebug() << "QVtkRenderer::LoadFileLoader::run() finished";
}


/// Assert renderWindow_ as current in response to WindowMakeCurrent event
void QVtkRenderer::makeCurrentCallback(vtkObject *, unsigned long eid,
                                       void *callData) {

  //  std::cout << "makeCurrentCallback()!" << std::endl;

  // Assert render window as current
  renderWindow_->SetIsCurrent(true);
}


void QVtkRenderer::setPickedPoint(double *worldCoords) {

  pointPicked_ = true;

  pickedCoords_[0] = worldCoords[0];
  pickedCoords_[1] = worldCoords[1];
  pickedCoords_[2] = worldCoords[2];

  /// Force render on next update
  forceRender_ = true;
}


bool QVtkRenderer::getPickedPoint(double *worldCoords) {
  if (!pointPicked_) {
    return false;
  }

  worldCoords[0] = pickedCoords_[0];
  worldCoords[1] = pickedCoords_[1];
  worldCoords[2] = pickedCoords_[2];

  return true;
}
  
