#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>
#include <QStringList>
#include <iostream>

#define MousePanAndZoom "Pan and zoom"
#define MouseRotateModel "Rotate model"
#define MouseRotateView "Rotate view"
#define MouseShading "Shading"
#define MousePickArea "Pick area"
#define MouseEditSites "Edit sites"
#define MouseEditRoutes "Edit routes"
#define MousePickNav "Pick nav"
#define MousePickNavFile "Pick nav file"


namespace mb_system {
/// Constants shared between C++ and QML code
class SharedConstants : public QObject
{
    Q_OBJECT

public:

  SharedConstants();
  
  enum class EditState : int {
			      ViewOnly,
			      EditRoute,
			      EditPoints,
			      EditOverlay
  };

  Q_ENUM(EditState)

  /// Define read-only QString property called "testString"
  Q_PROPERTY(QString testString READ getTestString)

  /// Just return a dummy string for now...
  QString getTestString() const {
      std::cerr << "**** getTestString()\n";
      return testString_;
  }

  /// List of supported color maps
  Q_PROPERTY(QStringList cmaps MEMBER colorMapsList_)

  /// List of supported mouse modes
  Q_PROPERTY(QStringList mouseModes MEMBER mouseModes_)  
  
protected:
      
  static const QString testString_;

  QStringList colorMapsList_;
  QStringList mouseModes_;
  
  
};

}
#endif
