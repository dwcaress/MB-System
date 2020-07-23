#include "ProcessingEngine.h"

#include <thread>
#include <memory>

#include <QDebug>
#include <QFileInfo>

#include <vtkAlgorithmOutput.h>
#include <vtkOBJReader.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkSTLReader.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include "Model.h"
#include "GmtGridReader.h"

ProcessingEngine::ProcessingEngine()
{
}


const std::shared_ptr<Model> &ProcessingEngine::addModel(const QUrl &modelFilePath)
{
	qDebug() << "ProcessingEngine::addModelData()";

	QString modelFilePathExtension = QFileInfo(modelFilePath.toString()).suffix().toLower();

    vtkSmartPointer<vtkOBJReader> objReader = vtkSmartPointer<vtkOBJReader>::New();
    vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();

    vtkSmartPointer<GmtGridReader> gmtReader =
      vtkSmartPointer<GmtGridReader>::New();

    vtkSmartPointer<vtkPolyData> inputData;

	if (modelFilePathExtension == "obj")
	{
		// Read OBJ file
		objReader->SetFileName(modelFilePath.toString().toStdString().c_str());
		objReader->Update();
		inputData = objReader->GetOutput();
	}
	else if (modelFilePathExtension == "stl")
	{
		// Read STL file
		stlReader->SetFileName(modelFilePath.toString().toStdString().c_str());
		stlReader->Update();
		inputData = stlReader->GetOutput();
	}
	else if (modelFilePathExtension == "grd") {
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
	
	std::cout << "inputData #strips: "
		  << inputData->GetNumberOfStrips() << std::endl;
	std::cout << "inputData #lines: "
		  << inputData->GetNumberOfLines() << std::endl;
	std::cout << "inputData #verts: "
		  << inputData->GetNumberOfVerts() << std::endl;  

	// Preprocess the polydata
	vtkSmartPointer<vtkPolyData> preprocessedPolydata = preprocessPolydata(inputData);

	// Create Model instance and insert it into the vector
	std::shared_ptr<Model> model = std::make_shared<Model>(preprocessedPolydata);
	
	m_models.push_back(model);

	return m_models.back();
}

vtkSmartPointer<vtkPolyData> ProcessingEngine::preprocessPolydata(const vtkSmartPointer<vtkPolyData> inputData) const
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
		model->getModelActor()->GetProperty()->SetRepresentation(modelsRepresentationOption);
	}
}

void ProcessingEngine::setModelsOpacity(const double modelsOpacity) const
{
	for (const std::shared_ptr<Model>& model : m_models)
	{
		model->getModelActor()->GetProperty()->SetOpacity(modelsOpacity);
	}
}

void ProcessingEngine::setModelsGouraudInterpolation(const bool enableGouraudInterpolation) const
{
	for (const std::shared_ptr<Model>& model : m_models)
	{
		if (enableGouraudInterpolation)
		{
			model->getModelActor()->GetProperty()->SetInterpolationToGouraud();
		}
		else
		{
			model->getModelActor()->GetProperty()->SetInterpolationToFlat();
		}
	}
}

void ProcessingEngine::updateModelsColor() const
{
	for (const std::shared_ptr<Model>& model : m_models)
	{
		model->updateModelColor();
	}
}

std::shared_ptr<Model> ProcessingEngine::getModelFromActor(const vtkSmartPointer<vtkActor> modelActor) const
{
	for (const std::shared_ptr<Model> &model : m_models)
	{
		if (model->getModelActor() == modelActor)
		{
			return model;
		}
	}

	// Raise exception instead
	return nullptr;
}
