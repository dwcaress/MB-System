#include "CommandModelAdd.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include "QVTKFramebufferObjectRenderer.h"


CommandModelAdd::CommandModelAdd(QVTKFramebufferObjectRenderer *vtkFboRenderer, std::shared_ptr<ProcessingEngine> processingEngine, QUrl modelPath)
	: m_processingEngine{processingEngine}
	, m_modelPath{modelPath}
{
	m_vtkFboRenderer = vtkFboRenderer;
}


void CommandModelAdd::run()
{
	qDebug() << "CommandModelAdd::run()";

	m_model = m_processingEngine->addModel(m_modelPath);

	m_processingEngine->placeModel(*m_model);

	m_ready = true;
	emit ready();
}


bool CommandModelAdd::isReady() const
{
	return m_ready;
}

void CommandModelAdd::execute()
{
	qDebug() << "CommandModelAdd::execute()";

	m_vtkFboRenderer->addModelActor(m_model);

	emit done();
}
