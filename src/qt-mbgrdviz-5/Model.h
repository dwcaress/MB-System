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
#include <vtkElevationFilter.h>


/** 
 */
class Model : public QObject
{
  Q_OBJECT

public:
  Model(vtkSmartPointer<vtkPolyData> modelData);

  const vtkSmartPointer<vtkActor>& getActor() const;

  double getPositionX();
  double getPositionY();

  void translateToPosition(const double x, const double y);

  void setSelected(const bool selected);
  static void setSelectedColor(const QColor &selectedColor);

  const double getMouseDeltaX() const;
  const double getMouseDeltaY() const;
  void setMouseDeltaXY(const double deltaX, const double deltaY);

  void updateColor();

signals:
  void positionXChanged(const double positionX);
  void positionYChanged(const double positionY);

private:
  void setPositionX(const double positionX);
  void setPositionY(const double positionY);

  void setColor(const QColor &color);

  static QColor m_defaultColor;
  static QColor m_selectedColor;

  vtkSmartPointer<vtkPolyData> m_polyData;
  vtkSmartPointer<vtkPolyDataMapper> m_mapper;
  vtkSmartPointer<vtkActor> m_actor;

  vtkSmartPointer<vtkTransformPolyDataFilter> m_filterTranslate;
  vtkSmartPointer<vtkElevationFilter> m_filterElevation;

  std::mutex m_propertiesMutex;

  double m_positionX {0.0};
  double m_positionY {0.0};
  double m_positionZ {0.0};

  bool m_selected = false;

  double m_mouseDeltaX = 0.0;
  double m_mouseDeltaY = 0.0;
};

#endif // MODEL_H
