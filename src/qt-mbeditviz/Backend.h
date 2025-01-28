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
#include "mb_info.h"
#include "mb_process.h"
}


/* MBeditviz defines */
#define MBEV_GRID_NONE 0
#define MBEV_GRID_NOTVIEWED 1
#define MBEV_GRID_VIEWED 2
#define MBEV_GRID_ALGORITH_SIMPLE 0
#define MBEV_GRID_ALGORITH_FOOTPRINT 1
#define MBEV_GRID_WEIGHT_TINY 0.0000001
#define MBEV_ALLOC_NUM 24
#define MBEV_ALLOCK_NUM 1024
#define MBEV_NODATA -10000000.0
#define MBEV_NUM_ESF_OPEN_MAX 25


/** qt-mbeditviz backend application logic; QObject subclass, so it can exchange
    info with QML  */
class Backend : public QObject  {

  /// This macro needed to enable info exchange with QML
  Q_OBJECT

public:
  Backend(int argc, char **argv);
  ~Backend(void);

protected:

typedef enum {
     MBEV_GRID_ALGORITHM_SIMPLEMEAN = 0,
     MBEV_GRID_ALGORITHM_FOOTPRINT = 1,
     MBEV_GRID_ALGORITHM_SHOALBIAS = 2,
 } gridalgorithm_t;

typedef enum {
     MBEV_OUTPUT_MODE_EDIT = 0,
     MBEV_OUTPUT_MODE_BROWSE = 1,
 } output_mode_t;


/* usage of footprint based weight */
#define MBEV_USE_NO 0
#define MBEV_USE_YES 1
#define MBEV_USE_CONDITIONAL 2

  /* mbeditviz structures */
  struct Ping {
    int time_i[7];
    double time_d;
    int multiplicity;
    double navlon;
    double navlat;
    double navlonx;
    double navlaty;
    double portlon;
    double portlat;
    double stbdlon;
    double stbdlat;
    double speed;
    double heading;
    double distance;
    double altitude;
    double sensordepth;
    double draft;
    double roll;
    double pitch;
    double heave;
    double ssv;
    int beams_bath;
    char *beamflag;
    char *beamflagorg;
    int *beamcolor;
    double *bath;
    double *amp;
    double *bathacrosstrack;
    double *bathalongtrack;
    double *bathcorr;
    double *bathlon;
    double *bathlat;
    double *bathx;
    double *bathy;
    double *angles;
    double *angles_forward;
    double *angles_null;
    double *ttimes;
    double *bheave;
    double *alongtrack_offset;
  };

  
  struct File {
    int load_status;
    int load_status_shown;
    bool locked;
    bool esf_exists;
    char name[MB_PATH_MAXLINE];
    char path[MB_PATH_MAXLINE];
    int format;
    int raw_info_loaded;
    int processed_info_loaded;
    struct mb_info_struct raw_info;
    struct mb_info_struct processed_info;
    struct mb_process_struct process;
    bool esf_open;
    bool esf_changed;
    char esffile[MB_PATH_MAXLINE];
    struct mb_esf_struct esf;
    int num_pings;
    int num_pings_alloc;
    struct mbev_ping_struct *pings;
    double beamwidth_xtrack;
    double beamwidth_ltrack;
    int topo_type;
    int n_async_heading;
    int n_async_heading_alloc;
    double *async_heading_time_d;
    double *async_heading_heading;
    int n_async_sensordepth;
    int n_async_sensordepth_alloc;
    double *async_sensordepth_time_d;
    double *async_sensordepth_sensordepth;
    int n_async_attitude;
    int n_async_attitude_alloc;
    double *async_attitude_time_d;
    double *async_attitude_roll;
    double *async_attitude_pitch;
    int n_sync_attitude;
    int n_sync_attitude_alloc;
    double *sync_attitude_time_d;
    double *sync_attitude_roll;
    double *sync_attitude_pitch;
  };

  
  struct Grid {
    int status;
    char projection_id[MB_PATH_MAXLINE];
    void *pjptr;

    /// minimum lat, maximum lat, minimum lon, maximum lon
    double bounds[4];

    /// minimum northing, maximum northing, minimum easting, maximum easting
    double boundsutm[4];

    /// Grid easting increment (meters)
    double dx;

    /// Grid northing increment (meters)
    double dy;

    int n_columns;
    int n_rows;

    /// minimum depth
    double min;

    /// maximum depth
    double max;

    double smin;

    double smax;

    /// Value denoting 'no data'
    float nodatavalue;

    float *sum;
    float *wgt;

    /// Depth values
    float *val;

    float *sgm;
  };


  /// Adapted from mbview/mb3dsoundings_sounding_struct  
  struct Sounding {
    int ifile;
    int iping;
    int ibeam;
    int beamcolor;
    char beamflag;
    char beamflagorg;
    double x;
    double y;
    double z;
    double a;
    float glx;
    float gly;
    float glz;
    float r;
    float g;
    float b;
    int winx;
    int winy;
  };

  /// Adapted from mbview/mb3dsoundings_soundings_struct
  struct Soundings {
    /* display flag */
    bool displayed;

    /* location and scale parameters */
    double xorigin;
    double yorigin;
    double zorigin;
    double xmin;
    double ymin;
    double zmin;
    double xmax;
    double ymax;
    double zmax;
    double bearing;
    double sinbearing;
    double cosbearing;
    double scale;
    double zscale;

    /* sounding data */
    int num_soundings;
    int num_soundings_unflagged;
    int num_soundings_flagged;
    int num_soundings_alloc;
    Sounding *soundings;
  };
  
  
  // ------- FORMER extern mbnavedit global variables
  int status_;
  int error_;
  int verbose_;

  /* mode parameters */
  int mode_output_;

  /* data parameters */
  int num_files_;
  int num_files_alloc_;
  int num_esf_open_;
  struct File *files_;
  Grid grid_;
  size_t instance_;

  /* gridding parameters */
  double grid_bounds_[4];
  double grid_boundsutm_[4];
  double grid_cellsize_;
  gridalgorithm_t grid_algorithm_;
  int grid_interpolation_;
  int grid_n_columns_;
  int grid_n_rows_;

  /* global patch test parameters */
  double rollBias_;
  double pitchBias_;
  double headingBias_;
  double timeLag_;
  double snell_;

  /* sparse voxel filter parameters */
  int sizeMultiplier_;
  int nSoundingThreshold_;

  /// selected sounding parameters
 Sounding selected_;

  /// MBIO control parameters
  int format_;

  /// 0: no longitude flip, 1: longitude flip
  int lonFlip_;

  /// Use lock files?
  bool useLockFiles_;
  
  // These methods derived from mbeditviz_prog functions
  int init(int argc, char **argv,
	   char *programName,
	   char *helpMsg,
	   char *usageMsg,
	   int (*showMessage)(char *),
	   int (*hideMessage)(void),
	   void (*updateGui)(void),
	   int (*showErrorDialog)(char *, char *, char *));

  int get_format(char *file, int *form);
  int open_data(char *path, int format);

  /// Read list of relevant files into global mbev_files array
  int import_file(char *path, int format);

  /// Read swath data from specified file into global mbev_file array element
  int load_file(int ifile, bool assertLock);

  int apply_biasesandtimelag(struct File *file, 
			     Ping *ping, double rollbias,
			     double pitchbias,
			     double headingbias, double timelag,
			     double *headingdelta, double *sensordepth,
			     double *rolldelta,
			     double *pitchdelta);

  int snell_correction(double snell, double roll, double *beam_xtrack,
		       double *beam_ltrack, double *beam_z);

  int beam_position(double navlon, double navlat, double mtodeglon,
		    double mtodeglat, double rawbath, double acrosstrack,
		    double alongtrack, double sensordepth, double rolldelta,
		    double pitchdelta, double heading,
		    double *bathcorr, double *lon, double *lat);

  int unload_file(int ifile, bool assertUnlock);

  int delete_file(int ifile);

  double erf(double x);

  int bin_weight(double foot_a, double foot_b, double scale, double pcx,
		 double pcy, double dx, double dy, double *px,
		 double *py, double *weight, int *use);

  /// Read grid bounds of loaded files into global mbev_grid_bounds array
  int get_grid_bounds(void);

  /// Setup the grid to contain loaded files
  int setup_grid(void);

  /// Allocate and load individual swath soundings
  int project_soundings(void);

  /// Create the grid to containing loaded files
  int make_grid(void);

  int grid_beam(File *file, Ping *ping,
		int ibeam, bool beam_ok, bool apply_now);

  int make_grid_simple(void);
  
  int destroy_grid(void);
  
  int selectregion(size_t instance);

  int selectarea(size_t instance);

  int selectnav(size_t instance);

  void mb3dsoundings_dismiss(void);

  void mb3dsoundings_edit(int ifile, int iping, int ibeam, char beamflag,
			  int flush);

  void mb3dsoundings_info(int ifile, int iping, int ibeam, char *infostring);

  void mb3dsoundings_bias(double rollbias, double pitchbias,
			  double headingbias, double timelag, double snell);

  void mb3dsoundings_biasapply(double rollbias, double pitchbias,
			       double headingbias, double timelag,
			       double snell);
  
  void mb3dsoundings_flagsparsevoxels(int sizemultiplier,
				      int nsoundingthreshold);

  void mb3dsoundings_colorsoundings(int color);

  void mb3dsoundings_optimizebiasvalues(int mode, double *rollbias,
					double *pitchbias, double *headingbias,
					double *timelag, double *snell);
  
  void mb3dsoundings_getbiasvariance(double local_grid_xmin,
				     double local_grid_xmax,
				     double local_grid_ymin,
				     double local_grid_ymax,
				     int local_grid_nx, int local_grid_ny,
				     double local_grid_dx,
				     double local_grid_dy,
				     double *local_grid_first,
				     double *local_grid_sum,
				     double *local_grid_sum2,
				     double *local_grid_variance,
				     int *local_grid_num,
				     double rollbias, double pitchbias,
				     double headingbias, double timelag,
				     double snell,
				     int *variance_total_num,
				     double *variance_total);


};


#endif // BACKEND_H
