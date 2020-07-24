#include "ProcessingEngine.h"

#include <thread>
#include <memory>

#include <QDebug>
#include <QFileInfo>

#include <vtkAlgorithmOutput.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkElevationFilter.h>

#include "Model.h"
#include "GmtGridReader.h"

ProcessingEngine::ProcessingEngine()
{
}


const std::shared_ptr<Model> &ProcessingEngine::addModel(const QUrl &modelFilePath)
{
  qDebug() << "ProcessingEngine::addModelData()";

  QString modelFilePathExtension =
    QFileInfo(modelFilePath.toString()).suffix().toLower();

  vtkSmartPointer<GmtGridReader> gmtReader =
    vtkSmartPointer<GmtGridReader>::New();

  vtkSmartPointer<vtkPolyData> inputData;

  if (modelFilePathExtension == "grd") {
    // Read OBJ file
    gmtReader->SetFileName(modelFilePath.toString().toStdString().c_str());
    gmtReader->Update();
    inputData = vtkSmartPointer<vtkPolyData>::New();
		
    inputData = gmtReader->GetOutput();
  }
  else {
    std::cerr << "Unknown file type: " << std::endl;
    exit(1);
  }
	
  // Preprocess the polydata
  vtkSmartPointer<vtkPolyData> processedData = preprocess(inputData);

  // Create Model instance and insert it into the vector
  std::shared_ptr<Model> model = std::make_shared<Model>(processedData);
	
  m_models.push_back(model);

  return m_models.back();
}

vtkSmartPointer<vtkPolyData> ProcessingEngine::preprocess(const vtkSmartPointer<vtkPolyData> inputData) const
{
  // Center the polygon
  double center[3];
  inputData->GetCenter(center);

  vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(-center[0], -center[1], -center[2]);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(inputData);
  transformFilter->SetTransform(translation);
  transformFilter->Update();

  // Normals - For the Gouraud interpolation to work
  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInputData(transformFilter->GetOutput());
  normals->ComputePointNormalsOn();
  normals->Update();

  return normals->GetOutput();
}

void ProcessingEngine::placeModel(Model &model) const
{
  qDebug() << "ProcessingEngine::placeModel()";

  model.translateToPosition(0, 0);
}

void ProcessingEngine::setModelsRepresentation(const int modelsRepresentationOption) const
{
  for (const std::shared_ptr<Model>& model : m_models)
    {
      model->getActor()->GetProperty()->SetRepresentation(modelsRepresentationOption);
    }
}

void ProcessingEngine::setModelsOpacity(const double modelsOpacity) const
{
  for (const std::shared_ptr<Model>& model : m_models)
    {
      model->getActor()->GetProperty()->SetOpacity(modelsOpacity);
    }
}

void ProcessingEngine::setModelsGouraudInterpolation(const bool enableGouraudInterpolation) const
{
  for (const std::shared_ptr<Model>& model : m_models)
    {
      if (enableGouraudInterpolation)
	{
	  model->getActor()->GetProperty()->SetInterpolationToGouraud();
	}
      else
	{
	  model->getActor()->GetProperty()->SetInterpolationToFlat();
	}
    }
}

void ProcessingEngine::updateModelsColor() const
{
  for (const std::shared_ptr<Model>& model : m_models)
    {
      model->updateColor();
    }
}

std::shared_ptr<Model> ProcessingEngine::getModelFromActor(const vtkSmartPointer<vtkActor> modelActor) const
{
  for (const std::shared_ptr<Model> &model : m_models)
    {
      if (model->getActor() == modelActor)
	{
	  return model;
	}
    }

  // Raise exception instead
  return nullptr;
}
