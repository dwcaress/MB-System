#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <mutex>

#include <QObject>
#include <QColor>

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>


/** 
 */
class Model : public QObject
{
	Q_OBJECT

public:
	Model(vtkSmartPointer<vtkPolyData> modelData);

	const vtkSmartPointer<vtkActor>& getModelActor() const;

	double getPositionX();
	double getPositionY();

	void translateToPosition(const double x, const double y);

	void setSelected(const bool selected);
	static void setSelectedModelColor(const QColor &selectedModelColor);

	const double getMouseDeltaX() const;
	const double getMouseDeltaY() const;
	void setMouseDeltaXY(const double deltaX, const double deltaY);

	void updateModelColor();

signals:
	void positionXChanged(const double positionX);
	void positionYChanged(const double positionY);

private:
	void setPositionX(const double positionX);
	void setPositionY(const double positionY);

	void setColor(const QColor &color);

	static QColor m_defaultModelColor;
	static QColor m_selectedModelColor;

	vtkSmartPointer<vtkPolyData> m_modelData;
	vtkSmartPointer<vtkPolyDataMapper> m_modelMapper;
	vtkSmartPointer<vtkActor> m_modelActor;

	vtkSmartPointer<vtkTransformPolyDataFilter> m_modelFilterTranslate;

	std::mutex m_propertiesMutex;

	double m_positionX {0.0};
	double m_positionY {0.0};
	double m_positionZ {0.0};

	bool m_selected = false;

	double m_mouseDeltaX = 0.0;
	double m_mouseDeltaY = 0.0;
};

#endif // MODEL_H
