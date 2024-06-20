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
///
/// N.B. TAKE CARE THAT QML USES THE LITERAL STRINGS DEFINED HERE FOR
/// objectName

/// Name of PixmapImage instantiated by QML, which holds swath graphs
#define SWATH_PIXMAP_NAME "swathPixmapObj"

/// Plot names
#define TIMEINT_PLOTNAME "timeInt"
#define LAT_PLOTNAME "lat"
#define LON_PLOTNAME "lon"
#define SPEED_PLOTNAME "speed"
#define HEADING_PLOTNAME "heading"
#define SENSORDEPTH_PLOTNAME "sensorDepth"
#define ATTITUDE_PLOTNAME "attitude"





/// Names of edit modes
#define TOGGLE_EDIT_MODE "toggleEdit"
#define PICK_EDIT_MODE "pickEdit"
#define ERASE_EDIT_MODE "eraseEdit"
#define RESTORE_EDIT_MODE "restoreEdit"
#define GRAB_EDIT_MODE "grabEdit"
#define INFO_EDIT_MODE "infoEdit"

#endif
