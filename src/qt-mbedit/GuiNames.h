#ifndef GUINAMES_
#define GUINAMES_

#include <QObject>


/**
   GuiNames defines QString properties used for objectName in QML GUI,
   defined to match the string literals referenced by Backend C++ methods.
*/
class GuiNames : public QObject {

  Q_OBJECT

public:
  
  GuiNames() {
  }

  /// Name of PixmapImage instantiated by QML, which holds swath graphs
  static inline const char *swathPixmapObjStr = "swathPixmapObj";
  Q_PROPERTY(QString swathPixmapObj READ swathPixmapObj CONSTANT)
  QString swathPixmapObj() const {return QString::fromLatin1(swathPixmapObjStr);}

  /// Names of ancillary data options created by QML.
  static inline const char *noneStr = "none";
  Q_PROPERTY(QString none READ none CONSTANT)
  QString none() const {return QString::fromLatin1(noneStr);}

  static inline const char *timeStr = "time";
  Q_PROPERTY(QString time READ time CONSTANT)
  QString time() const {return QString::fromLatin1(timeStr);}

  static inline const char *intervalStr = "interval";
  Q_PROPERTY(QString interval READ interval CONSTANT)
  QString interval() const {return QString::fromLatin1(intervalStr);}

  static inline const char *latitudeStr = "latitude";
  Q_PROPERTY(QString latitude READ latitude CONSTANT)
  QString latitude() const {return QString::fromLatin1(latitudeStr);}

  static inline const char *longitudeStr = "longitude";
  Q_PROPERTY(QString longitude READ longitude CONSTANT)
  QString longitude() const {return QString::fromLatin1(longitudeStr);}

  static inline const char *headingStr = "heading";
  Q_PROPERTY(QString heading READ heading CONSTANT)
  QString heading() const {return QString::fromLatin1(headingStr);}

  static inline const char *speedStr = "speed";
  Q_PROPERTY(QString speed READ speed CONSTANT)
  QString speed() const {return QString::fromLatin1(speedStr);}

  static inline const char *depthStr = "depth";
  Q_PROPERTY(QString depth READ depth CONSTANT)
  QString depth() const {return QString::fromLatin1(depthStr);}

  static inline const char *altitudeStr = "altitude";
  Q_PROPERTY(QString altitude READ altitude CONSTANT)
  QString altitude() const {return QString::fromLatin1(altitudeStr);}

  static inline const char *sensorDepthStr = "sensorDepth";
  Q_PROPERTY(QString sensorDepth READ sensorDepth CONSTANT)
  QString sensorDepth() const {return QString::fromLatin1(sensorDepthStr);}

  static inline const char *rollStr = "roll";
  Q_PROPERTY(QString roll READ roll CONSTANT)
  QString roll() const {return QString::fromLatin1(rollStr);}

  static inline const char *pitchStr = "pitch";
  Q_PROPERTY(QString pitch READ pitch CONSTANT)
  QString pitch() const {return QString::fromLatin1(pitchStr);}

  static inline const char *heaveStr = "heave";
  Q_PROPERTY(QString heave READ heave CONSTANT)
  QString heave() const {return QString::fromLatin1(heaveStr);}

  /// Names of slice options created by QML
  static inline const char *alongTrackStr = "alongTrack";
  Q_PROPERTY(QString alongTrack READ alongTrack CONSTANT)
  QString alongTrack() const {return QString::fromLatin1(alongTrackStr);}

  static inline const char *waterfallStr = "waterfall";
  Q_PROPERTY(QString waterfall READ waterfall CONSTANT)
  QString waterfall() const {return QString::fromLatin1(waterfallStr);}

  static inline const char *crossTrackStr = "crossTrack";
  Q_PROPERTY(QString crossTrack READ crossTrack CONSTANT)
  QString crossTrack() const {return QString::fromLatin1(crossTrackStr);}

  /// Names of color-coding options
  static inline const char *bottomDetectStr = "bottomDetect";
  Q_PROPERTY(QString bottomDetect READ bottomDetect CONSTANT)
  QString bottomDetect() const {return QString::fromLatin1(bottomDetectStr);}

  static inline const char *pulseSourceStr = "pulseSource";
  Q_PROPERTY(QString pulseSource READ pulseSource CONSTANT)
  QString pulseSource() const {return QString::fromLatin1(pulseSourceStr);}

  static inline const char *flagStateStr = "flagState";
  Q_PROPERTY(QString flagState READ flagState CONSTANT)
  QString flagState() const {return QString::fromLatin1(flagStateStr);}

  /// Names of edit modes
  static inline const char *toggleEditStr = "toggleEdit";
  Q_PROPERTY(QString toggleEdit READ toggleEdit CONSTANT)
  QString toggleEdit() const {return QString::fromLatin1(toggleEditStr);}

  static inline const char *pickEditStr = "pickEdit";
  Q_PROPERTY(QString pickEdit READ pickEdit CONSTANT)
  QString pickEdit() const {return QString::fromLatin1(pickEditStr);}

  static inline const char *eraseEditStr = "eraseEdit";
  Q_PROPERTY(QString eraseEdit READ eraseEdit CONSTANT)
  QString eraseEdit() const {return QString::fromLatin1(eraseEditStr);}

  static inline const char *restoreEditStr = "restoreEdit";
  Q_PROPERTY(QString restoreEdit READ restoreEdit CONSTANT)
  QString restoreEdit() const {return QString::fromLatin1(restoreEditStr);}

  static inline const char *grabEditStr = "grabEdit";
  Q_PROPERTY(QString grabEdit READ grabEdit CONSTANT)
  QString grabEdit() const {return QString::fromLatin1(grabEditStr);}

  static inline const char *infoEditStr = "infoEdit";
  Q_PROPERTY(QString infoEdit READ infoEdit CONSTANT)
  QString infoEdit() const {return QString::fromLatin1(infoEditStr);}


};

#endif
