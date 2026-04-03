#ifndef GUINAMES_
#define GUINAMES_

#include <QObject>

/// define names of C++-connected QML items, used when exchanging information
/// between QML GUI and C++ code.

/// Name of PixmapImage instantiated by QML, which holds swath graphs
#define SWATH_PIXMAP_NAME "swathPixmapObj"

/// Names of ancillary data options created by QML.
#define NONE_ANCILLDATA "none"
#define TIME_ANCILLDATA "time"
#define INTERVAL_ANCILLDATA "interval"
#define LATITUDE_ANCILLDATA "latitude"
#define LONGITUDE_ANCILLDATA "longitude"
#define HEADING_ANCILLDATA "heading"
#define SPEED_ANCILLDATA "speed"
#define DEPTH_ANCILLDATA "depth"
#define ALTITUDE_ANCILLDATA "altitude"
#define SENSORDEPTH_ANCILLDATA "sensorDepth"
#define ROLL_ANCILLDATA "roll"
#define PITCH_ANCILLDATA "pitch"
#define HEAVE_ANCILLDATA "heave"

/// Names of slice options created by QML
#define ALONGTRACK_SLICE "alongTrack"
#define WATERFALL_SLICE "waterfall"
#define CROSSTRACK_SLICE "crossTrack"

/// Names of color-coding options
#define BOTTOM_DETECT_COLOR "bottomDetect"
#define PULSE_SOURCE_COLOR "pulseSource"
#define FLAG_STATE_COLOR "flagState"

/// Names of edit modes
#define TOGGLE_EDIT_MODE "toggleEdit"
#define PICK_EDIT_MODE "pickEdit"
#define ERASE_EDIT_MODE "eraseEdit"
#define RESTORE_EDIT_MODE "restoreEdit"
#define GRAB_EDIT_MODE "grabEdit"
#define INFO_EDIT_MODE "infoEdit"

class GuiNames : public QObject {
  Q_OBJECT

public:
  
  GuiNames() {
  }
  
  
  Q_PROPERTY(QString swathPixmapObj READ swathPixmapObj CONSTANT)
  QString swathPixmapObj() const {return QStringLiteral(SWATH_PIXMAP_NAME);}

  Q_PROPERTY(QString none READ none CONSTANT)
  QString none() const {return QStringLiteral(NONE_ANCILLDATA);}

  Q_PROPERTY(QString time READ time CONSTANT)
  QString time() const {return QStringLiteral(TIME_ANCILLDATA);}

  Q_PROPERTY(QString interval READ interval CONSTANT)
  QString interval() const {return QStringLiteral(INTERVAL_ANCILLDATA);}

  Q_PROPERTY(QString latitude READ latitude CONSTANT)
  QString latitude() const {return QStringLiteral(LATITUDE_ANCILLDATA);}

  Q_PROPERTY(QString longitude READ longitude CONSTANT)
  QString longitude() const {return QStringLiteral(LONGITUDE_ANCILLDATA);}

  Q_PROPERTY(QString heading READ heading CONSTANT)
  QString heading() const {return QStringLiteral(HEADING_ANCILLDATA);}

  Q_PROPERTY(QString speed READ speed CONSTANT)
  QString speed() const {return QStringLiteral(SPEED_ANCILLDATA);}

  Q_PROPERTY(QString depth READ depth CONSTANT)
  QString depth() const {return QStringLiteral(DEPTH_ANCILLDATA);}

  Q_PROPERTY(QString altitude READ altitude CONSTANT)
  QString altitude() const {return QStringLiteral(ALTITUDE_ANCILLDATA);}

  Q_PROPERTY(QString sensorDepth READ sensorDepth CONSTANT)
  QString sensorDepth() const {return QStringLiteral(SENSORDEPTH_ANCILLDATA);}

  Q_PROPERTY(QString roll READ roll CONSTANT)
  QString roll() const {return QStringLiteral(ROLL_ANCILLDATA);}

  Q_PROPERTY(QString pitch READ pitch CONSTANT)
  QString pitch() const {return QStringLiteral(PITCH_ANCILLDATA);}

  Q_PROPERTY(QString heave READ heave CONSTANT)
  QString heave() const {return QStringLiteral(HEAVE_ANCILLDATA);}

  Q_PROPERTY(QString alongTrack READ alongTrack CONSTANT)
  QString alongTrack() const {return QStringLiteral(ALONGTRACK_SLICE);}

  Q_PROPERTY(QString waterfall READ waterfall CONSTANT)
  QString waterfall() const {return QStringLiteral(WATERFALL_SLICE);}

  Q_PROPERTY(QString crossTrack READ crossTrack CONSTANT)
  QString crossTrack() const {return QStringLiteral(CROSSTRACK_SLICE);}

  Q_PROPERTY(QString bottomDetect READ bottomDetect CONSTANT)
  QString bottomDetect() const {return QStringLiteral(BOTTOM_DETECT_COLOR);}

  Q_PROPERTY(QString pulseSource READ pulseSource CONSTANT)
  QString pulseSource() const {return QStringLiteral(PULSE_SOURCE_COLOR);}

  Q_PROPERTY(QString flagState READ flagState CONSTANT)
  QString flagState() const {return QStringLiteral(FLAG_STATE_COLOR);}

  Q_PROPERTY(QString toggleEdit READ toggleEdit CONSTANT)
  QString toggleEdit() const {return QStringLiteral(TOGGLE_EDIT_MODE);}

  Q_PROPERTY(QString pickEdit READ pickEdit CONSTANT)
  QString pickEdit() const {return QStringLiteral(PICK_EDIT_MODE);}

  Q_PROPERTY(QString eraseEdit READ eraseEdit CONSTANT)
  QString eraseEdit() const {return QStringLiteral(ERASE_EDIT_MODE);}

  Q_PROPERTY(QString restoreEdit READ restoreEdit CONSTANT)
  QString restoreEdit() const {return QStringLiteral(RESTORE_EDIT_MODE);}

  Q_PROPERTY(QString grabEdit READ grabEdit CONSTANT)
  QString grabEdit() const {return QStringLiteral(GRAB_EDIT_MODE);}

  Q_PROPERTY(QString infoEdit READ infoEdit CONSTANT)
  QString infoEdit() const {return QStringLiteral(INFO_EDIT_MODE);}
};

#endif
