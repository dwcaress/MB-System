#include "CommandModel.h"
#include "CommandModelAdd.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include "QVTKFramebufferObjectItem.h"
#include "QVTKFramebufferObjectRenderer.h"


QVTKFramebufferObjectItem::QVTKFramebufferObjectItem()
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseMove = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseWheel = std::make_shared<QWheelEvent>(QPointF(0,0), 0, Qt::NoButton, Qt::NoModifier, Qt::Vertical);

	this->setMirrorVertically(true); // QtQuick and OpenGL have opposite Y-Axis directions

	setAcceptedMouseButtons(Qt::RightButton);
}


QQuickFramebufferObject::Renderer *QVTKFramebufferObjectItem::createRenderer() const
{
	return new QVTKFramebufferObjectRenderer();
}

void QVTKFramebufferObjectItem::setVtkFboRenderer(QVTKFramebufferObjectRenderer* renderer)
{
	qDebug() << "QVTKFramebufferObjectItem::setVtkFboRenderer";

	m_vtkFboRenderer = renderer;

	connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::isModelSelectedChanged, this, &QVTKFramebufferObjectItem::isModelSelectedChanged);
	connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionXChanged, this, &QVTKFramebufferObjectItem::selectedModelPositionXChanged);
	connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionYChanged, this, &QVTKFramebufferObjectItem::selectedModelPositionYChanged);

	m_vtkFboRenderer->setProcessingEngine(m_processingEngine);
}

bool QVTKFramebufferObjectItem::isInitialized() const
{
	return (m_vtkFboRenderer != nullptr);
}

void QVTKFramebufferObjectItem::setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine)
{
	m_processingEngine = std::shared_ptr<ProcessingEngine>(processingEngine);
}


// Model releated functions

bool QVTKFramebufferObjectItem::isModelSelected() const
{
	return m_vtkFboRenderer->isModelSelected();
}

double QVTKFramebufferObjectItem::getSelectedModelPositionX() const
{
	return m_vtkFboRenderer->getSelectedModelPositionX();
}

double QVTKFramebufferObjectItem::getSelectedModelPositionY() const
{
	return m_vtkFboRenderer->getSelectedModelPositionY();
}


void QVTKFramebufferObjectItem::selectModel(const int screenX, const int screenY)
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(screenX, screenY), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	m_lastMouseLeftButton->ignore();

	update();
}

void QVTKFramebufferObjectItem::resetModelSelection()
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	m_lastMouseLeftButton->ignore();

	update();
}

void QVTKFramebufferObjectItem::addModelFromFile(const QUrl &modelPath)
{
	qDebug() << "QVTKFramebufferObjectItem::addModelFromFile";

	CommandModelAdd *command = new CommandModelAdd(m_vtkFboRenderer, m_processingEngine, modelPath);

	connect(command, &CommandModelAdd::ready, this, &QVTKFramebufferObjectItem::update);
	connect(command, &CommandModelAdd::done, this, &QVTKFramebufferObjectItem::addModelFromFileDone);

	command->start();

	this->addCommand(command);
}

void QVTKFramebufferObjectItem::translateModel(CommandModelTranslate::TranslateParams_t & translateData, const bool inTransition)
{
	if (translateData.model == nullptr)
	{
		// If no model selected yet, try to select one
		translateData.model = m_vtkFboRenderer->getSelectedModel();

		if (translateData.model == nullptr)
		{
			return;
		}
	}

	this->addCommand(new CommandModelTranslate(m_vtkFboRenderer, translateData, inTransition));
}


void QVTKFramebufferObjectItem::addCommand(CommandModel *command)
{
	m_commandsQueueMutex.lock();
	m_commandsQueue.push(command);
	m_commandsQueueMutex.unlock();

	update();
}


// Camera related functions

void QVTKFramebufferObjectItem::wheelEvent(QWheelEvent *e)
{
	m_lastMouseWheel = std::make_shared<QWheelEvent>(*e);
	m_lastMouseWheel->ignore();
	e->accept();
	update();
}

void QVTKFramebufferObjectItem::mousePressEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::RightButton)
	{
		m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
		m_lastMouseButton->ignore();
		e->accept();
		update();
	}
}

void QVTKFramebufferObjectItem::mouseReleaseEvent(QMouseEvent *e)
{
	m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
	m_lastMouseButton->ignore();
	e->accept();
	update();
}

void QVTKFramebufferObjectItem::mouseMoveEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::RightButton)
	{
		*m_lastMouseMove = *e;
		m_lastMouseMove->ignore();
		e->accept();
		update();
	}
}


QMouseEvent *QVTKFramebufferObjectItem::getLastMouseLeftButton()
{
	return m_lastMouseLeftButton.get();
}

QMouseEvent *QVTKFramebufferObjectItem::getLastMouseButton()
{
	return m_lastMouseButton.get();
}

QMouseEvent *QVTKFramebufferObjectItem::getLastMoveEvent()
{
	return m_lastMouseMove.get();
}

QWheelEvent *QVTKFramebufferObjectItem::getLastWheelEvent()
{
	return m_lastMouseWheel.get();
}


void QVTKFramebufferObjectItem::resetCamera()
{
	m_vtkFboRenderer->resetCamera();
	update();
}

int QVTKFramebufferObjectItem::getModelsRepresentation() const
{
	return m_modelsRepresentationOption;
}

double QVTKFramebufferObjectItem::getModelsOpacity() const
{
	return m_modelsOpacity;
}

bool QVTKFramebufferObjectItem::getGourauInterpolation() const
{
	return m_gouraudInterpolation;
}

int QVTKFramebufferObjectItem::getModelColorR() const
{
	return m_modelColorR;
}

int QVTKFramebufferObjectItem::getModelColorG() const
{
	return m_modelColorG;
}

int QVTKFramebufferObjectItem::getModelColorB() const
{
	return m_modelColorB;
}

void QVTKFramebufferObjectItem::setModelsRepresentation(const int representationOption)
{
	if (m_modelsRepresentationOption != representationOption)
	{
		m_modelsRepresentationOption = representationOption;
		update();
	}
}

void QVTKFramebufferObjectItem::setModelsOpacity(const double opacity)
{
	if (m_modelsOpacity != opacity)
	{
		m_modelsOpacity = opacity;
		update();
	}
}

void QVTKFramebufferObjectItem::setGouraudInterpolation(const bool gouraudInterpolation)
{
	if (m_gouraudInterpolation != gouraudInterpolation)
	{
		m_gouraudInterpolation = gouraudInterpolation;
		update();
	}
}

void QVTKFramebufferObjectItem::setModelColorR(const int colorR)
{
	if (m_modelColorR != colorR)
	{
		m_modelColorR = colorR;
		update();
	}
}

void QVTKFramebufferObjectItem::setModelColorG(const int colorG)
{
	if (m_modelColorG != colorG)
	{
		m_modelColorG = colorG;
		update();
	}
}

void QVTKFramebufferObjectItem::setModelColorB(const int colorB)
{
	if (m_modelColorB != colorB)
	{
		m_modelColorB = colorB;
		update();
	}
}

CommandModel *QVTKFramebufferObjectItem::getCommandsQueueFront() const
{
	return m_commandsQueue.front();
}

void QVTKFramebufferObjectItem::commandsQueuePop()
{
	m_commandsQueue.pop();
}

bool QVTKFramebufferObjectItem::isCommandsQueueEmpty() const
{
	return m_commandsQueue.empty();
}

void QVTKFramebufferObjectItem::lockCommandsQueueMutex()
{
	m_commandsQueueMutex.lock();
}

void QVTKFramebufferObjectItem::unlockCommandsQueueMutex()
{
	m_commandsQueueMutex.unlock();
}

