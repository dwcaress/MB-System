#ifndef GuiNames_
#define GuiNames_

#include <QObject>


/**
   GuiNames defines QString properties used for objectName in QML GUI,
   defined to match the string literals referenced by Backend C++ methods.
*/
class GuiNames : public QObject {

  Q_OBJECT

 public:
  GuiNames() {}

  /// Name of PixmapImage C++ object instantiated by QML, which holds
  /// swath graphs; Backend C++ code draws into this object
  static inline const char *swathPixmapObjStr = "swathPixmapObj";
  Q_PROPERTY(QString swathPixmapObj READ swathPixmapObj CONSTANT)
  QString swathPixmapObj() const {return QString::fromLatin1(swathPixmapObjStr);}

  /// GUI elements that specify which plots should be drawn.
  static inline const char *timeIntStr = "timeInt";
  Q_PROPERTY(QString timeInt READ timeInt CONSTANT)
  QString timeInt() const {return QString::fromLatin1(timeIntStr);}

  static inline const char *timeIntOrigStr = "timeIntOrig";
  Q_PROPERTY(QString timeIntOrig READ timeIntOrig CONSTANT)
  QString timeIntOrig() const {return QString::fromLatin1(timeIntOrigStr);}

  static inline const char *latStr = "lat";
  Q_PROPERTY(QString lat READ lat CONSTANT)
  QString lat() const {return QString::fromLatin1(latStr);}

  static inline const char *latOrigStr = "latOrig";
  Q_PROPERTY(QString latOrig READ latOrig CONSTANT)
  QString latOrig() const {return QString::fromLatin1(latOrigStr);}

  static inline const char *lonStr = "lon";
  Q_PROPERTY(QString lon READ lon CONSTANT)
  QString lon() const {return QString::fromLatin1(lonStr);}

  static inline const char *lonOrigStr = "lonOrig";
  Q_PROPERTY(QString lonOrig READ lonOrig CONSTANT)
  QString lonOrig() const {return QString::fromLatin1(lonOrigStr);}

  static inline const char *speedStr = "speed";
  Q_PROPERTY(QString speed READ speed CONSTANT)
  QString speed() const {return QString::fromLatin1(speedStr);}

  static inline const char *speedOrigStr = "speedOrig";
  Q_PROPERTY(QString speedOrig READ speedOrig CONSTANT)
  QString speedOrig() const {return QString::fromLatin1(speedOrigStr);}

  static inline const char *speedMadeGoodStr = "speedMadeGood";
  Q_PROPERTY(QString speedMadeGood READ speedMadeGood CONSTANT)
  QString speedMadeGood() const {return QString::fromLatin1(speedMadeGoodStr);}

  static inline const char *headingStr = "heading";
  Q_PROPERTY(QString heading READ heading CONSTANT)
  QString heading() const {return QString::fromLatin1(headingStr);}

  static inline const char *headingOrigStr = "headingOrig";
  Q_PROPERTY(QString headingOrig READ headingOrig CONSTANT)
  QString headingOrig() const {return QString::fromLatin1(headingOrigStr);}

  static inline const char *headingMadeGoodStr = "headingMadeGood";
  Q_PROPERTY(QString headingMadeGood READ headingMadeGood CONSTANT)
  QString headingMadeGood() const {return QString::fromLatin1(headingMadeGoodStr);}

  static inline const char *sensorDepthStr = "sensorDepth";
  Q_PROPERTY(QString sensorDepth READ sensorDepth CONSTANT)
  QString sensorDepth() const {return QString::fromLatin1(sensorDepthStr);}

  static inline const char *sensorDepthOrigStr = "sensorDepthOrig";
  Q_PROPERTY(QString sensorDepthOrig READ sensorDepthOrig CONSTANT)
  QString sensorDepthOrig() const {return QString::fromLatin1(sensorDepthOrigStr);}

  static inline const char *attitudeStr = "attitude";
  Q_PROPERTY(QString attitude READ attitude CONSTANT)
  QString attitude() const {return QString::fromLatin1(attitudeStr);}

  /// GUI elements that set edit modes
  static inline const char *pickModeStr = "pickMode";
  Q_PROPERTY(QString pickMode READ pickMode CONSTANT)
  QString pickMode() const {return QString::fromLatin1(pickModeStr);}

  // Define editing mode 
  static inline const char *selectModeStr = "selectMode";
  Q_PROPERTY(QString selectMode READ selectMode CONSTANT)
  QString selectMode() const {return QString::fromLatin1(selectModeStr);}

  static inline const char *deselectModeStr = "deselectMode";
  Q_PROPERTY(QString deselectMode READ deselectMode CONSTANT)
  QString deselectMode() const {return QString::fromLatin1(deselectModeStr);}

  static inline const char *selectAllModeStr = "selectAllMode";
  Q_PROPERTY(QString selectAllMode READ selectAllMode CONSTANT)
  QString selectAllMode() const {return QString::fromLatin1(selectAllModeStr);}

  static inline const char *deselectAllModeStr = "deselectAllMode";
  Q_PROPERTY(QString deselectAllMode READ deselectAllMode CONSTANT)
  QString deselectAllMode() const {return QString::fromLatin1(deselectAllModeStr);}

  static inline const char *defineIntervalModeStr = "defineIntervalMode";
  Q_PROPERTY(QString defineIntervalMode READ defineIntervalMode CONSTANT)
  QString defineIntervalMode() const {return QString::fromLatin1(defineIntervalModeStr);}

  
};

#endif
