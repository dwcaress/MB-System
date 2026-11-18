/*--------------------------------------------------------------------
 *    The MB-system:  mbe_editor.h  2/11/2026
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

#ifndef MBEDIT_QT_MBE_EDITOR_H
#define MBEDIT_QT_MBE_EDITOR_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <QDialog>
#include <QList>
#include <QWidget>

#include "mbe_canvas.h"
#include "mbe_core.h"
#include "mbe_panels.h"

class QAction;
class QActionGroup;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMenu;
class QProcess;
class QTemporaryDir;
class QPushButton;
class QRadioButton;
class QSplitter;
class QSpinBox;
class QDoubleSpinBox;
class QHideEvent;
class QShowEvent;
class QTableWidget;
class QThread;
class QTimer;
class QToolBar;

namespace mbe {

/* whether to apply the edits saved for a file when it is opened */
enum class SavedEdits { Ask, Apply, Ignore };
/* what to do with a file locked by the same user on the same computer */
enum class LockChoice { Refuse, OpenAnyway };
/* Window: the rows of controls of the program mbedit-qt. Panel: a compact form for
   part of another program's window, with one toolbar and one row under the plot. */
enum class Presentation { Window, Panel };

/* The whole editor as a widget, for a window of its own or a panel of another program.
   Before deleting it with a file open, call prepareToClose: deleting it discards edits
   that have not been saved. */
class EditorWidget : public QWidget {
  Q_OBJECT

 public:
  explicit EditorWidget(Presentation presentation = Presentation::Window, QWidget *parent = nullptr);
  ~EditorWidget() override;

  QSize sizeHint() const override { return presentation_ == Presentation::Panel ? QSize(900, 620) : QSize(1400, 850); }

  mbe::Session &session() { return session_; }
  Canvas *canvas() { return canvas_; }
  bool busy() const { return worker_ != nullptr; }

  /* the command line */
  void addInput(const std::string &path, int format);
  void setBrowse(bool browse);
  void setRunMbprocess(bool run) { run_mbprocess_ = run; }
  void setGuiMode(bool gui_mode) { session_.gui_mode = gui_mode; }
  void loadFirstFile(bool use_saved_edits);
  void loadReference(const QString &path);
  void setStartReference(const QString &path) { start_reference_ = path; }

  /* set before opening a file */
  void setProgramName(const std::string &name) { session_.program_name = name; }
  void setVerbose(int verbose) { session_.verbose = verbose; }
  void setTimeWindow(const int begin[7], const int end[7]);
  void setProgramDirectory(const QString &directory) { program_directory_ = directory; }  // where to look for other programs first
  /* decides about a lock instead of the question to the user */
  void setLockChoice(std::function<LockChoice(const QString &path, const QString &holder)> choose) { lock_choice_ = std::move(choose); }
  /* Opens mbeditviz or the water column for this file instead of starting the program.
     Returns false if that failed. The format is 0 if not known, the ping -1 if none. */
  void setToolOpener(std::function<bool(const QString &tool, const QString &path, int format, long ping)> open);

  /* From outside. These return false if the editor cannot do it now; the work of
     openFile runs in the background, so watch the signals. */
  bool openFile(const QString &path, int format, SavedEdits edits = SavedEdits::Ask);
  bool save();
  bool goToTime(double time_d);
  bool showRecord(long record);

  /* for the surrounding window */
  QList<QMenu *> menus() const { return menus_; }
  QString title() const { return title_; }
  void setDisplayName(const QString &name);  // in titles and message boxes
  void setShortcutContext(Qt::ShortcutContext context);
  /* True if the editor may close now. Otherwise it stays, because it is busy, the user
     cancelled, or the edits are being saved; closed_later is then called once they are. */
  bool prepareToClose(std::function<void()> closed_later);

 signals:
  void titleChanged(const QString &title);
  void unsavedChanged(bool unsaved);
  void statusMessage(const QString &text, int timeout_ms);
  void busyChanged(bool busy);
  void closeRequested();  // the Quit button, or the last file is done in GUI mode
  void fileOpened(const QString &path, int format);
  void fileClosed(const QString &path);
  void viewChanged(const QString &path, long first_record, long last_record, double first_time_d, double last_time_d);
  void editsChanged(const QString &path, long changes);
  void editsSaved(const QString &path);
  void soundingInfo(const QStringList &lines);

 protected:
  void hideEvent(QHideEvent *event) override;
  void showEvent(QShowEvent *event) override;

 private:
  /* set up */
  void buildPanel();
  QMenu *newMenu(const QString &title);
  void setWorking(bool working);
  void buildMenus();
  void buildControls();
  void buildGotoDialog();
  void buildBufferDialog();
  void buildAnnotationDialog();
  void buildFiltersDialog();
  void buildFileListDialog();
  void buildReasonsDialog();
  void buildMbcleanDialog();
  void buildHistoryDialog();

  /* files */
  void runJob(const QString &message, std::function<void()> work, std::function<void()> done);
  void openDialog();
  void loadFile(int index, bool ask_saved_edits, bool use_saved_edits);
  void afterCurrentFileClosed(std::function<void()> next);
  void doneOrNextFile();
  void nextBuffer();
  void startMbprocess();
  void showError(const QString &a, const QString &b, const QString &c);

  /* controls */
  void syncControls();
  void reportState();
  void syncFileList();
  void applySettings();
  void afterEdit();
  void afterMove();
  void setModeFromUi(int mode);
  void setViewMode(int view);
  void setPlotMode(int mode);
  void setShowMode(int mode);
  void pingsChanged(int value);
  void stepChanged(int value);
  void sliderMoved(int value);
  void flagAction(std::function<int()> action);
  void gotoApply();
  void filtersToDialog();
  void filtersApply();
  void editSelectedFile();
  void removeSelectedFile();
  void about();

  /* previews and the review queue run beside the editing */
  void stopBackground();
  void startFilterPreview();
  void showPreview(std::shared_ptr<mbe::FlagPreview> preview);
  void clearPreview();
  mbe::Filters filtersFromDialog() const;
  void startSuspects();
  void nextSuspect(int direction);
  void showPing(int ping);
  void updateReasonCount();
  void runMbclean();
  void mbcleanFinished();

  /* other programs */
  QString findProgram(const QString &name) const;
  QString waterColumnFile() const;
  void openWaterColumn(int ping);
  void openMbeditviz();
  void clearReference();
  void reportReference();
  void showHistory();
  void historyPreview();
  void historyRestore();
  void pingContextMenu(QPoint global, int ping, int beam);
  void flagWholePing(int ping, bool flag);

  mbe::Session session_;
  QThread *worker_ = nullptr;
  std::vector<mbe::InputFile> files_;
  std::vector<int> file_saved_;
  int current_file_ = -1;
  bool browse_ = false;
  bool run_mbprocess_ = false;
  std::string saved_path_;
  const Presentation presentation_;
  QWidget *central_;
  QToolBar *panel_bar_ = nullptr;
  QWidget *scales_panel_ = nullptr;
  QLabel *info_line_ = nullptr;
  QAction *reset_zoom_ = nullptr;
  bool job_had_focus_ = false;
  QList<QDialog *> hidden_dialogs_;
  QList<QMenu *> menus_;
  QString display_name_ = PROGRAM_NAME;
  QString title_;
  bool unsaved_ = false;
  QString program_directory_;
  std::function<LockChoice(const QString &, const QString &)> lock_choice_;
  std::function<bool(const QString &, const QString &, int, long)> tool_opener_;
  QString open_path_;
  long view_first_ = -1, view_last_ = -1, reported_changes_ = -1;
  bool closing_ = false;

  Canvas *canvas_;
  TimeSeriesPanel *panel_;
  TrackMap *map_;
  QSplitter *splitter_;
  PingSlider *slider_;
  QLabel *position_;
  ScaleControl *width_, *exagger_, *pings_, *step_;
  QButtonGroup *mode_group_;
  QPushButton *start_, *reverse_, *forward_, *end_, *next_, *done_, *flag_view_, *unflag_view_, *unflag_forward_, *quit_;
  QAction *undo_action_, *redo_action_, *show_amplitude_, *show_residual_, *show_map_;
  QAction *clear_reference_, *water_column_, *mbeditviz_, *history_action_;
  QAction *open_action_, *filelist_action_, *save_action_, *browse_action_;
  QAction *flagged_soundings_, *flagged_profile_, *reverse_keys_, *reverse_mouse_;
  QActionGroup *view_group_, *show_group_, *plot_group_;

  QDialog *goto_dialog_ = nullptr;
  QSpinBox *goto_fields_[6];
  QDialog *buffer_dialog_ = nullptr;
  ScaleControl *buffer_size_, *buffer_hold_;
  QDialog *annotation_dialog_ = nullptr;
  ScaleControl *x_interval_, *y_interval_;

  QDialog *filters_dialog_ = nullptr;
  QCheckBox *f_median_, *f_wrongside_, *f_cutbeam_, *f_cutdistance_, *f_cutangle_;
  QSpinBox *f_median_threshold_, *f_median_ltrack_, *f_median_xtrack_, *f_wrongside_threshold_, *f_cutbeam_begin_, *f_cutbeam_end_;
  QDoubleSpinBox *f_cutdistance_begin_, *f_cutdistance_end_, *f_cutangle_begin_, *f_cutangle_end_;

  QDialog *filelist_dialog_ = nullptr;
  QTableWidget *filelist_;
  QRadioButton *filelist_edit_, *filelist_browse_;
  QTimer *filelist_timer_;

  QCheckBox *f_preview_;
  QLabel *f_preview_label_;
  QTimer *preview_timer_;
  QThread *preview_thread_ = nullptr;
  QThread *suspects_thread_ = nullptr;
  std::atomic<bool> background_cancel_{false};
  int preview_generation_ = 0;
  mbe::FlagPreview preview_;
  SuspectStrip *strip_;
  std::vector<mbe::Suspect> suspects_;
  int suspect_index_ = -1;

  QDialog *reasons_dialog_ = nullptr;
  QCheckBox *show_manual_, *show_filter_, *show_sonar_;
  QComboBox *reason_, *scope_;
  QLabel *reason_count_;

  QDialog *mbclean_dialog_ = nullptr;
  QLineEdit *mbclean_args_;
  QLabel *mbclean_status_;
  QPushButton *mbclean_run_, *mbclean_accept_, *mbclean_reject_;
  QProcess *mbclean_process_ = nullptr;
  std::unique_ptr<QTemporaryDir> mbclean_dir_;
  QString mbclean_output_;

  QString start_reference_;

  QDialog *history_dialog_ = nullptr;
  QTableWidget *history_table_;
  QLabel *history_status_;
  std::vector<mbe::Session::HistoryVersion> history_;
};

}  // namespace mbe

#endif
