#ifndef PROCESSINGENGINE_H
#define PROCESSINGENGINE_H

#include <array>
#include <cstdint>
#include <vector>
#include <mutex>
#include <memory>

#include <QUrl>

#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>


class Model;

/**
   Interacts with VTK libraries to generate and process 3D models
 */
class ProcessingEngine
{
	public:
		ProcessingEngine();

		const std::shared_ptr<Model>& addModel(const QUrl &modelFilePath);

		void placeModel(Model &model) const;

		void setModelsRepresentation(const int modelsRepresentationOption) const;
		void setModelsOpacity(const double modelsOpacity) const;
		void setModelsGouraudInterpolation(const bool enableGouraudInterpolation) const;
		void updateModelsColor() const;

		std::shared_ptr<Model> getModelFromActor(const vtkSmartPointer<vtkActor> modelActor) const;

	private:
		vtkSmartPointer<vtkPolyData> preprocessPolydata(const vtkSmartPointer<vtkPolyData> inputData) const;

		std::vector<std::shared_ptr<Model>> m_models;
};

#endif // PROCESSINGENGINE_H
