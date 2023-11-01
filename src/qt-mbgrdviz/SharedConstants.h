#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>

/// Constants shared between C++ and QML code
class SharedConstants : public QObject
{
    Q_OBJECT

public:

  enum EditState {
		  Unknown,
		  Pointing,
		  EditRoute,
		  EditPoints,
		  EditOverlay
  };

  Q_ENUM(EditState)

  enum GuiParameter {ColorMap,
		     ShowAxes,
		     RouteFile,
		     SiteFile
  };
  
  Q_ENUM(GuiParameter)

  
  
  
  /// Define read-only QString property called "testString"
  Q_PROPERTY(QString testString READ getTestString)

  /// Just return a dummy string for now...
  QString getTestString() const {
      std::cerr << "**** getTestString()\n";
      return testString_;
  }


    protected:
      
      static const QString testString_;

};

#endif
