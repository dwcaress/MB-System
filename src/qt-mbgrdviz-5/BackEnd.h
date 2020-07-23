#ifndef CANVASHANDLER_H
#define CANVASHANDLER_H

#include <memory>

#include <QObject>
#include <QUrl>


class ProcessingEngine;
class QVtkItem;

/** 
BackEnd manages interaction with QML, is the QML C++ "backend", its properties and methods available
through QML.
*/
class BackEnd : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool showFileDialog MEMBER m_showFileDialog NOTIFY showFileDialogChanged)
	Q_PROPERTY(bool isModelSelected READ getIsModelSelected NOTIFY isModelSelectedChanged)
	Q_PROPERTY(double modelPositionX READ getSelectedModelPositionX NOTIFY selectedModelPositionXChanged)
	Q_PROPERTY(double modelPositionY READ getSelectedModelPositionY NOTIFY selectedModelPositionYChanged)

public:
	BackEnd(int argc, char **argv);

	Q_INVOKABLE void openModel(const QUrl &path) const;

	Q_INVOKABLE void mousePressEvent(const int button, const int mouseX, const int mouseY) const;
	Q_INVOKABLE void mouseMoveEvent(const int button, const int mouseX, const int mouseY);
	Q_INVOKABLE void mouseReleaseEvent(const int button, const int mouseX, const int mouseY);

	bool getIsModelSelected() const;
	double getSelectedModelPositionX() const;
	double getSelectedModelPositionY() const;

	Q_INVOKABLE void setModelsRepresentation(const int representationOption);
	Q_INVOKABLE void setModelsOpacity(const double opacity);
	Q_INVOKABLE void setGouraudInterpolation(const bool gouraudInterpolation);
	Q_INVOKABLE void setModelColorR(const int colorR);
	Q_INVOKABLE void setModelColorG(const int colorG);
	Q_INVOKABLE void setModelColorB(const int colorB);

public slots:
	void startApplication() const;

signals:
	void showFileDialogChanged();

	void isModelSelectedChanged();
	void selectedModelPositionXChanged();
	void selectedModelPositionYChanged();

private:

	std::shared_ptr<ProcessingEngine> m_processingEngine;
	QVtkItem *m_vtkFboItem = nullptr;

	double m_previousWorldX = 0;
	double m_previousWorldY = 0;
	bool m_draggingMouse = false;
	bool m_showFileDialog = false;
};

#endif // CANVASHANDLER_H
