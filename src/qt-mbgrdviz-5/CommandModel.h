#ifndef COMMANDMODEL_H
#define COMMANDMODEL_H


class QVTKFramebufferObjectRenderer;

class CommandModel
{
public:
	CommandModel(){}
	virtual ~CommandModel(){}

	virtual bool isReady() const = 0;
	virtual void execute() = 0;

protected:
	QVTKFramebufferObjectRenderer *m_vtkFboRenderer;
};

#endif // COMMANDMODEL_H
