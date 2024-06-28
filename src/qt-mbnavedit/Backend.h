#ifndef BACKEND_H
#define BACKEND_H
#include <iostream>
#include <QObject>
#include <QString>
#include <QQuickItem>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include "GuiNames.h"
#include "PixmapImage.h"
#include "CPixmapDrawer.h"
#include "Emitter.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
}

#define NUMBER_PLOTS_MAX 9
#define DEFAULT_PLOT_WIDTH 767
#define DEFAULT_PLOT_HEIGHT 300

#define MBNAVEDIT_BUFFER_SIZE 1000000

/** Backend application logic; QObject subclass, so it exchanges
    info with QML  */
class Backend : public QObject  {

  /// This macro needed to enable info exchange with QML
  Q_OBJECT

public:
  Backend(int argc, char **argv);
  ~Backend(void);

  /// Complete Backend initialization, load/display swath file if
  /// specified on command line
  bool initialize(QObject *loadedRoot, int argc, char **argv);

  /// Prepare for input of one or more specified swath data files
  static void parseDataList(char *file, int format) {
    std::cout << "parseDataList() not implemented\n";
    return;
  }

  int showError(const char *s1, const char *s2, const char *s3);

  int showMessage(const char *message);

  static int hideMessage(void) {
    std::cerr << "hideMessage() not implemented\n";
    return 0;
  }

  static void enableFileInput(void) {
    std::cerr << "enableFileInput\n";
  }

  static void disableFileInput(void) {
    std::cerr << "disableFileInput\n";
  }

  static void setUiElements() {
    std::cerr << "setUiElements() not implemented!\n";
  }

  /// Emit signals
  mb_system::Emitter emitter_;


protected:

  /// Mutually-exclusive edit modes 
  enum EditMode {
    Pick,
    Select,
    Deselect,
    SelectAll,
    DeselectAll,
    DefineInterval
    
  };
  
  struct Ping {
    int id;
    int record;
    int time_i[7];
    double time_d;
    double file_time_d;
    double tint;
    double lon;
    double lat;
    double speed;
    double heading;
    double draft;
    double roll;
    double pitch;
    double heave;
    double time_d_org;
    double tint_org;
    double lon_org;
    double lat_org;
    int mean_ok;
    double lon_dr;
    double lat_dr;
    double speed_org;
    double heading_org;
    double draft_org;
    double speed_made_good;
    double course_made_good;
    int tint_x;
    int tint_y;
    int lon_x;
    int lon_y;
    int lat_x;
    int lat_y;
    int speed_x;
    int speed_y;
    int heading_x;
    int heading_y;
    int draft_x;
    int draft_y;
    int tint_select;
    int lon_select;
    int lat_select;
    int speed_select;
    int heading_select;
    int draft_select;
    int lonlat_flag;
  };

  /* plot structure definition */
  struct Plot {
    int type;
    int ixmin;
    int ixmax;
    int iymin;
    int iymax;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double xscale;
    double yscale;
    double xinterval;
    double yinterval;
    char xlabel[128];
    char ylabel1[128];
    char ylabel2[128];
  };

  /// Process specified swath file
  Q_INVOKABLE bool processSwathFile(QUrl fileUrl);

  /// Parse input data file(s)
  void parseInputDataList(char *file, int format);
  
  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  

  /// world -> device xScale factor
  double xScale_ = 1;
  
  /// world -> device yScale factor
  double yScale_ = 1;
  
  /// UI root object
  QObject *ui_;

  /// QML-declared PixmapImage
  mb_system::PixmapImage *swathPixmapImage_;
  
  /// Test drawing to canvas
  bool plotTest(void);

  /// Do appropriate swath edit action at specified location, depending
  /// on edit-mode
  bool edit(double x, double y);

  /// Input swath file name
  char inputFilename_[256];

  /// Pixmap representation of swath data graph
  QPixmap *canvasPixmap_;

  /// painter_ draws into canvasPixmap_
  QPainter *painter_;

  
  /* ------- FORMER extern mbnavedit global variables ------- */
  /* mbnavedit global control parameters */
  int outputMode_;
  bool runMBProcess_;
  bool guiMode_;
  int dataShowMax_;
  int dataShowSize_;
  int dataStepMax_;
  int dataStepSize_;
  EditMode editMode_ = Pick;
  int modeSetInterval_;
  bool plotTint_;
  bool plotTintOrig_;
  bool plotLon_;
  bool plotLonOrig_;
  bool plotLonDr_;
  bool plotLat_;
  bool plotLatOrig_;
  bool plotLatDr_;
  bool plotSpeed_;
  bool plotSpeedOrig_;
  bool plotSmg_;
  bool plotHeading_;
  bool plotHeadingOrig_;
  bool plotCmg_;
  bool plotDraft_;
  bool plotDraftOrig_;
  bool plotDraftDr_;
  bool plotRoll_;
  bool plotPitch_;
  bool plotHeave_;
  int meanTimeWindow_;
  int driftLon_;
  int driftLat_;
  bool timestampProblem_;  
  bool usePingData_;  
  bool stripComments_;
  int format_;
  char ifile_[MB_PATH_MAXLINE];
  char nfile_[MB_PATHPLUS_MAXLINE];
  int modelMode_;
  double weightSpeed_;
  double weightAccel_;
  int scrollCount_;
  double offsetLon_;
  double offsetLat_;
  double offsetLonApplied_;
  double offsetLatApplied_;

  /* mbnavedit plot size parameters */
  int plotWidth_;
  int plotHeight_;
  int nPlots_;

  /* ------- FORMER mbnavedit_prog  global variables ------- */
  /* id variables */
  const char *programName_;
  const char *helpMessage_;
  const char *usageMessage_;

  /* status variables */
  int error_ = MB_ERROR_NO_ERROR;
  int verbose_ = 0;
  char *message_ = NULL;

  /* MBIO control parameters */
  int platformSource_;
  int navSource_;
  int sensorDepthSource_;
  int headingSource_;
  int attitudeSource_;
  int svpSource_;
  int nPings_;
  int lonFlip_;
  double bounds_[4];
  int btime_i_[7];
  int etime_i_[7];
  double btime_d_;
  double etime_d_;
  double speedMin_;
  double timeGap_;
  int beamsBath_;
  int beamsAmp_;
  int pixelsSS_;
  void *imbioPtr_ = NULL;
  bool useLockFiles_ = true;

  /* mbio read and write values */
  void *storePtr_ = NULL;
  int kind_;
  double distance_;
  double altitude_;
  double sensorDepth_;
  int nbath_;
  int namp_;
  int nss_;
  char *beamFlag_ = NULL;
  double *bath_ = NULL;
  double *bathAcrossTrack_ = NULL;
  double *bathAlongTrack_ = NULL;
  double *amp_ = NULL;
  double *ss_ = NULL;
  double *ssAcrossTrack_ = NULL;
  double *ssAlongTrack_ = NULL;
  char comment_[MB_COMMENT_MAXLINE];

  /* buffer control variables */
  bool fileOpen_ = false;
  bool nfileOpen_ = false;
  FILE *nfp_;
  int holdSize_ = 100;
  int nLoad_ = 0;
  int nDump_ = 0;
  int nBuff_ = 0;
  int currentId_ = 0;
  int nLoadTotal_ = 0;
  int nDumpTotal_ = 0;
  bool firstRead_ = false;

  /* plotting control variables */
  /// Array of Ping structures; array size is very large so
  /// make this member static to prevent memory overflow 
  static Backend::Ping ping_[MBNAVEDIT_BUFFER_SIZE];
  double plotStartTime_;
  double plotEndTime_;
  int nPlot_;
  Backend::Plot plot_[NUMBER_PLOTS_MAX];
  int dataSave_;
  double fileStarttime_d_;


  // Member functions, formerly mbnavedit_prog stand-alone C functions 
  int init(int argc, char **argv, bool *inputSpecd);
  
  int set_graphics(void *xgid, int ncol);

  int init_globals(void);
  
  int clear_screen(void);

  int action_open(bool useprevious);

  int open_file(bool useprevious);

  int close_file(void);

  int dump_data(int hold);


  int load_data(void);

  int action_next_buffer(bool *quit);


  int action_offset(void);

  int action_close(void);

  int action_done(bool *quit);

  int action_quit(void);

  int action_step(int step);

  int action_end(void);

  int action_start(void);

  int action_mouse_pick(int xx, int yy);

  int action_mouse_select(int xx, int yy);

  int action_mouse_deselect(int xx, int yy);

  int action_mouse_selectall(int xx, int yy);

  int action_mouse_deselectall(int xx, int yy);

  int action_deselect_all(int type);

  int action_set_interval(int xx, int yy, int which);

  int action_use_dr(void);

  int action_use_smg(void);

  int action_use_cmg(void);

  int action_interpolate(void);

  int action_interpolate_repeats(void);

  int action_revert(void);

  int action_flag(void);

  int action_unflag(void);

  int action_fixtime(void);

  int action_deletebadtime(void);

  int action_showall(void);

  int get_smgcmg(int i);

  int get_model(void);

  int get_gaussianmean(void);

  int get_dr(void);

  int get_inversion(void);

  int plot_all(void);

  int plot_tint(int iplot);

  int plot_lon(int iplot);

  int plot_lat(int iplot);

  int plot_speed(int iplot);

  int plot_heading(int iplot);

  int plot_draft(int iplot);

  int plot_roll(int iplot);

  int plot_pitch(int iplot);

  int plot_heave(int iplot);

  int plot_tint_value(int iplot, int iping);

  int plot_lon_value(int iplot, int iping);

  int plot_lat_value(int iplot, int iping);

  int plot_speed_value(int iplot, int iping);

  int plot_heading_value(int iplot, int iping);

  int plot_draft_value(int iplot, int iping);

						      
public slots:
  // These 'slots' can be invoked directly by QML code, i.e. not
  // necessarily connected by signals
  
  /// Methods called by QML code
  void setPlot(QString plotName, bool set);

  /// Called when edit mode changed
  void onEditModeChanged(QString modeName);

  void onLeftButtonClicked(int x, int y);

  void onRightButtonClicked(int x, int y);

  void onMiddleButtonClicked(int x, int y);

  void onMouseMoved(int x, int y);

  /// Invoked by QML when resize occurs; sets xScale_ and yScale member values
  void onPixmapImageResize(int width, int height);
    
  /// Reset time interval
  void onResetInterval(void);

  /// Move swath view to start
  void onGoStart(void);

  /// Move swath view forward
  void onGoForward(void);

  /// Move swath view back
  void onGoBack(void);

  /// Move swath view to end
  void onGoEnd(void);

  // Interpolate around selected points
  void onInterpolate(void);

  void onInterpolateRepeat();
  
  /// Invoked when main window is destroyed
  void onMainWindowDestroyed(void);


};


#endif // BACKEND_H
