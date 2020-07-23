#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "Model.h"
#include "ProcessingEngine.h"
#include "QVTKFramebufferObjectItem.h"
#include "QVTKFramebufferObjectRenderer.h"
#include "CanvasHandler.h"


CanvasHandler::CanvasHandler(int argc, char **argv)
{
	QApplication app(argc, argv);
	QQmlApplicationEngine engine;

	app.setApplicationName("QtVTK");
	app.setWindowIcon(QIcon(":/resources/bq.ico"));

	// Register QML types
	qmlRegisterType<QVTKFramebufferObjectItem>("QtVTK", 1, 0, "VtkFboItem");

	// Create classes instances
	m_processingEngine = std::shared_ptr<ProcessingEngine>(new ProcessingEngine());

	// Expose C++ classes to QML
	QQmlContext* ctxt = engine.rootContext();

	ctxt->setContextProperty("canvasHandler", this);

	QQuickStyle::setStyle("Material");

	// Load main QML file
	engine.load(QUrl("qrc:/main.qml"));

	// Get reference to the QVTKFramebufferObjectItem created in QML
	// We cannot use smart pointers because this object must be deleted by QML
	QObject *rootObject = engine.rootObjects().first();
	m_vtkFboItem = rootObject->findChild<QVTKFramebufferObjectItem*>("vtkFboItem");

	// Give the vtkFboItem reference to the CanvasHandler
	if (m_vtkFboItem)
	{
		qDebug() << "CanvasHandler::CanvasHandler: setting vtkFboItem to CanvasHandler";

		m_vtkFboItem->setProcessingEngine(m_processingEngine);

		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::rendererInitialized, this, &CanvasHandler::startApplication);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::isModelSelectedChanged, this, &CanvasHandler::isModelSelectedChanged);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionXChanged, this, &CanvasHandler::selectedModelPositionXChanged);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionYChanged, this, &CanvasHandler::selectedModelPositionYChanged);
	}
	else
	{
		qCritical() << "CanvasHandler::CanvasHandler: Unable to get vtkFboItem instance";
		return;
	}

	int rc = app.exec();

	qDebug() << "CanvasHandler::CanvasHandler: Execution finished with return code:" << rc;
}


void CanvasHandler::startApplication() const
{
	qDebug() << "CanvasHandler::startApplication()";

	disconnect(m_vtkFboItem, &QVTKFramebufferObjectItem::rendererInitialized, this, &CanvasHandler::startApplication);
}


void CanvasHandler::openModel(const QUrl &path) const
{
	qDebug() << "CanvasHandler::openModel():" << path;

	QUrl localFilePath;

	if (path.isLocalFile())
	{
		// Remove the "file:///" if present
		localFilePath = path.toLocalFile();
	}
	else
	{
		localFilePath = path;
	}

	m_vtkFboItem->addModelFromFile(localFilePath);
}

void CanvasHandler::mousePressEvent(const int button, const int screenX, const int screenY) const
{
	qDebug() << "CanvasHandler::mousePressEvent()";

	m_vtkFboItem->selectModel(screenX, screenY);
}

void CanvasHandler::mouseMoveEvent(const int button, const int screenX, const int screenY)
{
	if (!m_vtkFboItem->isModelSelected())
	{
		return;
	}

	if (!m_draggingMouse)
	{
		m_draggingMouse = true;

		m_previousWorldX = m_vtkFboItem->getSelectedModelPositionX();
		m_previousWorldY = m_vtkFboItem->getSelectedModelPositionY();
	}

	CommandModelTranslate::TranslateParams_t translateParams;

	translateParams.screenX = screenX;
	translateParams.screenY = screenY;

	m_vtkFboItem->translateModel(translateParams, true);
}

void CanvasHandler::mouseReleaseEvent(const int button, const int screenX, const int screenY)
{
	qDebug() << "CanvasHandler::mouseReleaseEvent()";

	if (!m_vtkFboItem->isModelSelected())
	{
		return;
	}

	if (m_draggingMouse)
	{
		m_draggingMouse = false;

		CommandModelTranslate::TranslateParams_t translateParams;

		translateParams.screenX = screenX;
		translateParams.screenY = screenY;
		translateParams.previousPositionX = m_previousWorldX;
		translateParams.previousPositionY = m_previousWorldY;

		m_vtkFboItem->translateModel(translateParams, false);
	}
}


bool CanvasHandler::getIsModelSelected() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->isModelSelected();
}

double CanvasHandler::getSelectedModelPositionX() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->getSelectedModelPositionX();
}

double CanvasHandler::getSelectedModelPositionY() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->getSelectedModelPositionY();
}

void CanvasHandler::setModelsRepresentation(const int representationOption)
{
	m_vtkFboItem->setModelsRepresentation(representationOption);
}

void CanvasHandler::setModelsOpacity(const double opacity)
{
	m_vtkFboItem->setModelsOpacity(opacity);
}

void CanvasHandler::setGouraudInterpolation(const bool gouraudInterpolation)
{
	m_vtkFboItem->setGouraudInterpolation(gouraudInterpolation);
}

void CanvasHandler::setModelColorR(const int colorR)
{
	m_vtkFboItem->setModelColorR(colorR);
}

void CanvasHandler::setModelColorG(const int colorG)
{
	m_vtkFboItem->setModelColorG(colorG);
}

void CanvasHandler::setModelColorB(const int colorB)
{
	m_vtkFboItem->setModelColorB(colorB);
}
