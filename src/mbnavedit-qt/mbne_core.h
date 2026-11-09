/*--------------------------------------------------------------------
 *    The MB-system:  mbne_core.h  9/11/2026
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
 * The navigation editing session of mbnavedit-qt, without Qt and without global
 * state: the records of a swath file, the edits, the navigation models, the layout
 * of the plots and the files. The arithmetic follows the Motif program mbnavedit,
 * so that the same actions write the same .nve file.
 */

#ifndef MBNAVEDIT_QT_MBNE_CORE_H
#define MBNAVEDIT_QT_MBNE_CORE_H

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"

namespace mbne {

constexpr char PROGRAM_NAME[] = "mbnavedit-qt";
constexpr int DEFAULT_BUFFER_SIZE = 1000000;
constexpr int DEFAULT_HOLD_SIZE = 100;
constexpr int NUMBER_PLOTS_MAX = 9;
constexpr int DEFAULT_PLOT_WIDTH = 767;
constexpr int DEFAULT_PLOT_HEIGHT = 300;
constexpr int PICK_DISTANCE = 50;
constexpr int SELECT_DISTANCE = 10;
constexpr long UNDO_LIMIT = 300000000;  // bytes kept for undo; the oldest steps go first

enum OutputMode { OUTPUT_EDIT = 0, OUTPUT_BROWSE = 1 };
enum PlotType {
  PLOT_TINT = 0,
  PLOT_LONGITUDE = 1,
  PLOT_LATITUDE = 2,
  PLOT_SPEED = 3,
  PLOT_HEADING = 4,
  PLOT_DRAFT = 5,
  PLOT_ROLL = 6,
  PLOT_PITCH = 7,
  PLOT_HEAVE = 8
};
constexpr int EDITABLE_PLOTS = 8;  // the types up to pitch can be selected and changed; heave only shown
constexpr int MOTIF_EDITABLE_PLOTS = 6;  // the Motif program stops at the sonar depth
enum ModelMode { MODEL_OFF = 0, MODEL_MEAN = 1, MODEL_DR = 2, MODEL_INVERT = 3 };

struct Record {
  int id = 0;
  int record = 0;
  int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
  double time_d = 0.0;
  double file_time_d = 0.0;
  double tint = 0.0;
  double lon = 0.0;
  double lat = 0.0;
  double speed = 0.0;
  double heading = 0.0;
  double draft = 0.0;
  double roll = 0.0;
  double pitch = 0.0;
  double heave = 0.0;
  double time_d_org = 0.0;
  double tint_org = 0.0;
  double lon_org = 0.0;
  double lat_org = 0.0;
  double speed_org = 0.0;
  double heading_org = 0.0;
  double draft_org = 0.0;
  double roll_org = 0.0;
  double pitch_org = 0.0;
  bool mean_ok = false;
  double lon_dr = 0.0;
  double lat_dr = 0.0;
  double speed_made_good = 0.0;
  double course_made_good = 0.0;
  bool lonlat_flag = false;
  unsigned char selected = 0;  // one bit per editable plot
  int pixel[2 * EDITABLE_PLOTS] = {};  // x, y per editable plot

  bool isSelected(int type) const { return (selected & (1 << type)) != 0; }
  void setSelected(int type, bool on) {
    if (on)
      selected |= (unsigned char)(1 << type);
    else
      selected &= (unsigned char)~(1 << type);
  }
};

struct Plot {
  int type = PLOT_TINT;
  int ixmin = 0;
  int ixmax = 0;
  int iymin = 0;
  int iymax = 0;
  double xmin = 0.0;
  double xmax = 0.0;
  double ymin = 0.0;
  double ymax = 0.0;
  double xscale = 1.0;
  double yscale = 1.0;
  std::string xlabel;
  std::string ylabel1;
  std::string ylabel2;
};

/* what is shown */
struct Show {
  bool tint = true;
  bool tint_org = true;
  bool lon = true;
  bool lon_org = true;
  bool lon_dr = false;
  bool lat = true;
  bool lat_org = true;
  bool lat_dr = false;
  bool speed = true;
  bool speed_org = true;
  bool smg = true;
  bool heading = true;
  bool heading_org = true;
  bool cmg = true;
  bool draft = true;
  bool draft_org = true;
  bool roll = false;
  bool pitch = false;
  bool heave = false;
};

struct InputFile {
  std::string path;
  int format = 0;
};

/* a file, or the files of a datalist when the format is -1, as the -I option takes them */
bool parse_input(const std::string &path, int format, std::vector<InputFile> &files, std::string &message);

class Session {
 public:
  Session();
  ~Session();
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  /* settings */
  int verbose = 0;
  Show show;
  int data_show_max = 2000;
  int data_show_size = 1000;  // seconds, 0 for all of the buffer
  int data_step_max = 2000;
  int data_step_size = 750;
  int mean_time_window = 100;  // tenths of a second
  int drift_lon = 0;           // 1e-5 degrees per hour
  int drift_lat = 0;
  ModelMode model_mode = MODEL_OFF;
  double weight_speed = 100.0;
  double weight_acceleration = 100.0;
  bool use_ping_data = false;
  bool gui_mode = false;
  int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
  int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
  int buffer_size = DEFAULT_BUFFER_SIZE;
  int hold_size = DEFAULT_HOLD_SIZE;
  /* do what the Motif program does, to compare with it: linear heading interpolation, no
     recalculation after the time tools, a session without edits still writes its .nve */
  bool compatibility = false;

  /* a message for the user, in three lines */
  std::function<void(const std::string &, const std::string &, const std::string &)> on_error;
  std::function<void(const std::string &)> on_progress;
  std::function<void(const std::string &)> on_saved;  // the path of the swath file

  /* files */
  static bool lockInfo(const std::string &path, std::string &description, bool &mine);
  static bool removeLock(const std::string &path);
  static bool hasSavedEdits(const std::string &path);
  int open(const std::string &path, int format, bool use_previous, OutputMode output_mode);
  int save();  // only when the whole file has been read: see canSave()
  bool canSave() const;
  int closeSave();     // writes what is needed and closes
  int closeDiscard();  // closes and leaves the .nve and .par as they were
  int done(bool *quit);
  int nextBuffer(bool *quit);
  bool unsaved() const;
  long changes() const;  // the records with a value different from the original one
  bool isOpen() const { return file_open_; }
  const char *file() const { return ifile_; }
  int format() const { return format_; }
  OutputMode outputMode() const { return output_mode_; }
  bool timestampProblem() const { return timestamp_problem_; }
  void setTimestampProblem(bool problem) { timestamp_problem_ = problem; }
  int navSource() const { return nav_source_; }
  int sensordepthSource() const { return sensordepth_source_; }
  int headingSource() const { return heading_source_; }
  int attitudeSource() const { return attitude_source_; }
  int loadedTotal() const { return nload_total_; }
  int dumpedTotal() const { return ndump_total_; }

  /* the view */
  const std::vector<Record> &records() const { return rec_; }
  std::vector<Record> &recordsForTest() { return rec_; }
  int nbuff() const { return (int)rec_.size(); }
  int current() const { return current_id_; }
  int nplot() const { return nplot_; }
  int numberPlots() const { return number_plots_; }
  void recountPlots();
  const std::vector<Plot> &plots() const { return plots_; }
  double plotStartTime() const { return plot_start_time_; }
  double plotEndTime() const { return plot_end_time_; }
  double fileStartTime() const { return file_start_time_d_; }
  void setPlotSize(int width, int height);
  /* in pixels; without these the margins are those of the Motif program, a fixed share of the plot */
  void setPlotMargins(int left, int right, int top, int bottom);
  int plotWidth() const { return plot_width_; }
  int plotHeight() const { return plot_height_; }
  int plotAll();
  int step(int seconds);
  int start();
  int end();
  int showAll();
  int goToTime(double time_d);  // the record at or after this time starts the view

  /* selecting with the pointer, in pixels */
  int mousePick(int x, int y);
  int mouseSelect(int x, int y);
  int mouseDeselect(int x, int y);
  int mouseSelectAll(int x, int y);
  int mouseDeselectAll();
  int deselectAll(int type);
  int setInterval(int x, int y, int which);  // 0 and 1 set the edges, 2 applies them, 3 forgets them
  bool intervalSet(int which) const { return which == 0 ? interval_set1_ : interval_set2_; }
  int intervalBound(int which) const { return which == 0 ? interval_bound1_ : interval_bound2_; }

  /* Selects the sonar depths further than threshold meters from the median of the records
     within window seconds around them, among the records in view, and nothing else, so that
     Interpolate then replaces them. Not an edit. A spike must be shorter than half the window.
     Also gives the largest distance from the median found, to help choose the threshold. */
  int despikeDepth(double window, double threshold, int *count, double *largest = nullptr);

  /* A change of the selection between these two calls is a step of undo, joined with the
     changes of selection just before it, so that one undo clears the picks made since the
     last edit. It is not an edit: it makes nothing unsaved. */
  void beginSelection();
  void finishSelection();

  /* edits: each one is a step of undo */
  int useDr();
  int useSmg();
  int useCmg();
  int interpolate();
  int interpolateRepeats();
  int revert();
  int flag();
  int unflag();
  int fixTime();
  int deleteBadTime();
  int applyOffset(double lon, double lat);
  double offsetLon() const { return offset_lon_; }
  double offsetLat() const { return offset_lat_; }

  bool canUndo() const { return !undo_.empty(); }
  bool canRedo() const { return !redo_.empty(); }
  const std::string &undoLabel() const;
  const std::string &redoLabel() const;
  int undo();
  int redo();

  void getModel();

  int editablePlots() const { return compatibility ? MOTIF_EDITABLE_PLOTS : EDITABLE_PLOTS; }

 private:
  struct RecordState {
    int time_i[7];
    double time_d, file_time_d, tint, lon, lat, speed, heading, draft, roll, pitch, speed_made_good, course_made_good;
    bool lonlat_flag;
  };
  struct Change {
    int index;
    RecordState before, after;
  };
  struct Scalars {
    int current_id;
    bool timestamp_problem;
    double offset_lon, offset_lat, offset_lon_applied, offset_lat_applied;
  };
  struct SelectionChange {
    int index;
    unsigned char before, after;
  };
  struct UndoStep {
    std::string label;
    bool is_selection = false;
    std::vector<SelectionChange> selection;
    Scalars before, after;
    std::vector<Change> changes;
    std::vector<Record> whole_before, whole_after;  // when the number of records changed
    long bytes() const;
  };
  static RecordState stateOf(const Record &r);
  static void setState(Record &r, const RecordState &s);
  static bool sameState(const RecordState &a, const RecordState &b);
  Scalars scalars() const;
  void setScalars(const Scalars &s);
  void beginEdit(bool whole = false);
  void finishEdit(const char *label);
  void clearUndo();
  long editDepth() const;  // the steps of undo that are edits
  int applyStep(const UndoStep &step, bool forward);

  void error3(const std::string &a, const std::string &b, const std::string &c);
  void progress(const std::string &message);
  int openFile(const std::string &path, int format, bool use_previous);
  void closeReader();
  int closeFile(bool commit);
  int actionClose(bool commit);
  bool commitNve(const std::string &source);
  void updatePar();
  int loadData();
  void dumpData(int hold);
  void writeRecord(FILE *fp, const Record &r);
  void clearView();

  int activePlotAt(int x, int y) const;
  bool deselectOthers(int active);
  double headingBetween(double a, double b, double fraction) const;
  void getSmgCmg(int i);
  void getGaussianMean();
  void getDeadReckoning();
  void getInversion();

  void interpolateTrack(double Record::*field, int type, bool &changed);
  void interpolateHold(double Record::*field, int type, bool marks_position, bool &timelonlat_change, bool &speedheading_change);
  void repeatField(double Record::*field, int type, bool &changed);

  std::vector<Record> rec_;
  int current_id_ = 0;
  int nplot_ = 0;
  int number_plots_ = 6;
  std::vector<Plot> plots_;
  int plot_width_ = DEFAULT_PLOT_WIDTH;
  int plot_height_ = DEFAULT_PLOT_HEIGHT;
  int margin_left_ = -1, margin_right_ = -1, margin_top_ = -1, margin_bottom_ = -1;
  double plot_start_time_ = 0.0;
  double plot_end_time_ = 0.0;
  double file_start_time_d_ = 0.0;
  bool first_read_ = false;
  bool timestamp_problem_ = false;
  bool at_eof_ = false;

  int interval_bound1_ = 0;
  int interval_bound2_ = 0;
  double interval_time1_ = 0.0;
  double interval_time2_ = 0.0;
  bool interval_set1_ = false;
  bool interval_set2_ = false;

  double offset_lon_ = 0.0;
  double offset_lat_ = 0.0;
  double offset_lon_applied_ = 0.0;
  double offset_lat_applied_ = 0.0;

  /* MBIO */
  int error_ = MB_ERROR_NO_ERROR;
  bool uselockfiles_ = true;
  OutputMode output_mode_ = OUTPUT_EDIT;
  mb_path ifile_ = "";
  mb_pathplus nfile_ = "";
  int format_ = 0;
  bool file_open_ = false;
  void *imbio_ptr_ = nullptr;
  void *store_ptr_ = nullptr;
  int platform_source_ = 0;
  int nav_source_ = 0;
  int sensordepth_source_ = 0;
  int heading_source_ = 0;
  int attitude_source_ = 0;
  int svp_source_ = 0;
  char *beamflag_ = nullptr;
  double *bath_ = nullptr;
  double *amp_ = nullptr;
  double *bathacrosstrack_ = nullptr;
  double *bathalongtrack_ = nullptr;
  double *ss_ = nullptr;
  double *ssacrosstrack_ = nullptr;
  double *ssalongtrack_ = nullptr;
  int nload_ = 0;
  int ndump_ = 0;
  int nload_total_ = 0;
  int ndump_total_ = 0;

  /* the new .nve grows here, and replaces the old one only on saving */
  std::string new_file_;
  FILE *nfp_ = nullptr;
  bool write_failed_ = false;
  bool replacing_ = false;
  bool saved_in_session_ = false;

  /* undo */
  std::vector<UndoStep> undo_, redo_;
  std::vector<unsigned char> selection_snapshot_;
  long undo_bytes_ = 0;
  long saved_depth_ = 0;         // the number of undo steps at the last save; -1 when out of reach
  bool dumped_changes_ = false;  // edited records have left the buffer since the last save
  std::vector<RecordState> snapshot_;
  std::vector<Record> whole_snapshot_;
  Scalars snapshot_scalars_{};
  bool whole_edit_ = false;
};

}  // namespace mbne

#endif
