#ifndef COMMANDMODELADD_H
#define COMMANDMODELADD_H

#include <memory>

#include <QUrl>
#include <QThread>

#include "CommandModel.h"


class Model;
class ProcessingEngine;
class QVtkRenderer;

class CommandModelAdd : public QThread, public CommandModel
{
	Q_OBJECT

public:
	CommandModelAdd(QVtkRenderer *vtkFboRenderer,
			std::shared_ptr<ProcessingEngine> processingEngine,
			QUrl modelPath);

	void run() Q_DECL_OVERRIDE;

	bool isReady() const override;
	void execute() override;

signals:
	void ready();
	void done();

private:
	std::shared_ptr<ProcessingEngine> m_processingEngine;
	std::shared_ptr<Model> m_model = nullptr;
	QUrl m_modelPath;
	double m_positionX;
	double m_positionY;

	bool m_ready = false;
};

#endif // COMMANDMODELADD_H
