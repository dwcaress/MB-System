#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>
#include <QStringList>
#include <iostream>

/// These mouse mode names are displayed as choices in QML, then
/// selected mode name is passed to backend function when user selects.
#define MousePanAndZoom "Basic pan, tilt, zoom"
#define MouseRotateModel "Rotate model"
#define MouseRotateView "Rotate view"
#define MouseLighting "Lighting"
#define MouseDataSelect "Select data"
#define MousePickArea "Pick area"
#define MouseEditSites "Edit sites"
#define MouseEditRoutes "Edit routes"
#define MousePickNav "Pick nav"
#define MousePickNavFile "Pick nav file"
#define MouseElevProfile "Define elev profile"
#define MouseTest "TESTING"

/***
    These classes describe data that is accessed by C++
    (especially TopoDataItem) as well as QML for individual QtQuck applications.
 */
namespace mb_system {

  /// Data model for 'mouse mode', accessed from QML
  /// including mode name and brief ToolTip. User can choose from
  /// a set of available mouse modes (e.g. as menu items).
  /// This class is defined outside of SharedConstants class, as
  /// nested QObject definitions are not supported.
  /// SharedConstants contains a QList<MouseMode>.
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

  /// SharedConstants are defined here in C++ and are accessible through QML,
  /// e.g. can be instantiated in QML when registered with
  /// qmlReisterType<SharedConstants>() from main.cpp.
  class SharedConstants : public QObject {
    Q_OBJECT

  public:

    SharedConstants();

    /// (Not yet anywhere yet)
    enum class EditState : int {
      ViewOnly,
      EditRoute,
      EditPoints,
      EditOverlay
    };


    /// List of supported color maps
    Q_PROPERTY(QStringList cmaps MEMBER colorMapsList_ NOTIFY cmapsChanged)

    /// List of supported mouse modes
    Q_PROPERTY(QList<MouseMode *> mouseModes MEMBER mouseModes_
	       NOTIFY mouseModesChanged)

  signals:
    // Emit this if cmaps changes (probably never)
    void cmapsChanged();

    // Emit this if mouseModes changes (probably never)
    void mouseModesChanged();  
  
  protected:
      
    /// Populated by SharedConstants constructor
    QStringList colorMapsList_;

    /// Populated by SharedConstants constructor
    QList<MouseMode *> mouseModes_;
  };

}   // namespace

#endif
