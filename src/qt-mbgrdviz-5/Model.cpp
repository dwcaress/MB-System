#include <QDebug>

#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTransform.h>

#include "Model.h"


QColor Model::m_defaultModelColor = QColor{"#0277bd"};
QColor Model::m_selectedModelColor = QColor{"#03a9f4"};


Model::Model(vtkSmartPointer<vtkPolyData> modelData)
	: m_modelData{modelData}
{
	// Place model with lower Z bound at zero
	m_positionZ = -m_modelData->GetBounds()[4];

	vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
	translation->Translate(m_positionX, m_positionY, m_positionZ);

	m_modelFilterTranslate = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	m_modelFilterTranslate->SetInputData(m_modelData);
	m_modelFilterTranslate->SetTransform(translation);
	m_modelFilterTranslate->Update();

	// Model Mapper
	m_modelMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_modelMapper->SetInputConnection(m_modelFilterTranslate->GetOutputPort());
	m_modelMapper->ScalarVisibilityOff();

	// Model Actor
	m_modelActor = vtkSmartPointer<vtkActor>::New();
	m_modelActor->SetMapper(m_modelMapper);
	m_modelActor->GetProperty()->SetInterpolationToFlat();

	m_modelActor->GetProperty()->SetAmbient(0.1);
	m_modelActor->GetProperty()->SetDiffuse(0.7);
	m_modelActor->GetProperty()->SetSpecular(0.3);
	this->setColor(m_defaultModelColor);

	m_modelActor->SetPosition(0.0, 0.0, 0.0);
}


const vtkSmartPointer<vtkActor> &Model::getModelActor() const
{
	return m_modelActor;
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
	m_modelFilterTranslate->SetTransform(translation);
	m_modelFilterTranslate->Update();

	emit positionXChanged(m_positionX);
	emit positionYChanged(m_positionY);
}


void Model::setSelected(const bool selected)
{
	if (m_selected != selected)
	{
		m_selected = selected;

		this->updateModelColor();
	}
}

void Model::setSelectedModelColor(const QColor &selectedModelColor)
{
	m_selectedModelColor = selectedModelColor;
}

void Model::updateModelColor()
{
	if (m_selected)
	{
		this->setColor(m_selectedModelColor);
	}
	else
	{
		this->setColor(m_defaultModelColor);
	}
}

void Model::setColor(const QColor &color)
{
	m_modelActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
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
