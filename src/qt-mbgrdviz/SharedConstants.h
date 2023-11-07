#ifndef SHAREDCONSTANTS_H
#define SHAREDCONSTANTS_H
#include <QObject>

namespace sharedQmlCpp {
/// Constants shared between C++ and QML code
class Const : public QObject
{
    Q_OBJECT

public:

  enum class EditState {
		  Unknown,
		  Pointing,
		  EditRoute,
		  EditPoints,
		  EditOverlay
  };

  Q_ENUM(EditState)


  /// Command issued from GUI
  enum class Cmd : int {ColorMap,
			ShowAxes,
			VerticalExag,
			RouteFile,
			SiteFile};
  
  Q_ENUM(Cmd)

    
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

}
#endif
