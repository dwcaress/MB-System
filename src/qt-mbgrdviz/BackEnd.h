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

  enum EnumTest {
		 State0,
		 State1,
		 State2 };

  Q_ENUM(EnumTest)

  explicit BackEnd(QQmlApplicationEngine *engine,
		   QObject *parent = nullptr);

  /// Create and register singleton
  static bool registerSingleton(int argc, char **argv,
                                QQmlApplicationEngine *engine);

  /// Specify grid file to display
  Q_INVOKABLE bool setGridFile(QUrl file);

					 
public slots:
  
  // Slot for qml-generated signals
  void qmlSlot(const QString &msg);

  void sigSlot(const int param, const QString &value);
  
signals:
  
  
protected:

  /// Singleton
  static BackEnd *singleInstance_;

  /// QVtkItem instantiated by QML
  mb_system::QVtkItem *qVtkItem_;

  /// Selected file name item
  QObject *selectedFileItem_;

private:
  /// Constant string member test
  inline static const std::string testString_ = "test string member";

};

#endif // BACKEND_H
