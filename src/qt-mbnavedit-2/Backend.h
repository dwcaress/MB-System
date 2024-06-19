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
#define MBNAVEDIT_PICK_DISTANCE 50
#define MBNAVEDIT_ERASE_DISTANCE 10
#define MBNAVEDIT_BUFFER_SIZE 1000000

/** Backend application logic; QObject subclass, so it exchanges
    info with QML  */
class Backend : public QObject  {

  /// This macro needed to enable info exchange with QML
  Q_OBJECT

public:
  Backend(int argc, char **argv);
  ~Backend();

  /// Complete Backend initialization, load/display swath file if
  /// specified on command line
  bool initialize(QObject *loadedRoot, int argc, char **argv);

  /// Prepare for input of one or more specified swath data files
  static void parseDataList(char *file, int format) {
    std::cout << "parseDataList() not implemented\n";
    return;
  }

  static int showError(const char *s1, const char *s2, const char *s3) {
    std::cerr << "showError(): " << s1 << "\n" << s2 << "\n" << s3 << "\n";
    char msg[256];
    sprintf(msg, "%s\n%s\n%s\n", s1, s2, s3);
    emit staticEmitter_.showMessage(QVariant(msg));
    return 0;
  }

  static int showMessage(const char *message) {
    std::cerr << "showMessage(): " << message << "\n";
    emit staticEmitter_.showMessage(QVariant(message));
    
    return 0;
  }

  static int hideMessage(void) {
    std::cerr << "hideMessage()\n";
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

  /// Emit signals on behalf of static member functions
  static mb_system::Emitter staticEmitter_;


protected:

    struct mbnavedit_ping_struct {
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
  struct mbnavedit_plot_struct {
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

  /// Plot current swath file
  bool plotSwath(void);

  /// Parse input data file(s)
  void parseInputDataList(char *file, int format);
  
  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  
  
  /// UI root object
  QObject *ui_;

  /// QML-declared PixmapImage
  mb_system::PixmapImage *swathPixmapImage_;
  
  /// Test drawing to canvas
  bool plotTest(void);

  /// Do appropriate swath edit action at specified location, depending
  /// on edit-mode
  bool edit(double x, double y);

  /// Dummy first argument to canvas-drawing member funtions
  void *dummy_;

  /// Input swath file name
  char inputFilename_[256];

  /// Pixmap representation of swath data graph
  QPixmap *canvasPixmap_;

  /// painter_ draws into canvasPixmap_
  QPainter *painter_;

  /// Indicates if data is plotted
  bool dataPlotted_;

  /* ------- FORMER extern mbnavedit global variables ------- */
/* mbnavedit global control parameters */
  int outputMode_;
  bool runMBProcess_;
  bool guiMode_;
  int dataShowMax_;
  int dataShowSize_;
  int dataStepMax_;
  int dataStepSize_;
  int modePick_;
  int modeSetInterval_;
  int plotTint_;
  int plotTintOrg_;
  int plotLon_;
  int plotLonOrg_;
  int plotLonDr_;
  int plotLat_;
  int plotLatOrg_;
  int plotLatDr_;
  int plotSpeed_;
  int plotSpeedOrg_;
  int plotSmg_;
  int plotHeading_;
  int plotHeadingOrg_;
  int plotCmg_;
  int plotDraft_;
  int plotDraftOrg_;
  int plotDraftDr_;
  int plotRoll_;
  int plotPitch_;
  int plotHeave_;
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

  /* ------- FORMER mnavedit_prog  global variables ------- */
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
#define MBNAVEDIT_BUFFER_SIZE 1000000
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
#define NUMBER_PLOTS_MAX 9
#define DEFAULT_PLOT_WIDTH 767
#define DEFAULT_PLOT_HEIGHT 300
#define MBNAVEDIT_PICK_DISTANCE 50
#define MBNAVEDIT_ERASE_DISTANCE 10
 struct mbnavedit_ping_struct ping[MBNAVEDIT_BUFFER_SIZE];
    double plotStartTime_;
 double plotEndTime_;
    int nPlot_;
 void *mbnaveditXgid_;
 struct mbnavedit_plot_struct mbnavplot[NUMBER_PLOTS_MAX];
 int dataSave_;
 double fileStarttime_d_;

 int nColors_;
  


  // Member functions, formerly mbnavedit_prog stand-alone C functions 

  int mbnavedit_init(int argc, char **argv, bool *inputSpecd);
  
  int mbnavedit_set_graphics(void *xgid, int ncol);

  int mbnavedit_init_globals(void);
  
  int mbnavedit_clear_screen(void);

  int mbnavedit_action_open(bool useprevious);

  int mbnavedit_open_file(bool useprevious);

  int mbnavedit_close_file(void);

  int mbnavedit_dump_data(int hold);


  int mbnavedit_load_data(void);

  int mbnavedit_action_next_buffer(bool *quit);


  int mbnavedit_action_offset(void);

  int mbnavedit_action_close(void);

  int mbnavedit_action_done(bool *quit);

  int mbnavedit_action_quit(void);

  int mbnavedit_action_step(int step);

  int mbnavedit_action_end(void);

  int mbnavedit_action_start(void);

  int mbnavedit_action_mouse_pick(int xx, int yy);

  int mbnavedit_action_mouse_select(int xx, int yy);

  int mbnavedit_action_mouse_deselect(int xx, int yy);

  int mbnavedit_action_mouse_selectall(int xx, int yy);

  int mbnavedit_action_mouse_deselectall(int xx, int yy);

  int mbnavedit_action_deselect_all(int type);

  int mbnavedit_action_set_interval(int xx, int yy, int which);

  int mbnavedit_action_use_dr(void);

  int mbnavedit_action_use_smg(void);

  int mbnavedit_action_use_cmg(void);

  int mbnavedit_action_interpolate(void);

  int mbnavedit_action_interpolaterepeats(void);

  int mbnavedit_action_revert(void);

  int mbnavedit_action_flag(void);

  int mbnavedit_action_unflag(void);

  int mbnavedit_action_fixtime(void);

  int mbnavedit_action_deletebadtime(void);

  int mbnavedit_action_showall(void);

  int mbnavedit_get_smgcmg(int i);

  int mbnavedit_get_model(void);

  int mbnavedit_get_gaussianmean(void);

  int mbnavedit_get_dr(void);

  int mbnavedit_get_inversion(void);

  int mbnavedit_plot_all(void);

  int mbnavedit_plot_tint(int iplot);

  int mbnavedit_plot_lon(int iplot);

  int mbnavedit_plot_lat(int iplot);

  int mbnavedit_plot_speed(int iplot);

  int mbnavedit_plot_heading(int iplot);

  int mbnavedit_plot_draft(int iplot);

  int mbnavedit_plot_roll(int iplot);

  int mbnavedit_plot_pitch(int iplot);

  int mbnavedit_plot_heave(int iplot);

  int mbnavedit_plot_tint_value(int iplot, int iping);

  int mbnavedit_plot_lon_value(int iplot, int iping);

  int mbnavedit_plot_lat_value(int iplot, int iping);

  int mbnavedit_plot_speed_value(int iplot, int iping);

  int mbnavedit_plot_heading_value(int iplot, int iping);

  int mbnavedit_plot_draft_value(int iplot, int iping);

						      
public slots:
  // These 'slots' can be invoked directly by QML code, i.e. not connected by
  // signals
  
  /// Methods called by QML code

  /// Invoked when main window is destroyed
  void onMainWindowDestroyed(void);

};


#endif // BACKEND_H
