#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>
#include <QStringList>

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
  
protected:
      
  static const QString testString_;

  QStringList colorMapsList_;
  
  
};

}
#endif
