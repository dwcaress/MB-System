/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_core.h  9/11/2026
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
 * mbnaq::Session is the Qt-free core of mbnavadjust-qt. It runs the unchanged
 * mbnavadjust_prog.c and holds what the Motif callbacks in
 * mbnavadjust_callbacks.c computed for display: the list rows, the status
 * lines and which controls are enabled. The program logic keeps its state in
 * global variables, so there can be one Session, and one open project, in a
 * process.
 */

#ifndef MBNAQ_CORE_H_
#define MBNAQ_CORE_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace mbnaq {

/* Where the program logic draws. The plots receive one surface each. */
struct UndoStep;

class DrawSurface {
 public:
  virtual ~DrawSurface();
  virtual void line(int x1, int y1, int x2, int y2, unsigned int pixel, int style) = 0;
  virtual void rectangle(int x, int y, int width, int height, unsigned int pixel, int style) = 0;
  virtual void fillRectangle(int x, int y, int width, int height, unsigned int pixel, int style) = 0;
  virtual void text(int x, int y, const char *string, unsigned int pixel, int style) = 0;
  virtual void measure(const char *string, int *width, int *ascent, int *descent) = 0;
  virtual void clip(int x, int y, int width, int height) = 0;

  /* what has been drawn on this surface */
  long lines = 0, rectangles = 0, fills = 0, texts = 0;
};

/* The values match MBNA_VIEW_LIST_* in mbnavadjust_io.h. */
enum ListKind {
  ListReferenceGrids,
  ListSurveys,
  ListBlocks,
  ListFiles,
  ListSections,
  ListCrossings,
  ListMediocreCrossings,
  ListGoodCrossings,
  ListBetterCrossings,
  ListTrueCrossings,
  ListTies,
  ListTiesSortedAll,
  ListTiesSortedWorst,
  ListTiesSortedBad,
  ListGlobalTies,
  ListGlobalTiesSorted,
  ListKindCount
};

/* The values match MBNA_VIEW_MODE_* in mbnavadjust_io.h. */
enum ViewMode {
  ViewAll,
  ViewSurvey,
  ViewWithSurvey,
  ViewBlock,
  ViewFile,
  ViewWithFile,
  ViewWithSection,
  ViewModeCount
};

enum class FileNav { Poor, Good, Fixed, FixedXY, FixedZ };
enum class TieSetting { XY, Z, XYZ, Unfixed, Fixed };
enum class CrossingWindowMode { Crossing, Section };
enum class ModelPlotStyle { TimeSeries, Perturbation, TieOffsets };

/* The list shown in the main window. */
struct ListView {
  std::string title;
  std::vector<std::string> rows;
  int selected = -1; /* row, or -1 */
};

/* The current selections and the state of the crossing window. */
struct Selection {
  int list, mode, survey, survey1, survey2, file, section, crossing, tie;
  int naverrMode, status, currentCrossing, currentTie, currentFile, currentSection;
  double offsetX, offsetY, offsetZ, sortedThreshold;
  int saveCount;
  bool projectIsOpen;
  int numTies;
  int inversionStatus;
};

/* The values of the Controls form. */
struct ControlValues {
  int decimation;
  double sectionLength;
  int sectionSoundings;
  double contourInterval, colorInterval, tickInterval, smoothing, zOffsetWidth;
  int useMode; /* 0 primary, 1 secondary, 2 tertiary */
};

/* The heading and roll biases of the two loaded sections. */
struct BiasValues {
  bool available;
  int file1, section1, file2, section2;
  double heading1, roll1, heading2, roll2;
};

/* What the owner of a Session is told. Each may be left empty. They are
   called on the thread that called into the Session. */
struct Callbacks {
  std::function<void(const std::string &)> messageOn;
  std::function<void(const std::string &)> messageUpdate;
  std::function<void()> messageOff;
  std::function<void(const std::string &)> info;
  std::function<void(const std::string &, const std::string &, const std::string &)> error;
  std::function<void()> statusChanged;
  std::function<void()> crossingChanged;
};

class Session {
 public:
  /* The one Session of this process, or nullptr if there is one already. */
  static Session *create();
  static Session *current();
  ~Session();
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  void setCallbacks(const Callbacks &callbacks);

  /* startup choices, made before a project is opened */
  void setVerbose(int verbose);
  void setDarkBackground(bool dark);
  void setResetCrossings(bool reset);
  /* keep the faults of the Motif program, for comparing with it */
  void setMotifCompatibility(bool compatible) { motifCompatibility_ = compatible; }
  bool motifCompatibility() const { return motifCompatibility_; }

  /* project */
  bool newProject(const std::string &path);
  bool openProject(const std::string &path);
  bool importData(const std::string &path, int format, bool singleSurvey);
  bool importReference(const std::string &path);
  /* Motif's File>Close dropped up to nine edits; pass true to write them. */
  void closeProject(bool writeOutstanding);
  bool writeProject();
  std::string projectName() const;
  std::string versionText() const;

  /* main window list */
  void showList(ListKind kind);
  bool setViewMode(ViewMode mode);
  const ListView &list() const { return list_; }
  void clickListRow(int row);
  void clickSelectedRow();

  /* what is displayed */
  const std::vector<std::string> &statusLines() const { return statusLines_; }
  const std::map<std::string, bool> &enabledStates() const { return enabled_; }
  bool isEnabled(const std::string &widget) const;
  const std::vector<std::string> &crossingStatusLines() const { return crossingLines_; }
  const std::vector<std::string> &crossingOffsetLines() const { return offsetLines_; }
  Selection selection() const;

  /* crossing window */
  void openCrossingWindow(CrossingWindowMode mode);
  /* load one crossing or section, as a click in the list does; false if there is no such one */
  bool showCrossing(int crossing, int tie);
  bool showSection(int file, int section);
  void nextCrossing();
  void previousCrossing();
  void nextUnsetCrossing();
  void addTie();
  void deleteTie();
  void selectTie();
  void setTie();
  void resetTie();
  void unsetCrossing();
  void skipCrossing();
  void dismissCrossingWindow();
  void fullSize();
  void zeroOffset();
  void zeroZOffset();
  void applyZOffset();
  void moveToMinimumMisfit();
  void moveToMinimumXYMisfit();
  void setMisfitCenter(bool zeroCenter);
  bool misfitCenterIsZero() const;
  void reloadCrossingWindow();

  /* Controls and Biases forms */
  ControlValues controlValues() const;
  void applyControls(const ControlValues &values);
  BiasValues biasValues() const;
  void applyBiases(double heading1, double roll1, double heading2, double roll2);
  void applyBiasesToAllFiles(double heading, double roll);

  /* undo of interactive tie and status edits */
  bool canUndo() const { return !undoSteps_.empty(); }
  bool canRedo() const { return !redoSteps_.empty(); }
  void undo();
  void redo();

  /* plots: without surfaces the plot code still runs and draws nothing */
  void setCrossingSurfaces(DrawSurface *contour, DrawSurface *misfit, DrawSurface *zOffset);
  /* plot sizes as the shared plot code uses them: the last pixel position in each direction */
  void setCrossingPlotSizes(int contourWidth, int contourHeight, int misfitWidth, int misfitHeight, int zOffsetWidth,
                            int zOffsetHeight);
  bool crossingLoaded() const;
  bool zooming() const { return button2Down_; }
  void replotCrossing();
  void setPlotsChanged(const std::function<void()> &plotsChanged) { plotsChanged_ = plotsChanged; }

  /* mouse on the plots; button is 1 left, 2 middle, 3 right */
  void contourPress(int button, int x, int y);
  void contourDrag(int x, int y);
  void contourRelease(int button, int x, int y);
  void misfitPress(int x, int y);
  void misfitDrag(int x, int y);
  void misfitRelease();
  void zOffsetPress(int x, int y);
  void zOffsetDrag(int x, int y);
  void zOffsetRelease();

  /* model plot */
  bool modelPlotOpen() const;
  void openModelPlot(DrawSurface *surface, int width, int height);
  void closeModelPlot();
  void resizeModelPlot(int width, int height);
  void setModelPlotStyle(ModelPlotStyle style);
  void replotModelPlot();
  void modelPlotFullSize();
  void modelPlotClearBlock();
  void modelPlotPress(int button, int x, int y);
  void modelPlotDrag(int x, int y);
  void modelPlotRelease(int button, int x, int y);
  const std::vector<std::string> &modelPlotLines() const { return modelLines_; }

  /* one line with the counts of what each plot has drawn */
  std::string drawCounts() const;

  /* main window actions */
  void setFileNav(FileNav value);
  void setTieSetting(TieSetting value);
  void autoPick(bool withVertical);
  void autoSetVertical();
  void findCrossings();
  void zeroZOffsets();
  void unsetSkipped();
  void invert();
  void updateGrids();
  void applyNavigation();

  /* called from the program logic through the bridge */
  void updateStatus();
  void updateCrossing();
  void updateModelPlotStatus();
  void initCrossingWindow(int mode);
  void addInfo(const char *info, bool timeTag);
  void notifyMessageOn(const char *message);
  void notifyMessageUpdate(const char *message);
  void notifyMessageOff();
  void notifyError(const char *s1, const char *s2, const char *s3);

 private:
  Session();

  struct RowRef {
    int a, b, c;
  };

  void setEnabled(const char *widget, bool enabled);
  void setEnabled(const char *const *widgets, int count, bool enabled);
  void buildStatusLines();
  void buildList();
  std::string listTitle() const;
  void updateEnabled();
  void updateCrossingOffsets();
  void plotCrossing(int plotMode);
  void notifyPlots();
  void setModelPlotBorders(int width, int height);
  void loadCrossingRow(int crossing, int tie);
  void loadSectionRow(int file, int section);
  void refreshAfterEdit(bool modelOutOfDate);
  void pushUndo();
  void applyStep(struct UndoStep &step);
  void takeStep(struct UndoStep &step) const;
  void resetSelections();

  Callbacks callbacks_;
  ListView list_;
  std::vector<RowRef> rowRefs_;
  std::vector<std::string> statusLines_;
  std::vector<std::string> crossingLines_;
  std::vector<std::string> offsetLines_;
  std::vector<std::string> modelLines_;
  std::map<std::string, bool> enabled_;
  bool motifCompatibility_ = false;
  std::vector<struct UndoStep> undoSteps_;
  std::vector<struct UndoStep> redoSteps_;
  std::function<void()> plotsChanged_;

  DrawSurface *contourSurface_;
  DrawSurface *misfitSurface_;
  DrawSurface *zOffsetSurface_;
  DrawSurface *modelSurface_ = nullptr;
  int modelBorders_[4] = {0, 0, 0, 0};

  /* the mouse buttons, shared by the plots as in the Motif code */
  bool button1Down_ = false;
  bool button2Down_ = false;
  bool button3Down_ = false;
  int locX_ = 0;
  int locY_ = 0;

  /* the plot bounds that were asked for; the scaling widens them to fit each plot size */
  bool haveRequestedBounds_ = false;
  double requestedBounds_[4] = {0.0, 0.0, 0.0, 0.0};
  double scaledBounds_[4] = {0.0, 0.0, 0.0, 0.0};
};

}  // namespace mbnaq

#endif
