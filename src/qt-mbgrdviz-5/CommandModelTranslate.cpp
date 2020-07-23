#include <array>

#include "CommandModelTranslate.h"
#include "Model.h"
#include "QVTKFramebufferObjectRenderer.h"


CommandModelTranslate::CommandModelTranslate(QVTKFramebufferObjectRenderer *vtkFboRenderer, const TranslateParams_t & translateData, bool inTransition)
	: m_translateParams{translateData}
	, m_inTransition{inTransition}
{
	m_vtkFboRenderer = vtkFboRenderer;
}

bool CommandModelTranslate::isReady() const
{
	return true;
}

void CommandModelTranslate::transformCoordinates()
{
	std::array<double, 3> worldCoordinates;

	if (m_vtkFboRenderer->screenToWorld(m_translateParams.screenX, m_translateParams.screenY, worldCoordinates.data()))
	{
		m_translateParams.targetPositionX = worldCoordinates[0] - m_translateParams.model->getMouseDeltaX();
		m_translateParams.targetPositionY = worldCoordinates[1] - m_translateParams.model->getMouseDeltaY();
	}
	else
	{
		m_translateParams.targetPositionX = m_translateParams.model->getPositionX();
		m_translateParams.targetPositionY = m_translateParams.model->getPositionY();
	}

	m_needsTransformation = false;
}

void CommandModelTranslate::execute()
{
	// Screen to world transformation can only be done within the Renderer thread
	if (m_needsTransformation)
	{
		this->transformCoordinates();
	}

	m_translateParams.model->translateToPosition(m_translateParams.targetPositionX, m_translateParams.targetPositionY);
}

