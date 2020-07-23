#include "CommandModel.h"
#include "CommandModelAdd.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include "QVtkItem.h"
#include "QVtkRenderer.h"


QVtkItem::QVtkItem()
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseMove = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	m_lastMouseWheel = std::make_shared<QWheelEvent>(QPointF(0,0), 0, Qt::NoButton, Qt::NoModifier, Qt::Vertical);

	this->setMirrorVertically(true); // QtQuick and OpenGL have opposite Y-Axis directions

	setAcceptedMouseButtons(Qt::RightButton);
}


QQuickFramebufferObject::Renderer *QVtkItem::createRenderer() const
{
	return new QVtkRenderer();
}

void QVtkItem::setVtkFboRenderer(QVtkRenderer* renderer)
{
	qDebug() << "QVtkItem::setVtkFboRenderer";

	m_vtkFboRenderer = renderer;

	connect(m_vtkFboRenderer, &QVtkRenderer::isModelSelectedChanged, this, &QVtkItem::isModelSelectedChanged);
	connect(m_vtkFboRenderer, &QVtkRenderer::selectedModelPositionXChanged, this, &QVtkItem::selectedModelPositionXChanged);
	connect(m_vtkFboRenderer, &QVtkRenderer::selectedModelPositionYChanged, this, &QVtkItem::selectedModelPositionYChanged);

	m_vtkFboRenderer->setProcessingEngine(m_processingEngine);
}

bool QVtkItem::isInitialized() const
{
	return (m_vtkFboRenderer != nullptr);
}

void QVtkItem::setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine)
{
	m_processingEngine = std::shared_ptr<ProcessingEngine>(processingEngine);
}


// Model releated functions

bool QVtkItem::isModelSelected() const
{
	return m_vtkFboRenderer->isModelSelected();
}

double QVtkItem::getSelectedModelPositionX() const
{
	return m_vtkFboRenderer->getSelectedModelPositionX();
}

double QVtkItem::getSelectedModelPositionY() const
{
	return m_vtkFboRenderer->getSelectedModelPositionY();
}


void QVtkItem::selectModel(const int screenX, const int screenY)
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(screenX, screenY), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	m_lastMouseLeftButton->ignore();

	update();
}

void QVtkItem::resetModelSelection()
{
	m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	m_lastMouseLeftButton->ignore();

	update();
}

void QVtkItem::addModelFromFile(const QUrl &modelPath)
{
	qDebug() << "QVtkItem::addModelFromFile";

	CommandModelAdd *command = new CommandModelAdd(m_vtkFboRenderer, m_processingEngine, modelPath);

	connect(command, &CommandModelAdd::ready, this, &QVtkItem::update);
	connect(command, &CommandModelAdd::done, this, &QVtkItem::addModelFromFileDone);

	command->start();

	this->addCommand(command);
}

void QVtkItem::translateModel(CommandModelTranslate::TranslateParams_t & translateData, const bool inTransition)
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


void QVtkItem::addCommand(CommandModel *command)
{
	m_commandsQueueMutex.lock();
	m_commandsQueue.push(command);
	m_commandsQueueMutex.unlock();

	update();
}


// Camera related functions

void QVtkItem::wheelEvent(QWheelEvent *e)
{
	m_lastMouseWheel = std::make_shared<QWheelEvent>(*e);
	m_lastMouseWheel->ignore();
	e->accept();
	update();
}

void QVtkItem::mousePressEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::RightButton)
	{
		m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
		m_lastMouseButton->ignore();
		e->accept();
		update();
	}
}

void QVtkItem::mouseReleaseEvent(QMouseEvent *e)
{
	m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
	m_lastMouseButton->ignore();
	e->accept();
	update();
}

void QVtkItem::mouseMoveEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::RightButton)
	{
		*m_lastMouseMove = *e;
		m_lastMouseMove->ignore();
		e->accept();
		update();
	}
}


QMouseEvent *QVtkItem::getLastMouseLeftButton()
{
	return m_lastMouseLeftButton.get();
}

QMouseEvent *QVtkItem::getLastMouseButton()
{
	return m_lastMouseButton.get();
}

QMouseEvent *QVtkItem::getLastMoveEvent()
{
	return m_lastMouseMove.get();
}

QWheelEvent *QVtkItem::getLastWheelEvent()
{
	return m_lastMouseWheel.get();
}


void QVtkItem::resetCamera()
{
	m_vtkFboRenderer->resetCamera();
	update();
}

int QVtkItem::getModelsRepresentation() const
{
	return m_modelsRepresentationOption;
}

double QVtkItem::getModelsOpacity() const
{
	return m_modelsOpacity;
}

bool QVtkItem::getGourauInterpolation() const
{
	return m_gouraudInterpolation;
}

int QVtkItem::getModelColorR() const
{
	return m_modelColorR;
}

int QVtkItem::getModelColorG() const
{
	return m_modelColorG;
}

int QVtkItem::getModelColorB() const
{
	return m_modelColorB;
}

void QVtkItem::setModelsRepresentation(const int representationOption)
{
	if (m_modelsRepresentationOption != representationOption)
	{
		m_modelsRepresentationOption = representationOption;
		update();
	}
}

void QVtkItem::setModelsOpacity(const double opacity)
{
	if (m_modelsOpacity != opacity)
	{
		m_modelsOpacity = opacity;
		update();
	}
}

void QVtkItem::setGouraudInterpolation(const bool gouraudInterpolation)
{
	if (m_gouraudInterpolation != gouraudInterpolation)
	{
		m_gouraudInterpolation = gouraudInterpolation;
		update();
	}
}

void QVtkItem::setModelColorR(const int colorR)
{
	if (m_modelColorR != colorR)
	{
		m_modelColorR = colorR;
		update();
	}
}

void QVtkItem::setModelColorG(const int colorG)
{
	if (m_modelColorG != colorG)
	{
		m_modelColorG = colorG;
		update();
	}
}

void QVtkItem::setModelColorB(const int colorB)
{
	if (m_modelColorB != colorB)
	{
		m_modelColorB = colorB;
		update();
	}
}

CommandModel *QVtkItem::getCommandsQueueFront() const
{
	return m_commandsQueue.front();
}

void QVtkItem::commandsQueuePop()
{
	m_commandsQueue.pop();
}

bool QVtkItem::isCommandsQueueEmpty() const
{
	return m_commandsQueue.empty();
}

void QVtkItem::lockCommandsQueueMutex()
{
	m_commandsQueueMutex.lock();
}

void QVtkItem::unlockCommandsQueueMutex()
{
	m_commandsQueueMutex.unlock();
}

