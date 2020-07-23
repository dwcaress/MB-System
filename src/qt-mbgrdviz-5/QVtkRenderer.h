#ifndef QVTKFRAMEBUFFEROBJECTRENDERER_H
#define QVTKFRAMEBUFFEROBJECTRENDERER_H

#include <memory>
#include <vector>
#include <mutex>

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickFramebufferObject>
#include <QUndoStack>
#include <QDir>

#include <vtkActor.h>
#include <vtkCellPicker.h>
#include <vtkCubeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkObject.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

class Model;
class QVtkItem;
class ProcessingEngine;

/**
Based on Nicano Romero Venier's QtVTK demo at 
https://github.com/nicanor-romero/QtVtk
 */
class QVtkRenderer : public QObject, public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	QVtkRenderer();

	void setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine);

	virtual void synchronize(QQuickFramebufferObject *item);
	virtual void render();
	virtual void openGLInitState();
	QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);

	void addModelActor(const std::shared_ptr<Model> model);

	std::shared_ptr<Model> getSelectedModel() const;
	bool isModelSelected() const;

	void setSelectedModelPositionX(const double positionX);
	void setSelectedModelPositionY(const double positionY);

	double getSelectedModelPositionX() const;
	double getSelectedModelPositionY() const;

	void resetCamera();
	const bool screenToWorld(const int16_t screenX, const int16_t screenY, double worldPos[]);

signals:
	void isModelSelectedChanged();

	void selectedModelPositionXChanged();
	void selectedModelPositionYChanged();

private:
	void initScene();
	void generatePlatform();
	void updatePlatform();

	void selectModel(const int16_t x, const int16_t y);
	void clearSelectedModel();
	void setIsModelSelected(const bool isModelSelected);

	void createLine(const double x1, const double y1, const double z1, const double x2, const double y2, const double z2, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkCellArray> cells);
	std::shared_ptr<Model> getSelectedModelNoLock() const;

	std::shared_ptr<ProcessingEngine> m_processingEngine;
	QVtkItem *m_vtkFboItem = nullptr;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_vtkRenderWindow;
	vtkSmartPointer<vtkRenderer> m_renderer;
	vtkSmartPointer<vtkGenericRenderWindowInteractor> m_vtkRenderWindowInteractor;

	vtkSmartPointer<vtkCellPicker> m_picker;

	std::shared_ptr<Model> m_selectedModel = nullptr;
	vtkSmartPointer<vtkActor> m_selectedActor = nullptr;
	bool m_isModelSelected = false;

	double m_selectedModelPositionX = 0.0;
	double m_selectedModelPositionY = 0.0;

	std::shared_ptr<QMouseEvent> m_mouseLeftButton = nullptr;
	std::shared_ptr<QMouseEvent> m_mouseEvent = nullptr;
	std::shared_ptr<QMouseEvent> m_moveEvent = nullptr;
	std::shared_ptr<QWheelEvent> m_wheelEvent = nullptr;

	vtkSmartPointer<vtkCubeSource> m_platformModel;
	vtkSmartPointer<vtkPolyData> m_platformGrid;
	vtkSmartPointer<vtkActor> m_platformModelActor;
	vtkSmartPointer<vtkActor> m_platformGridActor;

	double m_platformWidth = 200.0;
	double m_platformDepth = 200.0;
	double m_platformHeight = 200.0;
	double m_platformThickness = 2.0;
	double m_gridBottomHeight = 0.15;
	uint16_t m_gridSize = 10;

	double m_camPositionX;
	double m_camPositionY;
	double m_camPositionZ;

	double m_clickPositionZ = 0.0;

	bool m_firstRender = true;

	int m_modelsRepresentationOption = 0;
	double m_modelsOpacity = 1.0;
	bool m_modelsGouraudInterpolation = false;
};

#endif // QVTKFRAMEBUFFEROBJECTRENDERER_H
