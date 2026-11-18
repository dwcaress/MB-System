/*--------------------------------------------------------------------
 *    The MB-system:  mbe_core.h  2/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The editing session of mbedit-qt, without Qt and without global state:
 * reading pings through MBIO, the edit actions, the filters, the layout of
 * the plot in pixels, and the edit save files. The behaviour follows
 * src/mbedit/mbedit_prog.c; the differences are listed in the mbedit-qt
 * manual page. Setting compatibility mode restores the old behaviour where
 * the two differ, so that the same session can be compared with the old code.
 */

#ifndef MBEDIT_QT_MBE_CORE_H
#define MBEDIT_QT_MBE_CORE_H

#include <atomic>
#include <cstdio>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"

namespace mbe {

constexpr char PROGRAM_NAME[] = "mbedit-qt";
constexpr int DEFAULT_BUFFER_SIZE = 30000;
constexpr int MAX_PLOT_PINGS = 250;
constexpr int MAX_PLOT_WIDTH = 40000;  // m, 20 km to each side
constexpr int PICK_DISTANCE = 50;
constexpr int ERASE_DISTANCE = 15;

enum OutputMode { OUTPUT_EDIT = 1, OUTPUT_BROWSE = 2 };
enum Outbounds { OUTBOUNDS_NONE = 0, OUTBOUNDS_FLAGGED = 1, OUTBOUNDS_UNFLAGGED = 2 };
enum PlotMode {
  PLOT_WIDE = 0,
  PLOT_TIME = 1,
  PLOT_INTERVAL = 2,
  PLOT_LON = 3,
  PLOT_LAT = 4,
  PLOT_HEADING = 5,
  PLOT_SPEED = 6,
  PLOT_DEPTH = 7,
  PLOT_ALTITUDE = 8,
  PLOT_SENSORDEPTH = 9,
  PLOT_ROLL = 10,
  PLOT_PITCH = 11,
  PLOT_HEAVE = 12
};
enum ViewMode { VIEW_WATERFALL = 0, VIEW_ALONGTRACK = 1, VIEW_ACROSSTRACK = 2 };
enum ShowMode { SHOW_FLAG = 0, SHOW_DETECT = 1, SHOW_PULSE = 2, SHOW_AMPLITUDE = 3, SHOW_ANGLE = 4, SHOW_RESIDUAL = 5 };
enum GrabMode { GRAB_START = 0, GRAB_MOVE = 1, GRAB_END = 2 };

struct Ping {
  int record = 0;
  int outbounds = OUTBOUNDS_NONE;
  int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
  double time_d = 0.0;
  int multiplicity = 0;
  bool dualprofile = false;
  int dualprofilebeam = 0;
  double time_interval = 0.0;
  double navlon = 0.0;
  double navlat = 0.0;
  double speed = 0.0;
  double heading = 0.0;
  double altitude = 0.0;
  double sensordepth = 0.0;
  double roll = 0.0;
  double pitch = 0.0;
  double heave = 0.0;
  double distance = 0.0;
  int beams_bath = 0;
  bool has_amp = false;
  std::vector<char> beamflag;         // now
  std::vector<char> beamflagorg;      // in the data file
  std::vector<char> beamflagsession;  // at the last save, or when loaded with old edits applied
  std::vector<double> bath;
  std::vector<double> amp;
  std::vector<double> bathacrosstrack;
  std::vector<double> bathalongtrack;
  std::vector<unsigned char> detect;
  std::vector<unsigned char> priority;
  std::vector<signed char> pulses;
  std::vector<int> bath_x;  // pixels, only for the pings in the view
  std::vector<int> bath_y;
  std::string label;
  int label_x = 0;
  int label_y = 0;
  int label_width = 0;
  int zap_x1 = 0;
  int zap_x2 = 0;
  int zap_y1 = 0;
  int zap_y2 = 0;
};

struct Filters {
  bool medianspike = false;
  int medianspike_threshold = 10;  // percent of the altitude
  int medianspike_xtrack = 5;      // beams
  int medianspike_ltrack = 1;      // pings
  bool wrongside = false;
  int wrongside_threshold = 15;  // beams from the center
  bool cutbeam = false;
  int cutbeam_begin = 0;
  int cutbeam_end = 0;
  bool cutdistance = false;
  double cutdistance_begin = 0.0;  // m
  double cutdistance_end = 0.0;
  bool cutangle = false;
  double cutangle_begin = 0.0;  // degrees
  double cutangle_end = 0.0;
  bool any() const { return medianspike || wrongside || cutbeam || cutdistance || cutangle; }
};

/* what the plot needs from the user interface */
struct PlotSettings {
  int plot_width = 5000;  // m
  int exagger = 100;      // vertical exaggeration x 100
  int x_interval = 1000;  // m
  int y_interval = 250;   // m
  int plot_size = 10;     // pings shown
  int show_mode = SHOW_FLAG;
  bool show_flaggedsoundings = true;
  bool show_flaggedprofiles = false;
  int show_time = PLOT_TIME;
};

struct PlotBox {
  int width = 1016;
  int height = 525;
  int margin = 0;
  int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
};

struct BeamInfo {
  bool set = false;
  int ping = 0;
  int beam = 0;
  int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
  double time_d = 0.0;
  double navlon = 0.0, navlat = 0.0, speed = 0.0, heading = 0.0, altitude = 0.0;
  int beams_bath = 0;
  char beamflag = 0;
  double bath = 0.0, amp = 0.0, bathacrosstrack = 0.0, bathalongtrack = 0.0;
  int detect = 0, priority = 0, pulse = 0;
};

/* why a sounding is flagged, for showing and unflagging by reason */
enum FlagReason { REASON_MANUAL = 1, REASON_FILTER = 2, REASON_SONAR = 4, REASON_ALL = 7 };
enum Scope { SCOPE_VIEW = 0, SCOPE_FORWARD = 1, SCOPE_BUFFER = 2 };

/* what would change, before it does */
struct FlagPreview {
  struct Item {
    int ping;
    int beam;
    char after;
  };
  std::vector<Item> items;
  long nflag = 0;
  long nunflag = 0;
  int npings = 0;
  bool complete = true;  // false if the edits reach beyond the pings in memory
};

/* a ping worth a look, and why */
struct Suspect {
  int ping = 0;
  double score = 0.0;
  int spikes = 0;
  int reference_spikes = 0;  // soundings far from the reference surface
  double deficit = 0.0;  // fraction of good soundings missing against the neighbouring pings
  bool attitude_jump = false;
  bool time_gap = false;
};

struct InputFile {
  std::string path;
  int format = 0;
};

/* a file, or the files of a datalist when the format is -1, as the -I option takes them */
bool parse_input(const std::string &path, int format, std::vector<InputFile> &files, std::string &error);

class Session {
 public:
  Session();
  ~Session();
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  /* set before opening a file */
  std::string program_name = PROGRAM_NAME;  // written in lock files and edit save files
  int verbose = 0;
  int lonflip = 0;
  double bounds[4] = {-360.0, 360.0, -90.0, 90.0};
  int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
  int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
  double speedmin = 0.0;
  double timegap = 1000000000.0;
  bool gui_mode = false;
  void setBuffer(int size, int hold);
  /* the old behaviour where it differs, to compare with the old code */
  bool compatibility = false;

  /* notices for the user interface */
  std::function<void(const std::string &, const std::string &, const std::string &)> on_error;
  std::function<void(const std::string &)> on_progress;
  /* called after a save, once the file is unlocked (for -X) */
  std::function<void(const std::string &)> on_saved;
  /* the messages of the session; they go to stderr if this is not set */
  std::function<void(const std::string &)> on_log;
  /* the width of a text in the plot, in pixels */
  std::function<int(const char *)> text_width;
  int text_ascent = 11;
  int text_descent = 2;

  /* files */
  int open(const std::string &path, int format, bool use_saved_edits, int output_mode, bool recover = false);
  int save();         // write the edits now and keep the file open
  int closeSave();    // write the edits and close (Done, Quit)
  int closeDiscard(); // close and leave the edit save file as it was
  int done(bool *quit);
  int nextBuffer(bool *quit);
  int gotoTime(int time_i[7]);
  bool isOpen() const { return file_open_; }
  /* the pings of this file that left the buffer before the first one in memory */
  long pingsBefore() const { return pings_before_; }
  bool moreData() const { return nbuff() >= buff_size_; }
  /* soundings changed since the last save; true also if saving would replace unused old edits */
  long changes() const;
  bool unsaved() const;
  static std::string esfName(const std::string &path);
  static bool hasInterruptedSession(const std::string &path);
  /* "program, user, cpu, date" if the file is locked, and whether by this user on this cpu */
  static bool lockInfo(const std::string &path, std::string &description, bool &mine);
  static bool removeLock(const std::string &path);

  /* plot */
  PlotSettings settings;
  void setCanvas(int width, int height);  // the layout of the Motif program
  void setPlotRect(int width, int height, int xmin, int xmax, int ymin, int ymax);
  void setPlotMode(int show_time);
  void setViewMode(int view_mode) { view_mode_ = view_mode; }
  int viewMode() const { return view_mode_; }
  int plot(bool autoscale = false);
  int replot();  // after a change of the settings
  int step(int nsteps);
  const PlotBox &box() const { return box_; }
  double medianDepth() const { return bathmedian_; }

  /* mouse */
  int mouseToggle(int x, int y);
  int mousePick(int x, int y);
  int mouseErase(int x, int y);
  int mouseRestore(int x, int y);
  int mouseGrab(int mode, int x, int y);
  int mouseInfo(int x, int y);
  int zapOutbounds(int iping);
  int mouseLasso(const std::vector<std::pair<int, int>> &polygon);
  bool grabRect(int &xmin, int &xmax, int &ymin, int &ymax) const;
  /* the sounding a click would take in toggle (or info), or in pick mode */
  bool nearestBeam(int x, int y, bool pick_mode, int *iping, int *jbeam) const;
  std::vector<std::string> infoText() const;

  /* undo: a stroke makes one step of everything done between its begin and end */
  void beginStroke();
  void endStroke(const char *label);
  bool canUndo() const { return !undo_.empty(); }
  bool canRedo() const { return !redo_.empty(); }
  const std::string &undoLabel() const;
  const std::string &redoLabel() const;
  int undo();
  int redo();

  /* zoom and pan: the view follows the mouse, and never changes a sounding */
  void zoomWidth(int x, double factor);
  void zoomExaggeration(double factor);
  void panBy(int dx, int dy);
  void resetView();
  double panAcross() const { return pan_across_; }
  /* zoomed or moved: what is outside the plot is hidden and cannot be edited */
  bool zoomed() const { return zoomed_; }
  bool hasAmplitude() const;

  /* keys and buttons */
  int badPing();
  int goodPing();
  int leftPing();
  int rightPing();
  int zeroPing();
  int flagView();
  int unflagView();
  int unflagForward();

  /* filters */
  Filters filters;
  int filterForward();
  void filterLimits(int *beams_max, double *distance_max) const;

  /* A preview works on a copy of the flags, so it may run in another thread while the
     flags are edited. Only reading new pings or closing the file must wait for it. */
  using FlagCopy = std::vector<std::vector<char>>;
  FlagCopy copyFlags() const;
  FlagPreview previewFilters(const Filters &f, FlagCopy flags, int first, const std::atomic<bool> *cancel = nullptr) const;
  /* the edits in an edit save file, from the flags of the data file, against the flags now */
  bool previewEditFile(const std::string &path, FlagPreview &preview, std::string &message) const;
  int applyPreview(const FlagPreview &preview, const char *label);
  bool writeCurrentEdits(const std::string &path) const;

  /* flags by reason */
  int shown_reasons = REASON_ALL;
  static int flagReason(char flag);
  long countReason(int reason, int scope) const;
  int unflagReason(int reason, int scope);

  /* the review queue: suspect pings, worst first */
  std::vector<Suspect> findSuspects(const FlagCopy &flags, const std::atomic<bool> *cancel = nullptr) const;

  /* A reference surface: a grid, geographic or projected, depth or topography, and the
     difference of the soundings from it (positive: the sounding is deeper). */
  bool loadReference(const std::string &path, std::string &message);
  void clearReference();
  bool hasReference() const { return reference_ != nullptr; }
  const std::string &referenceName() const { return reference_name_; }
  bool referenceIsTopography() const { return ref_sign_ < 0.0; }
  /* decide depth or topography from the pings in memory; returns the soundings compared */
  long updateReferenceSign();
  /* for one ping; not a number where the grid has no value */
  void residuals(int iping, const std::vector<char> *flags, std::vector<double> &values) const;

  /* the versions of the edit save file kept in mbhistory, oldest first */
  struct HistoryVersion {
    int version = 0;
    std::string utc, program, user, note, path;
    long nedits = 0;
  };
  static std::vector<HistoryVersion> historyVersions(const std::string &swath_path);

  /* time series */
  static const char *tsLabel(int data_id);
  double tsValue(int iping, int data_id) const;
  void tsMinMax(int iping, int nping, int data_id, double *tsmin, double *tsmax) const;
  double xtrackSlope(int iping) const;

  /* state */
  int nbuff() const { return (int)ping_.size(); }
  const std::vector<Ping> &pings() const { return ping_; }
  int current() const { return current_id_; }
  int nplot() const { return nplot_; }
  int beamsBath() const { return beams_bath_; }
  int format() const { return format_; }
  const char *file() const { return ifile_; }
  int outputMode() const { return output_mode_; }
  int nloadTotal() const { return nload_total_; }
  int ndumpTotal() const { return ndump_total_; }
  bool beamSaved() const { return beam_save_; }
  int savedPing() const { return iping_save_; }
  int savedBeam() const { return jbeam_save_; }
  const BeamInfo &info() const { return info_; }
  const std::string &lastPing() const { return last_ping_; }

 private:
  int openFile(const std::string &path, int format, bool use_saved_edits, bool recover);
  int openEsf(bool use_saved_edits, bool recover);
  int closeFile(bool commit);
  bool commitEsf(const std::string &source);
  int dumpData(int hold_size, int *ndumped);
  int loadData(int buffer_size, int *nloaded);
  int filterPing(int iping);
  void layoutPingLabel(int iping, bool save);
  void error3(const std::string &a, const std::string &b, const std::string &c);
  void progress(const std::string &text);
  void log(const char *format, ...) const;
  void streamFlush();
  void resetInfo() { info_.set = false; }

  struct BeamChange {
    int ping;
    int beam;
    char before;
    char after;
  };
  struct UndoStep {
    std::string label;
    std::vector<BeamChange> changes;
  };
  void finishAction(const char *label);
  void clearUndo();
  int applyStep(const UndoStep &step, bool forward);
  void setFlag(int i, int j, int action, char flag);
  void flagBeam(int i, int j);
  void unflagBeam(int i, int j);
  void filterBeam(int i, int j);
  void saveBeam(int i, int j);
  bool zapHit(int x, int y, int *zap_ping) const;
  bool outsideBox(int i, int j) const;
  double centerDepth(const Ping &p) const;
  bool hidden(int i, int j) const { return zoomed_ && outsideBox(i, j); }
  bool reasonHidden(char flag) const { return !mb_beam_ok(flag) && (flagReason(flag) & shown_reasons) == 0; }
  bool writeEdits(FILE *fp) const;

  /* MBIO */
  int error_ = MB_ERROR_NO_ERROR;
  int format_ = 0;
  mb_path ifile_ = "";
  void *imbio_ptr_ = nullptr;
  void *store_ptr_ = nullptr;
  int output_mode_ = OUTPUT_EDIT;
  bool uselockfiles_ = true;
  int beams_bath_ = 0, beams_amp_ = 0, pixels_ss_ = 0;
  double btime_d_ = 0.0, etime_d_ = 0.0;
  char *beamflag_ = nullptr;
  double *bath_ = nullptr, *amp_ = nullptr, *bathacrosstrack_ = nullptr, *bathalongtrack_ = nullptr;
  double *ss_ = nullptr, *ssacrosstrack_ = nullptr, *ssalongtrack_ = nullptr;
  int *detect_ = nullptr, *priority_ = nullptr, *pulses_ = nullptr;

  /* buffer */
  bool file_open_ = false;
  int buff_size_ = DEFAULT_BUFFER_SIZE;
  int hold_size_ = DEFAULT_BUFFER_SIZE / 1000;
  int current_id_ = 0;
  int nload_total_ = 0;
  int ndump_total_ = 0;
  std::vector<Ping> ping_;
  std::string last_ping_;
  std::vector<double> bathlist_;

  /* edit save files */
  struct mb_esf_struct esf_;
  mb_path esffile_ = "";
  std::string esf_new_;
  bool esffile_open_ = false;
  bool write_failed_ = false;
  bool replacing_ = false;
  bool saved_in_session_ = false;
  long dumped_changes_ = 0;

  long pings_before_ = 0;

  /* reference surface */
  float *reference_ = nullptr;
  std::string reference_name_;
  int ref_nx_ = 0, ref_ny_ = 0;
  double ref_x0_ = 0.0, ref_y0_ = 0.0, ref_dx_ = 1.0, ref_dy_ = 1.0;
  float ref_nodata_ = 0.0f;
  double ref_sign_ = 1.0;
  void *ref_projection_ = nullptr;
  mutable std::mutex ref_mutex_;
  double referenceAt(double lon, double lat) const;

  /* undo */
  std::vector<UndoStep> undo_, redo_;
  std::vector<BeamChange> pending_;
  bool stroke_open_ = false;
  long undo_total_ = 0;

  /* plot */
  double pan_across_ = 0.0;  // m at the center of the plot
  double pan_depth_ = 0.0;   // m
  bool zoomed_ = false;
  int view_mode_ = VIEW_WATERFALL;
  int nplot_ = 0;
  PlotBox box_;
  bool classic_layout_ = true;
  double bathmedian_ = 0.0;
  int laid_first_ = 0, laid_last_ = 0;
  bool beam_save_ = false;
  int iping_save_ = 0;
  int jbeam_save_ = 0;
  BeamInfo info_;
  bool grab_set_ = false;
  int grab_start_x_ = 0, grab_start_y_ = 0, grab_end_x_ = 0, grab_end_y_ = 0;
};

}  // namespace mbe

#endif
