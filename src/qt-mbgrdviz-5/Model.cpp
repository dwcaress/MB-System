#include <QDebug>

#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkElevationFilter.h>

#include "Model.h"


QColor Model::m_defaultColor = QColor{"#0277bd"};
QColor Model::m_selectedColor = QColor{"#03a9f4"};


Model::Model(vtkSmartPointer<vtkPolyData> modelData)
  : m_polyData{modelData}
{
  // Place model with lower Z bound at zero
  m_positionZ = -m_polyData->GetBounds()[4];

  vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(m_positionX, m_positionY, m_positionZ);

  m_filterTranslate = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  m_filterTranslate->SetInputData(m_polyData);
  m_filterTranslate->SetTransform(translation);
  m_filterTranslate->Update();

  // Color based on Z
  double bounds[6];
  m_polyData->GetBounds(bounds);

  double zMin = bounds[4];
  double zMax = bounds[5];
  // Color data points based on z-value
  vtkSmartPointer<vtkElevationFilter> colorizer =
    vtkSmartPointer<vtkElevationFilter>::New();

  colorizer->SetInputConnection(m_filterTranslate->GetOutputPort());   
  colorizer->SetLowPoint(0, 0, zMin);
  colorizer->SetHighPoint(0, 0, zMax);
   
  // Model Mapper
  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetInputConnection(colorizer->GetOutputPort());
  m_mapper->ScalarVisibilityOff();

  // Model Actor
  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetInterpolationToFlat();

  m_actor->GetProperty()->SetAmbient(0.1);
  m_actor->GetProperty()->SetDiffuse(0.7);
  m_actor->GetProperty()->SetSpecular(0.3);
  this->setColor(m_defaultColor);

  m_actor->SetPosition(0.0, 0.0, 0.0);
}


const vtkSmartPointer<vtkActor> &Model::getActor() const
{
  return m_actor;
}


double Model::getPositionX()
{
  m_propertiesMutex.lock();
  double positionX = m_positionX;
  m_propertiesMutex.unlock();
  return positionX;
}

double Model::getPositionY()
{
  m_propertiesMutex.lock();
  double positionY = m_positionY;
  m_propertiesMutex.unlock();
  return positionY;
}

void Model::setPositionX(const double positionX)
{
  if (m_positionX != positionX)
    {
      m_positionX = positionX;
      emit positionXChanged(m_positionX);
    }
}

void Model::setPositionY(const double positionY)
{
  if (m_positionY != positionY)
    {
      m_positionY = positionY;
      emit positionYChanged(m_positionY);
    }
}


void Model::translateToPosition(const double x, const double y)
{
  if (m_positionX == x && m_positionY == y)
    {
      return;
    }

  m_propertiesMutex.lock();
  this->setPositionX(x);
  this->setPositionY(y);
  m_propertiesMutex.unlock();

  vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(m_positionX, m_positionY, m_positionZ);
  m_filterTranslate->SetTransform(translation);
  m_filterTranslate->Update();

  emit positionXChanged(m_positionX);
  emit positionYChanged(m_positionY);
}


void Model::setSelected(const bool selected)
{
  if (m_selected != selected)
    {
      m_selected = selected;

      this->updateColor();
    }
}

void Model::setSelectedColor(const QColor &selectedColor)
{
  m_selectedColor = selectedColor;
}

void Model::updateColor()
{
  if (m_selected)
    {
      this->setColor(m_selectedColor);
    }
  else
    {
      this->setColor(m_defaultColor);
    }
}

void Model::setColor(const QColor &color)
{
  m_actor->GetProperty()->SetColor(color.redF(), color.greenF(),
				   color.blueF());
}


const double Model::getMouseDeltaX() const
{
  return m_mouseDeltaX;
}

const double Model::getMouseDeltaY() const
{
  return m_mouseDeltaY;
}

void Model::setMouseDeltaXY(const double deltaX, const double deltaY)
{
  m_mouseDeltaX = deltaX;
  m_mouseDeltaY = deltaY;
}
