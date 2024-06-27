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

/// Names of edit modes
#define PICK_MODENAME "pickMode"
#define SELECT_MODENAME "selectMode"
#define DESELECT_MODENAME "deselectMode"
#define SELECT_ALL_MODENAME "selectAllMode"
#define DESELECT_ALL_MODENAME "deselectAllMode"
#define DEFINE_INTERVAL_MODENAME "defineIntervalMode"



#endif
