#ifndef GUINAMES_
#define GUINAMES_

#include <QObject>

class GuiNames : public QObject {

  Q_OBJECT

 public:
  GuiNames();

};

/// define names of C++-connected QML items, used when exchanging information
/// between QML GUI and C++ code.

/// Name of object created by QML, which holds swath images
#define SWATH_PIXMAP_NAME "swathPixmapObj"

/// Names of ancillary data options created by QML
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

#endif
