#ifndef GuiNames_
#define GuiNames_

#include <QObject>


/// Define names of C++-connected QML items, used when exchanging information
/// between QML GUI and C++ code.
/// N.B.: Take care that QML code uses the literal strings defined here
/// objectName

/// Name of PixmapImage C++ object instantiated by QML, which holds swath
/// graphs; Backend C++ code draws into this object
#define SWATH_PIXMAP_NAME "swathPixmapObj"

/// Names of user-selected checkboxes that enable drawing of specific plots;
/// passed by QML as arg to Backend C++ callbacks
#define TIMEINT_PLOTNAME "timeInt"
#define TIMEINT_ORIG_PLOTNAME "timeIntOrig"
#define LAT_PLOTNAME "lat"
#define LAT_ORIG_PLOTNAME "latOrig"
#define LON_PLOTNAME "lon"
#define LON_ORIG_PLOTNAME "lonOrig"
#define SPEED_PLOTNAME "speed"
#define SPEED_ORIG_PLOTNAME "speedOrig"
#define SPEED_MADEGOOD_PLOTNAME "speedMadeGood"
#define HEADING_PLOTNAME "heading"
#define HEADING_ORIG_PLOTNAME "headingOrig"
#define HEADING_MADEGOOD_PLOTNAME "headingMadeGood"
#define SENSORDEPTH_PLOTNAME "sensorDepth"
#define SENSORDEPTH_ORIG_PLOTNAME "sensorDepthOrig"
#define ATTITUDE_PLOTNAME "attitude"

/// Names of edit modes selected by user; passed by QML as arg to
/// Backend C++ callbacks
#define PICK_MODENAME "pickMode"
#define SELECT_MODENAME "selectMode"
#define DESELECT_MODENAME "deselectMode"
#define SELECT_ALL_MODENAME "selectAllMode"
#define DESELECT_ALL_MODENAME "deselectAllMode"
#define DEFINE_INTERVAL_MODENAME "defineIntervalMode"

class GuiNames : public QObject {

  Q_OBJECT

 public:
  GuiNames() {}
  
  Q_PROPERTY(QString swathPixmap READ swathPixmap CONSTANT)
  QString swathPixmap() const {return QStringLiteral(SWATH_PIXMAP_NAME);}

  Q_PROPERTY(QString timeIntPlot READ timeIntPlot CONSTANT)
  QString timeIntPlot() const {return QStringLiteral(TIMEINT_PLOTNAME);}  

  Q_PROPERTY(QString swathPixmapObj READ swathPixmapObj CONSTANT)
  QString swathPixmapObj() const {return QStringLiteral(SWATH_PIXMAP_NAME);}

  Q_PROPERTY(QString timeInt READ timeInt CONSTANT)
  QString timeInt() const {return QStringLiteral(TIMEINT_PLOTNAME);}

  Q_PROPERTY(QString timeIntOrig READ timeIntOrig CONSTANT)
  QString timeIntOrig() const {return QStringLiteral(TIMEINT_ORIG_PLOTNAME);}

  Q_PROPERTY(QString lat READ lat CONSTANT)
  QString lat() const {return QStringLiteral(LAT_PLOTNAME);}

  Q_PROPERTY(QString latOrig READ latOrig CONSTANT)
  QString latOrig() const {return QStringLiteral(LAT_ORIG_PLOTNAME);}

  Q_PROPERTY(QString lon READ lon CONSTANT)
  QString lon() const {return QStringLiteral(LON_PLOTNAME);}

  Q_PROPERTY(QString lonOrig READ lonOrig CONSTANT)
  QString lonOrig() const {return QStringLiteral(LON_ORIG_PLOTNAME);}

  Q_PROPERTY(QString speed READ speed CONSTANT)
  QString speed() const {return QStringLiteral(SPEED_PLOTNAME);}

  Q_PROPERTY(QString speedOrig READ speedOrig CONSTANT)
  QString speedOrig() const {return QStringLiteral(SPEED_ORIG_PLOTNAME);}

  Q_PROPERTY(QString speedMadeGood READ speedMadeGood CONSTANT)
  QString speedMadeGood() const {return QStringLiteral(SPEED_MADEGOOD_PLOTNAME);}

  Q_PROPERTY(QString heading READ heading CONSTANT)
  QString heading() const {return QStringLiteral(HEADING_PLOTNAME);}

  Q_PROPERTY(QString headingOrig READ headingOrig CONSTANT)
  QString headingOrig() const {return QStringLiteral(HEADING_ORIG_PLOTNAME);}

  Q_PROPERTY(QString headingMadeGood READ headingMadeGood CONSTANT)
  QString headingMadeGood() const {return QStringLiteral(HEADING_MADEGOOD_PLOTNAME);}

  Q_PROPERTY(QString sensorDepth READ sensorDepth CONSTANT)
  QString sensorDepth() const {return QStringLiteral(SENSORDEPTH_PLOTNAME);}

  Q_PROPERTY(QString sensorDepthOrig READ sensorDepthOrig CONSTANT)
  QString sensorDepthOrig() const {return QStringLiteral(SENSORDEPTH_ORIG_PLOTNAME);}

  Q_PROPERTY(QString attitude READ attitude CONSTANT)
  QString attitude() const {return QStringLiteral(ATTITUDE_PLOTNAME);}

  Q_PROPERTY(QString pickMode READ pickMode CONSTANT)
  QString pickMode() const {return QStringLiteral(PICK_MODENAME);}

  Q_PROPERTY(QString selectMode READ selectMode CONSTANT)
  QString selectMode() const {return QStringLiteral(SELECT_MODENAME);}

  Q_PROPERTY(QString deselectMode READ deselectMode CONSTANT)
  QString deselectMode() const {return QStringLiteral(DESELECT_MODENAME);}

  Q_PROPERTY(QString selectAllMode READ selectAllMode CONSTANT)
  QString selectAllMode() const {return QStringLiteral(SELECT_ALL_MODENAME);}

  Q_PROPERTY(QString deselectAllMode READ deselectAllMode CONSTANT)
  QString deselectAllMode()
    const {return QStringLiteral(DESELECT_ALL_MODENAME);}

  Q_PROPERTY(QString defineIntervalMode READ defineIntervalMode CONSTANT)
  QString defineIntervalMode()
    const {return QStringLiteral(DEFINE_INTERVAL_MODENAME);}
  
};

#endif
