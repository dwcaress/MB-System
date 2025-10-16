#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>
#include <QStringList>
#include <iostream>

#define MousePanAndZoom "Basic pan and zoom"
#define MouseRotateModel "Rotate model"
#define MouseRotateView "Rotate view"
#define MouseLighting "Lighting"
#define MouseDataSelect "Select data"
#define MousePickArea "Pick area"
#define MouseEditSites "Edit sites"
#define MouseEditRoutes "Edit routes"
#define MousePickNav "Pick nav"
#define MousePickNavFile "Pick nav file"
#define MouseTest "TESTING"

namespace mb_system {

  /// Data model for 'mouse mode', accessed from QML
  /// including mode name and brief ToolTip.
  /// Class is defined outside of SharedConstants, as
  /// nested QObject definitions are not supported.
  class MouseMode : public QObject {
    Q_OBJECT

  public:
    // Make sure the assignment operator is public
    MouseMode& operator=(const MouseMode& other);

    MouseMode(QString name, QString toolTip) {
      name_ = name;
      toolTip_ = toolTip;
    }
    
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QString toolTip READ getToolTip)

    QString getName() const { return name_; }
    QString getToolTip() const { return toolTip_; }

    QString name_;
    QString toolTip_;
  };

/// Constants defined here in C++ and accessible through QML.
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
  Q_PROPERTY(QStringList cmaps MEMBER colorMapsList_ NOTIFY cmapsChanged)

  /// List of supported mouse modes
  Q_PROPERTY(QList<MouseMode *> mouseModes MEMBER mouseModes_
	     NOTIFY mouseModesChanged)

signals:
  // Emit this if cmaps changes
  void cmapsChanged();

  // Emit this if mouseModes changes
  void mouseModesChanged();  
  
protected:
      
  static const QString testString_;

  QStringList colorMapsList_;
  QList<MouseMode *> mouseModes_;
};

}
#endif
