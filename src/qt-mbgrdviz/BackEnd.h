#ifndef BACKEND_H
#define BACKEND_H
#include <vector>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QStringListModel>
#include "TopoGridItem.h"


/**
   @deprecated BackEnd is an interface between QML and C++ objects and 
   functions.BackEnd methods handle user inputs to QML, e.g. menu selections.
   NOT CURRENTLY NEEDED, since TopoGridItem INVOKABLE functions handle most
   needs now.
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


  Q_INVOKABLE QStringList getColorMaps() const {
    std::cerr << "*** getColorMaps()\n";
    return colorMapsList_;
  }
  
					 
public slots:

  void sigSlot(const int param, const QString &value);
  
	      
signals:
  void listChanged(QString m);
  
  
protected:

  /// Singleton
  static BackEnd *singleInstance_;

  /// TopoGridItem instantiated by QML
  mb_system::TopoGridItem *topoGridItem_;

  /// Selected file name item
  QObject *selectedFileItem_;

  ///  std::vector<const char *> colorMapNames_;
  /// QStringListModel colorMapsModel_;
  QStringList colorMapsList_;  
  
private:
  /// Constant string member test
  inline static const std::string testString_ = "test string member";

};

#endif // BACKEND_H
