#ifndef GUINAMES_
#define GUINAMES_

#include <QObject>

class GuiNames : public QObject {

  Q_OBJECT

 public:
  GuiNames();

};

/// define names of C++-connected QML items
#define EDIT_MODES_NAME "editModesObj"
#define PING_STEP_SLIDER_NAME "pingStepSliderObj"
#define VERTICAL_EXAGG_SLIDER_NAME "verticalExaggSliderObj"
#define PINGS_SHOWN_SLIDER_NAME "pingsShownSliderObj"
#define XTRACK_SLIDER_NAME "xTrackSliderObj"
#define SWATH_PIXMAP_NAME "swathPixmapObj"


/// define names of C++-connected QML radio buttons
#define TOGGLE_EDIT_NAME "toggleEditObj"
#define PICK_EDIT_NAME "pickEditObj"
#define ERASE_EDIT_NAME "eraseEditObj"
#define RESTORE_EDIT_NAME "restoreEditObj"
#define GRAB_EDIT_NAME "grabEditObj"

/// define names of ancillary data options
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

/// define slice option names
#define ALONGTRACK_SLICE "alongTrack"
#define WATERFALL_SLICE "waterfall"
#define CROSSTRACK_SLICE "crossTrack"




#endif
