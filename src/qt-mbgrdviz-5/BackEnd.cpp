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
#include "BackEnd.h"


BackEnd::BackEnd(int argc, char **argv)
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

	// Give the vtkFboItem reference to the BackEnd
	if (m_vtkFboItem)
	{
		qDebug() << "BackEnd::BackEnd: setting vtkFboItem to BackEnd";

		m_vtkFboItem->setProcessingEngine(m_processingEngine);

		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::rendererInitialized, this, &BackEnd::startApplication);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::isModelSelectedChanged, this, &BackEnd::isModelSelectedChanged);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionXChanged, this, &BackEnd::selectedModelPositionXChanged);
		connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionYChanged, this, &BackEnd::selectedModelPositionYChanged);
	}
	else
	{
		qCritical() << "BackEnd::BackEnd: Unable to get vtkFboItem instance";
		return;
	}

	int rc = app.exec();

	qDebug() << "BackEnd::BackEnd: Execution finished with return code:" << rc;
}


void BackEnd::startApplication() const
{
	qDebug() << "BackEnd::startApplication()";

	disconnect(m_vtkFboItem, &QVTKFramebufferObjectItem::rendererInitialized, this, &BackEnd::startApplication);
}


void BackEnd::openModel(const QUrl &path) const
{
	qDebug() << "BackEnd::openModel():" << path;

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

void BackEnd::mousePressEvent(const int button, const int screenX, const int screenY) const
{
	qDebug() << "BackEnd::mousePressEvent()";

	m_vtkFboItem->selectModel(screenX, screenY);
}

void BackEnd::mouseMoveEvent(const int button, const int screenX, const int screenY)
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

void BackEnd::mouseReleaseEvent(const int button, const int screenX, const int screenY)
{
	qDebug() << "BackEnd::mouseReleaseEvent()";

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


bool BackEnd::getIsModelSelected() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->isModelSelected();
}

double BackEnd::getSelectedModelPositionX() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->getSelectedModelPositionX();
}

double BackEnd::getSelectedModelPositionY() const
{
	// QVTKFramebufferObjectItem might not be initialized when QML loads
	if (!m_vtkFboItem)
	{
		return 0;
	}

	return m_vtkFboItem->getSelectedModelPositionY();
}

void BackEnd::setModelsRepresentation(const int representationOption)
{
	m_vtkFboItem->setModelsRepresentation(representationOption);
}

void BackEnd::setModelsOpacity(const double opacity)
{
	m_vtkFboItem->setModelsOpacity(opacity);
}

void BackEnd::setGouraudInterpolation(const bool gouraudInterpolation)
{
	m_vtkFboItem->setGouraudInterpolation(gouraudInterpolation);
}

void BackEnd::setModelColorR(const int colorR)
{
	m_vtkFboItem->setModelColorR(colorR);
}

void BackEnd::setModelColorG(const int colorG)
{
	m_vtkFboItem->setModelColorG(colorG);
}

void BackEnd::setModelColorB(const int colorB)
{
	m_vtkFboItem->setModelColorB(colorB);
}
