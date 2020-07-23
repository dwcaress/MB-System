#ifndef QVTKFRAMEBUFFEROBJECTITEM_H
#define QVTKFRAMEBUFFEROBJECTITEM_H

#include <memory>
#include <queue>
#include <mutex>

#include <QtQuick/QQuickFramebufferObject>

#include "CommandModelTranslate.h"


class CommandModel;
class Model;
class ProcessingEngine;
class QVTKFramebufferObjectRenderer;

/**
Captures and processes mouse events from canvas 

Based on Nicano Romero Venier's QtVTK demo at 
https://github.com/nicanor-romero/QtVtk
 */
class QVTKFramebufferObjectItem : public QQuickFramebufferObject
{
	Q_OBJECT

public:
	QVTKFramebufferObjectItem();

	Renderer *createRenderer() const Q_DECL_OVERRIDE;
	void setVtkFboRenderer(QVTKFramebufferObjectRenderer*);
	bool isInitialized() const;
	void setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine);

	// Model releated functions
	bool isModelSelected() const;

	double getSelectedModelPositionX() const;
	double getSelectedModelPositionY() const;

	void selectModel(const int screenX, const int screenY);
	void resetModelSelection();
	void addModelFromFile(const QUrl &modelPath);

	void translateModel(CommandModelTranslate::TranslateParams_t &translateData, const bool inTransition);

	// Camera related functions
	void wheelEvent(QWheelEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;

	QMouseEvent *getLastMouseLeftButton();
	QMouseEvent *getLastMouseButton();
	QMouseEvent *getLastMoveEvent();
	QWheelEvent *getLastWheelEvent();

	void resetCamera();

	int getModelsRepresentation() const;
	double getModelsOpacity() const;
	bool getGourauInterpolation() const;
	int getModelColorR() const;
	int getModelColorG() const;
	int getModelColorB() const;

	void setModelsRepresentation(const int representationOption);
	void setModelsOpacity(const double opacity);
	void setGouraudInterpolation(const bool gouraudInterpolation);
	void setModelColorR(const int colorR);
	void setModelColorG(const int colorG);
	void setModelColorB(const int colorB);

	CommandModel* getCommandsQueueFront() const;
	void commandsQueuePop();
	bool isCommandsQueueEmpty() const;
	void lockCommandsQueueMutex();
	void unlockCommandsQueueMutex();

signals:
	void rendererInitialized();

	void isModelSelectedChanged();
	void selectedModelPositionXChanged();
	void selectedModelPositionYChanged();

	void addModelFromFileDone();
	void addModelFromFileError(QString error);

private:
	void addCommand(CommandModel* command);

	QVTKFramebufferObjectRenderer *m_vtkFboRenderer = nullptr;
	std::shared_ptr<ProcessingEngine> m_processingEngine;

	std::queue<CommandModel*> m_commandsQueue;
	std::mutex m_commandsQueueMutex;

	std::shared_ptr<QMouseEvent> m_lastMouseLeftButton;
	std::shared_ptr<QMouseEvent> m_lastMouseButton;
	std::shared_ptr<QMouseEvent> m_lastMouseMove;
	std::shared_ptr<QWheelEvent> m_lastMouseWheel;

	int m_modelsRepresentationOption = 2;
	double m_modelsOpacity = 1.0;
	bool m_gouraudInterpolation = false;
	int m_modelColorR = 3;
	int m_modelColorG = 169;
	int m_modelColorB = 244;
};

#endif // QVTKFRAMEBUFFEROBJECTITEM_H
