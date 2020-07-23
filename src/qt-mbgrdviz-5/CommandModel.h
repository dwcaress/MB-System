#ifndef COMMANDMODEL_H
#define COMMANDMODEL_H


class QVtkRenderer;

class CommandModel
{
public:
	CommandModel(){}
	virtual ~CommandModel(){}

	virtual bool isReady() const = 0;
	virtual void execute() = 0;

protected:
	QVtkRenderer *m_vtkFboRenderer;
};

#endif // COMMANDMODEL_H
