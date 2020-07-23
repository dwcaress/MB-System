#include <queue>

#include <QQuickWindow>

#include <vtkAxesActor.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkCellArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLight.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkSTLReader.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <QVTKOpenGLNativeWidget.h>

#include "CommandModel.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include "QVtkItem.h"
#include "QVtkRenderer.h"

QVtkRenderer::QVtkRenderer()
{
  // Renderer
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
  m_vtkRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_vtkRenderWindow->AddRenderer(m_renderer);

  // Interactor
  m_vtkRenderWindowInteractor = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
  m_vtkRenderWindowInteractor->EnableRenderOff();
  m_vtkRenderWindow->SetInteractor(m_vtkRenderWindowInteractor);

  // Initialize the OpenGL context for the renderer
  m_vtkRenderWindow->OpenGLInitContext();

  // Interactor Style
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  style->SetDefaultRenderer(m_renderer);
  style->SetMotionFactor(10.0);
  m_vtkRenderWindowInteractor->SetInteractorStyle(style);

  // Picker
  m_picker = vtkSmartPointer<vtkCellPicker>::New();
  m_picker->SetTolerance(0.0);

  update();
}

void QVtkRenderer::setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine)
{
  m_processingEngine = std::shared_ptr<ProcessingEngine>(processingEngine);
}

void QVtkRenderer::synchronize(QQuickFramebufferObject *item)
{
  // For the first synchronize
  if (!m_vtkFboItem)
    {
      m_vtkFboItem = static_cast<QVtkItem *>(item);
    }

  if (!m_vtkFboItem->isInitialized())
    {
      m_vtkFboItem->setVtkFboRenderer(this);

      emit m_vtkFboItem->rendererInitialized();
    }

  int *rendererSize = m_vtkRenderWindow->GetSize();

  if (m_vtkFboItem->width() != rendererSize[0] || m_vtkFboItem->height() != rendererSize[1])
    {
      m_vtkRenderWindow->SetSize(m_vtkFboItem->width(), m_vtkFboItem->height());
    }

  // Copy mouse events
  if (!m_vtkFboItem->getLastMouseLeftButton()->isAccepted())
    {
      m_mouseLeftButton = std::make_shared<QMouseEvent>(*m_vtkFboItem->getLastMouseLeftButton());
      m_vtkFboItem->getLastMouseLeftButton()->accept();
    }

  if (!m_vtkFboItem->getLastMouseButton()->isAccepted())
    {
      m_mouseEvent = std::make_shared<QMouseEvent>(*m_vtkFboItem->getLastMouseButton());
      m_vtkFboItem->getLastMouseButton()->accept();
    }

  if (!m_vtkFboItem->getLastMoveEvent()->isAccepted())
    {
      m_moveEvent = std::make_shared<QMouseEvent>(*m_vtkFboItem->getLastMoveEvent());
      m_vtkFboItem->getLastMoveEvent()->accept();
    }

  if (!m_vtkFboItem->getLastWheelEvent()->isAccepted())
    {
      m_wheelEvent = std::make_shared<QWheelEvent>(*m_vtkFboItem->getLastWheelEvent());
      m_vtkFboItem->getLastWheelEvent()->accept();
    }

  // Get extra data
  m_modelsRepresentationOption = m_vtkFboItem->getModelsRepresentation();
  m_modelsOpacity = m_vtkFboItem->getModelsOpacity();
  m_modelsGouraudInterpolation = m_vtkFboItem->getGourauInterpolation();
  Model::setSelectedModelColor(QColor(m_vtkFboItem->getModelColorR(), m_vtkFboItem->getModelColorG(), m_vtkFboItem->getModelColorB()));
}

void QVtkRenderer::render()
{
  m_vtkRenderWindow->PushState();
  this->openGLInitState();
  m_vtkRenderWindow->Start();

  if (m_firstRender)
    {
      this->initScene();
      m_firstRender = false;
    }

  // Process camera related commands

  // Process mouse event
  if (m_mouseEvent && !m_mouseEvent->isAccepted())
    {
      m_vtkRenderWindowInteractor->SetEventInformationFlipY(m_mouseEvent->x(), m_mouseEvent->y(),
							    (m_mouseEvent->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
							    (m_mouseEvent->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0,
							    m_mouseEvent->type() == QEvent::MouseButtonDblClick ? 1 : 0);

      if (m_mouseEvent->type() == QEvent::MouseButtonPress)
	{
	  m_vtkRenderWindowInteractor->InvokeEvent(vtkCommand::LeftButtonPressEvent, m_mouseEvent.get());
	}
      else if (m_mouseEvent->type() == QEvent::MouseButtonRelease)
	{
	  m_vtkRenderWindowInteractor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, m_mouseEvent.get());
	}

      m_mouseEvent->accept();
    }

  // Process move event
  if (m_moveEvent && !m_moveEvent->isAccepted())
    {
      if (m_moveEvent->type() == QEvent::MouseMove && m_moveEvent->buttons() & Qt::RightButton)
	{
	  m_vtkRenderWindowInteractor->SetEventInformationFlipY(m_moveEvent->x(), m_moveEvent->y(),
								(m_moveEvent->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
								(m_moveEvent->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0, 0,
								m_moveEvent->type() == QEvent::MouseButtonDblClick ? 1 : 0);

	  m_vtkRenderWindowInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, m_moveEvent.get());
	}

      m_moveEvent->accept();
    }

  // Process wheel event
  if (m_wheelEvent && !m_wheelEvent->isAccepted())
    {
      if (m_wheelEvent->delta() > 0)
	{
	  m_vtkRenderWindowInteractor->InvokeEvent(vtkCommand::MouseWheelForwardEvent, m_wheelEvent.get());
	}
      else if (m_wheelEvent->delta() < 0)
	{
	  m_vtkRenderWindowInteractor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, m_wheelEvent.get());
	}

      m_wheelEvent->accept();
    }

  // Process model related commands

  // Select model

  if (m_mouseLeftButton && !m_mouseLeftButton->isAccepted())
    {
      this->selectModel(m_mouseLeftButton->x(), m_mouseLeftButton->y());
      m_mouseLeftButton->accept();
    }

  // Model transformations

  CommandModel *command;
  while (!m_vtkFboItem->isCommandsQueueEmpty())
    {
      m_vtkFboItem->lockCommandsQueueMutex();

      command = m_vtkFboItem->getCommandsQueueFront();
      if (!command->isReady())
	{
	  m_vtkFboItem->unlockCommandsQueueMutex();
	  break;
	}
      m_vtkFboItem->commandsQueuePop();

      m_vtkFboItem->unlockCommandsQueueMutex();

      command->execute();
    }

  // Reset the view-up vector. This improves the interaction of the camera with the plate.
  m_renderer->GetActiveCamera()->SetViewUp(0.0, 0.0, 1.0);

  // Extra actions
  m_processingEngine->setModelsRepresentation(m_modelsRepresentationOption);
  m_processingEngine->setModelsOpacity(m_modelsOpacity);
  m_processingEngine->setModelsGouraudInterpolation(m_modelsGouraudInterpolation);
  m_processingEngine->updateModelsColor();

  // Render
  m_vtkRenderWindow->Render();
  m_vtkRenderWindow->PopState();

  m_vtkFboItem->window()->resetOpenGLState();
}

void QVtkRenderer::openGLInitState()
{
  m_vtkRenderWindow->OpenGLInitState();
  m_vtkRenderWindow->MakeCurrent();
  QOpenGLFunctions::initializeOpenGLFunctions();
  QOpenGLFunctions::glUseProgram(0);
}

QOpenGLFramebufferObject *QVtkRenderer::createFramebufferObject(const QSize &size)
{
  //	QSize macSize = QSize(size.width() / 2, size.height() / 2);

  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::Depth);

#ifdef Q_OS_MAC
  std::unique_ptr<QOpenGLFramebufferObject> framebufferObject(new QOpenGLFramebufferObject(macSize, format));
#else
  std::unique_ptr<QOpenGLFramebufferObject> framebufferObject(new QOpenGLFramebufferObject(size, format));
#endif
  m_vtkRenderWindow->SetBackLeftBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetFrontLeftBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetBackBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetFrontBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetSize(framebufferObject->size().width(), framebufferObject->size().height());
  m_vtkRenderWindow->SetOffScreenRendering(true);
  m_vtkRenderWindow->Modified();

  return framebufferObject.release();
}

void QVtkRenderer::initScene()
{
  qDebug() << "QVtkRenderer::initScene()";

  m_vtkRenderWindow->SetOffScreenRendering(true);

  // Top background color
  double r2 = 245 / 255.0;
  double g2 = 245 / 255.0;
  double b2 = 245 / 255.0;

  // Bottom background color
  double r1 = 170 / 255.0;
  double g1 = 170 / 255.0;
  double b1 = 170 / 255.0;

  m_renderer->SetBackground(r2, g2, b2);
  m_renderer->SetBackground2(r1, g1, b1);
  m_renderer->GradientBackgroundOn();

  // Axes
  vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
  double axes_length = 20.0;
  int16_t axes_label_font_size = 20;
  axes->SetTotalLength(axes_length, axes_length, axes_length);
  axes->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axes->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axes->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
  axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(axes_label_font_size);
  axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(axes_label_font_size);
  axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(axes_label_font_size);
  m_renderer->AddActor(axes);

  // Platform
  this->generatePlatform();

  // Initial camera position
  this->resetCamera();
}

void QVtkRenderer::generatePlatform()
{
  qDebug() << "QVtkRenderer::generatePlatform()";

  // Platform Model
  vtkSmartPointer<vtkPolyDataMapper> platformModelMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  m_platformModel = vtkSmartPointer<vtkCubeSource>::New();
  platformModelMapper->SetInputConnection(m_platformModel->GetOutputPort());

  m_platformModelActor = vtkSmartPointer<vtkActor>::New();
  m_platformModelActor->SetMapper(platformModelMapper);
  m_platformModelActor->GetProperty()->SetColor(1, 1, 1);
  m_platformModelActor->GetProperty()->LightingOn();
  m_platformModelActor->GetProperty()->SetOpacity(1);
  m_platformModelActor->GetProperty()->SetAmbient(0.45);
  m_platformModelActor->GetProperty()->SetDiffuse(0.4);

  m_platformModelActor->PickableOff();
  m_renderer->AddActor(m_platformModelActor);

  // Platform Grid
  m_platformGrid = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkPolyDataMapper> platformGridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  platformGridMapper->SetInputData(m_platformGrid);

  m_platformGridActor = vtkSmartPointer<vtkActor>::New();
  m_platformGridActor->SetMapper(platformGridMapper);
  m_platformGridActor->GetProperty()->LightingOff();
  m_platformGridActor->GetProperty()->SetColor(0.45, 0.45, 0.45);
  m_platformGridActor->GetProperty()->SetOpacity(1);
  m_platformGridActor->PickableOff();
  m_renderer->AddActor(m_platformGridActor);

  this->updatePlatform();
}

void QVtkRenderer::updatePlatform()
{
  qDebug() << "QVtkRenderer::updatePlatform()";

  // Platform Model

  if (m_platformModel)
    {
      m_platformModel->SetXLength(m_platformWidth);
      m_platformModel->SetYLength(m_platformDepth);
      m_platformModel->SetZLength(m_platformThickness);
      m_platformModel->SetCenter(0.0, 0.0, -m_platformThickness / 2);
    }

  // Platform Grid
  vtkSmartPointer<vtkPoints> gridPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> gridCells = vtkSmartPointer<vtkCellArray>::New();

  for (int16_t i = -m_platformWidth / 2; i <= m_platformWidth / 2; i += m_gridSize)
    {
      createLine(i, -m_platformDepth / 2, m_gridBottomHeight, i, m_platformDepth / 2, m_gridBottomHeight, gridPoints, gridCells);
    }

  for (int16_t i = -m_platformDepth / 2; i <= m_platformDepth / 2; i += m_gridSize)
    {
      createLine(-m_platformWidth / 2, i, m_gridBottomHeight, m_platformWidth / 2, i, m_gridBottomHeight, gridPoints, gridCells);
    }

  m_platformGrid->SetPoints(gridPoints);
  m_platformGrid->SetLines(gridCells);
}

void QVtkRenderer::createLine(const double x1, const double y1, const double z1, const double x2, const double y2, const double z2, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkCellArray> cells)
{
  vtkSmartPointer<vtkPolyLine> line;
  line = vtkSmartPointer<vtkPolyLine>::New();
  line->GetPointIds()->SetNumberOfIds(2);

  vtkIdType id_1, id_2;
  id_1 = points->InsertNextPoint(x1, y1, z1);
  id_2 = points->InsertNextPoint(x2, y2, z2);

  line->GetPointIds()->SetId(0, id_1);
  line->GetPointIds()->SetId(1, id_2);

  cells->InsertNextCell(line);
}

void QVtkRenderer::addModelActor(const std::shared_ptr<Model> model)
{
  m_renderer->AddActor(model->getModelActor());

  qDebug() << "QVtkRenderer::addModelActor(): Model added " << model.get();
}

void QVtkRenderer::selectModel(const int16_t x, const int16_t y)
{
  qDebug() << "QVtkRenderer::selectModel()";

  // Compensate the y-axis flip for the picking
  m_picker->Pick(x, m_renderer->GetSize()[1] - y, 0, m_renderer);

  // Get pick position
  double clickPosition[3];
  m_picker->GetPickPosition(clickPosition);
  m_clickPositionZ = clickPosition[2];

  if (m_selectedActor == m_picker->GetActor())
    {
      if (m_selectedModel)
	{
	  m_selectedModel->setMouseDeltaXY(clickPosition[0] - m_selectedModel->getPositionX(), clickPosition[1] - m_selectedModel->getPositionY());
	}
      return;
    }

  // Disconnect signals
  if (m_selectedModel)
    {
      this->clearSelectedModel();
    }

  // Pick the new actor
  m_selectedActor = m_picker->GetActor();

  m_selectedModel = this->getSelectedModelNoLock();

  if (m_selectedActor)
    {
      qDebug() << "QVtkRenderer::selectModel(): picked actor" << m_selectedActor;

      m_selectedModel->setSelected(true);

      // Connect signals
      connect(m_selectedModel.get(), &Model::positionXChanged, this, &QVtkRenderer::setSelectedModelPositionX);
      connect(m_selectedModel.get(), &Model::positionYChanged, this, &QVtkRenderer::setSelectedModelPositionY);

      this->setSelectedModelPositionX(m_selectedModel->getPositionX());
      this->setSelectedModelPositionY(m_selectedModel->getPositionY());

      this->setIsModelSelected(true);

      // Set mouse click delta from center position
      m_selectedModel->setMouseDeltaXY(clickPosition[0] - m_selectedModel->getPositionX(), clickPosition[1] - m_selectedModel->getPositionY());
    }
  else
    {
      this->setIsModelSelected(false);
    }

  qDebug() << "QVtkRenderer::selectModel() end";
}

void QVtkRenderer::clearSelectedModel()
{
  m_selectedModel->setSelected(false);

  disconnect(m_selectedModel.get(), &Model::positionXChanged, this, &QVtkRenderer::setSelectedModelPositionX);
  disconnect(m_selectedModel.get(), &Model::positionYChanged, this, &QVtkRenderer::setSelectedModelPositionY);

  m_selectedModel = nullptr;
  m_selectedActor = nullptr;
}

void QVtkRenderer::setIsModelSelected(const bool isModelSelected)
{
  if (m_isModelSelected != isModelSelected)
    {
      qDebug() << "QVtkRenderer::setIsModelSelected():" << isModelSelected;
      m_isModelSelected = isModelSelected;
      emit isModelSelectedChanged();
    }
}

bool QVtkRenderer::isModelSelected() const
{
  return m_isModelSelected;
}

std::shared_ptr<Model> QVtkRenderer::getSelectedModel() const
{
  std::shared_ptr<Model> selectedModel = this->getSelectedModelNoLock();

  return selectedModel;
}

std::shared_ptr<Model> QVtkRenderer::getSelectedModelNoLock() const
{
  return m_processingEngine->getModelFromActor(m_selectedActor);
}

void QVtkRenderer::setSelectedModelPositionX(const double positionX)
{
  if (m_selectedModelPositionX != positionX)
    {
      m_selectedModelPositionX = positionX;
      emit selectedModelPositionXChanged();
    }
}

void QVtkRenderer::setSelectedModelPositionY(const double positionY)
{
  if (m_selectedModelPositionY != positionY)
    {
      m_selectedModelPositionY = positionY;
      emit selectedModelPositionYChanged();
    }
}

double QVtkRenderer::getSelectedModelPositionX() const
{
  return m_selectedModelPositionX;
}

double QVtkRenderer::getSelectedModelPositionY() const
{
  return m_selectedModelPositionY;
}

const bool QVtkRenderer::screenToWorld(const int16_t screenX, const int16_t screenY, double worldPos[])
{
  //Create bounding planes for projection plane
  vtkSmartPointer<vtkPlane> boundingPlanes[4];

  boundingPlanes[0] = vtkSmartPointer<vtkPlane>::New();
  boundingPlanes[0]->SetOrigin(0.0, 1000.0, 0.0);
  boundingPlanes[0]->SetNormal(0.0, -1.0, 0.0);

  boundingPlanes[1] = vtkSmartPointer<vtkPlane>::New();
  boundingPlanes[1]->SetOrigin(0.0, -1000.0, 0.0);
  boundingPlanes[1]->SetNormal(0.0, 1.0, 0.0);

  boundingPlanes[2] = vtkSmartPointer<vtkPlane>::New();
  boundingPlanes[2]->SetOrigin(1000.0, 0.0, 0.0);
  boundingPlanes[2]->SetNormal(-1.0, 0.0, 0.0);

  boundingPlanes[3] = vtkSmartPointer<vtkPlane>::New();
  boundingPlanes[3]->SetOrigin(-1000.0, 0.0, 0.0);
  boundingPlanes[3]->SetNormal(1.0, 0.0, 0.0);

  // Create projection plane parallel platform and Z coordinate from clicked position in model
  vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
  plane->SetOrigin(0.0, 0.0, m_clickPositionZ);
  plane->SetNormal(0.0, 0.0, 1.0);

  // Set projection and bounding planes to placer
  vtkSmartPointer<vtkBoundedPlanePointPlacer> placer = vtkSmartPointer<vtkBoundedPlanePointPlacer>::New();
  placer->SetObliquePlane(plane);
  placer->SetProjectionNormalToOblique();

  placer->AddBoundingPlane(boundingPlanes[0].Get());
  placer->AddBoundingPlane(boundingPlanes[1].Get());
  placer->AddBoundingPlane(boundingPlanes[2].Get());
  placer->AddBoundingPlane(boundingPlanes[3].Get());

  double screenPos[2];
  double worldOrient[9];

  screenPos[0] = screenX;
  // Compensate the y-axis flip for the picking
  screenPos[1] = m_renderer->GetSize()[1] - screenY;

  int16_t withinBounds;
  withinBounds = placer->ComputeWorldPosition(m_renderer, screenPos, worldPos, worldOrient);

  return withinBounds;
}

void QVtkRenderer::resetCamera()
{
  // Seting the clipping range here messes with the opacity of the actors prior to moving the camera
  m_camPositionX = -237.885;
  m_camPositionY = -392.348;
  m_camPositionZ = 369.477;
  m_renderer->GetActiveCamera()->SetPosition(m_camPositionX, m_camPositionY, m_camPositionZ);
  m_renderer->GetActiveCamera()->SetFocalPoint(0.0, 0.0, 0.0);
  m_renderer->GetActiveCamera()->SetViewUp(0.0, 0.0, 1.0);
  m_renderer->ResetCameraClippingRange();
}
