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
#include <vtkNamedColors.h>
#include <vtkColor.h>
#include "QVtkRenderer.h"
#include "QVtkItem.h"
#include "GmtGridReader.h"

using namespace mb_system;

QVtkRenderer::QVtkRenderer() :
  displayProperties_(nullptr),
  item_(nullptr),
  initialized_(false),
  gridFilename_(nullptr),
  wheelEvent_(nullptr),
  mouseButtonEvent_(nullptr),
  mouseMoveEvent_(nullptr)
{

}

QOpenGLFramebufferObject *QVtkRenderer::createFramebufferObject(const QSize &size) {

  qDebug() << "QVtkRenderer::createFrameBufferObject";
  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

  // optionally enable multisampling by doing format.setSamples(4);
  return new QOpenGLFramebufferObject(size, format);
}


void QVtkRenderer::render() {
  qDebug() << "QVtkRenderer::render()";
  if (!renderWindow_) {
    qDebug() << "renderWindow not yet defined";
    return;
  }

  double angle = renderer_->GetActiveCamera()->GetViewAngle();
  qDebug() << "*** render(): viewAngle = " << angle;

  /* ***
     double position[3];
     renderer_->GetActiveCamera()->GetPosition(position);

     qDebug() << "*** render(): position = " << position[0] << ", " << position[1]
     << ", " << position[2];
  
     *** */
  
  renderWindow_->PushState();
  initializeOpenGLState();
  renderWindow_->Start();

  if (!initialized_) {
    initialize();
    initialized_ = true;
  }

  if (wheelEvent_ && !wheelEvent_->isAccepted()) {
    qDebug() << "render(): handle wheelEvent";
    if (wheelEvent_->delta() > 0) {
      renderWindowInteractor_->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
    }
    else {
      renderWindowInteractor_->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
    }
    wheelEvent_->accept();
  }

  if (mouseButtonEvent_ && !mouseButtonEvent_->isAccepted()) {
    qDebug() << "render(): handle mouseButtonEvent";

    renderWindowInteractor_->SetEventInformationFlipY(mouseButtonEvent_->x(),
						      mouseButtonEvent_->y(),
						      (mouseButtonEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
						      (mouseButtonEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0,
						      mouseButtonEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0);

    if (mouseButtonEvent_->type() == QEvent::MouseButtonPress) {
      qDebug() << "mouse button press";
      renderWindowInteractor_->InvokeEvent(vtkCommand::LeftButtonPressEvent);
    }
    else if (mouseButtonEvent_->type() == QEvent::MouseButtonRelease) {
      qDebug() << "mouse button release";
      renderWindowInteractor_->InvokeEvent(vtkCommand::LeftButtonReleaseEvent);

    }
    mouseButtonEvent_->accept();
  }

  if (mouseMoveEvent_ && !mouseMoveEvent_->isAccepted()) {
    if (mouseMoveEvent_->type() == QEvent::MouseMove &&
	mouseMoveEvent_->buttons() & Qt::RightButton) {
      // Got right-button mouse-drag
      qDebug() << "render(): command mouse move; x=" <<
	mouseMoveEvent_->x() << ", y=" <<
	mouseMoveEvent_->y();

      renderWindowInteractor_->SetEventInformationFlipY(mouseMoveEvent_->x(),
							mouseMoveEvent_->y(),
							(mouseMoveEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
							(mouseMoveEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0,
							mouseMoveEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0);

      renderWindowInteractor_->InvokeEvent(vtkCommand::MouseMoveEvent);
      mouseMoveEvent_->accept();
    }
    else {
      qDebug() << "render(): no action on mouseMove event";
    }
  }

  int *rendererSize = renderWindow_->GetSize();

  if (item_->width() != rendererSize[0] || item_->height() != rendererSize[1])
    {
      renderWindow_->SetSize(item_->width(), item_->height());
    }

  renderWindow_->Render();

  // Done with render. Reset OpenGL state
  renderWindow_->PopState();
  item_->window()->resetOpenGLState();;
}


void QVtkRenderer::synchronize(QQuickFramebufferObject *item) {
  qDebug() << "QVtkRenderer::synchronize()";

  // Copy data from item to this renderer
  if (!item_) {
    item_ = static_cast<QVtkItem *>(item);
    displayProperties_ = item_->displayProperties();
  }

  char *gridFilename = item_->getGridFilename();
  bool filenameChanged = false;
  if (!gridFilename && gridFilename_) {
    free((void *)gridFilename_);
    filenameChanged = true;
  }
  else if (gridFilename && !gridFilename_) {
    gridFilename_ = strdup(gridFilename);
    filenameChanged = true;
  }
  else if (gridFilename && gridFilename_) {
    if (strcmp(gridFilename, gridFilename_)) {
      filenameChanged = true;
      free((void *)gridFilename_);
      gridFilename_ = strdup(gridFilename);
    }
  }
  if (filenameChanged) {
    // New grid file specified - load it into a pipeline
    initializePipeline(gridFilename_);
  }

  if (item_->latestWheelEvent() &&
      !item_->latestWheelEvent()->isAccepted()) {
    // Copy and accept latest wheel event
    qDebug() << "synchronize() - copy wheelEvent";
    wheelEvent_ = std::make_shared<QWheelEvent>(*item_->latestWheelEvent());
    item_->latestWheelEvent()->accept();
  }

  if (item_->latestMouseButtonEvent() &&
      !item_->latestMouseButtonEvent()->isAccepted()) {
    qDebug() << "synchronize() - copy mouseButtonEvent";
    mouseButtonEvent_ =
      std::make_shared<QMouseEvent>(*item_->latestMouseButtonEvent());
    item_->latestMouseButtonEvent()->accept();
  }

  if (item_->latestMouseMoveEvent() &&
      !item_->latestMouseMoveEvent()->isAccepted()) {
    qDebug() << "synchronize() - copy mouseMoveEvent";
    mouseMoveEvent_ =
      std::make_shared<QMouseEvent>(*item_->latestMouseMoveEvent());
    item_->latestMouseMoveEvent()->accept();
  }


  // Copy pointer to display properties
  displayProperties_ = item_->displayProperties();
  
}


void QVtkRenderer::initialize() {
  qDebug() << "QVtkRenderer::initialize()";

  if (gridFilename_ && !initializePipeline(gridFilename_)) {
    qCritical() << "initializePipeline() failed for" << gridFilename_;
  }
  initialized_ = true;
}

bool QVtkRenderer::initializePipeline(const char *gridFilename) {
  qDebug() << "QVtkRenderer::initializePipeline() " << gridFilename;

  gridReader_ =
    vtkSmartPointer<GmtGridReader>::New();

  gridReader_->SetFileName ( gridFilename );
  qDebug() << "reader->Update()";
  gridReader_->Update();

  // Color data points based on z-value
  elevColorizer_ =
    vtkSmartPointer<vtkElevationFilter>::New();

  elevColorizer_->SetInputConnection(gridReader_->GetOutputPort());
  float xMin, xMax, yMin, yMax, zMin, zMax;
  gridReader_->bounds(&xMin, &xMax, &yMin, &yMax, &zMax, &zMin);
  elevColorizer_->SetLowPoint(0, 0, zMin);
  elevColorizer_->SetHighPoint(0, 0, zMax);

  // Visualize the data...

  // Create renderer
  qDebug() << "create vtk renderer";
  renderer_ =
    vtkSmartPointer<vtkRenderer>::New();

  // Create mapper
  qDebug() << "create vtk mapper";
  mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();

  qDebug() << "mapper->SetInputConnection()";
  mapper_->SetInputConnection(elevColorizer_->GetOutputPort());

  // Create actor for grid surface
  qDebug() << "create vtk actor";
  surfaceActor_ = vtkSmartPointer<vtkActor>::New();

  // Assign mapper to actor
  qDebug() << "assign mapper to actor";
  surfaceActor_->SetMapper(mapper_);

  if (displayProperties_->drawAxes) {
    // Axes for surface
    vtkSmartPointer<vtkNamedColors> colors = 
      vtkSmartPointer<vtkNamedColors>::New();

    vtkColor3d axisColor = colors->GetColor3d("Black");
    vtkColor3d labelColor = colors->GetColor3d("Black");
  
    axesActor_ = vtkSmartPointer<vtkCubeAxesActor>::New();
    axesActor_->SetUseTextActor3D(1);
    axesActor_->SetBounds(gridReader_->GetOutput()->GetBounds());
    axesActor_->SetCamera(renderer_->GetActiveCamera());
    axesActor_->GetTitleTextProperty(0)->SetFontSize(48);
    axesActor_->DrawXGridlinesOn();
    axesActor_->DrawYGridlinesOn();
    axesActor_->DrawZGridlinesOn();

    // Set axis line color
    axesActor_->GetXAxesLinesProperty()->SetColor(axisColor.GetData());
    axesActor_->GetYAxesLinesProperty()->SetColor(axisColor.GetData());
    axesActor_->GetZAxesLinesProperty()->SetColor(axisColor.GetData());
  
    axesActor_->GetXAxesGridlinesProperty()->SetColor(axisColor.GetData());
    axesActor_->GetYAxesGridlinesProperty()->SetColor(axisColor.GetData());
    axesActor_->GetZAxesGridlinesProperty()->SetColor(axisColor.GetData());

    axesActor_->GetTitleTextProperty(0)->SetColor(labelColor.GetData());
    axesActor_->GetLabelTextProperty(0)->SetColor(labelColor.GetData());
    axesActor_->GetTitleTextProperty(1)->SetColor(labelColor.GetData());
    axesActor_->GetLabelTextProperty(1)->SetColor(labelColor.GetData());
    axesActor_->GetTitleTextProperty(2)->SetColor(labelColor.GetData());
    axesActor_->GetLabelTextProperty(2)->SetColor(labelColor.GetData());       

  
#if VTK_MAJOR_VERSION == 6
    axesActor_->SetGridLineLocation(VTK_GRID_LINES_FURTHEST);
#endif
#if VTK_MAJOR_VERSION > 6
    axesActor_->SetGridLineLocation(
				    axesActor_->VTK_GRID_LINES_FURTHEST);
#endif
  
    axesActor_->XAxisMinorTickVisibilityOff();
    axesActor_->YAxisMinorTickVisibilityOff();
    axesActor_->ZAxisMinorTickVisibilityOff();

    axesActor_->SetFlyModeToStaticEdges();
  }
  
  // Create vtk renderWindow
  qDebug() << "create renderWindow";
  renderWindow_ =
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

  // Add renderer to the renderWindow
  qDebug() << "add renderer to renderWindow";
  renderWindow_->AddRenderer(renderer_);

  // Create vtk renderWindowInteractor
  qDebug() << "create renderWindowInteractor";
  renderWindowInteractor_ =
    vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();

  renderWindowInteractor_->EnableRenderOff();
  qDebug() << "renderWindow_->SetInteractor()";

  vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
    vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

  renderWindowInteractor_->SetInteractorStyle(style);

  renderWindow_->SetInteractor(renderWindowInteractor_);

  renderer_->AddActor(surfaceActor_);

  if (displayProperties_->drawAxes) {
    renderer_->AddActor(axesActor_);    
  }
  
  renderer_->ResetCamera();

  // Initialize the OpenGL context for the renderer
  renderWindow_->OpenGLInitContext();

  qDebug() << "rendererr->AddActor()";

  return true;
}


void QVtkRenderer::initializeOpenGLState()
{
  renderWindow_->OpenGLInitState();
  renderWindow_->MakeCurrent();
  QOpenGLFunctions::initializeOpenGLFunctions();
  QOpenGLFunctions::glUseProgram(0);
}

