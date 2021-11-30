#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "QVtkItem.h"

/**
   BackEnd is an interface between QML and C++ objects and functions.
   BackEnd methods handle user inputs to QML, e.g. menu selections.
*/
class BackEnd : public QObject
{
  Q_OBJECT
public:
  explicit BackEnd(QQmlApplicationEngine *engine,
		   QObject *parent = nullptr);

  /// Create and register singleton
  static bool registerSingleton(int argc, char **argv,
                                QQmlApplicationEngine *engine);
  /// Specify grid file to display
  Q_INVOKABLE bool setGridFile(QUrl file);

  /// Toggle axes display
  Q_INVOKABLE void showAxes(bool show);


public slots:
  // Slot for qml-generated signals
  void qmlSlot(const QString &msg);
  
signals:

protected:

  /// Singleton
  static BackEnd *singleInstance_;

  /// QVtkItem instantiated by QML
  mb_system::QVtkItem *qVtkItem_;

  /// Selected file name item
  QObject *selectedFileItem_;

};

#endif // BACKEND_H
