/*--------------------------------------------------------------------
 *    The MB-system:  mbne_widget.h  9/11/2026
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

#ifndef MBNAVEDIT_QT_MBNE_WIDGET_H
#define MBNAVEDIT_QT_MBNE_WIDGET_H

#include <functional>
#include <string>
#include <vector>

#include <QList>
#include <QWidget>

#include "mbne_canvas.h"
#include "mbne_core.h"
#include "mbne_panels.h"

class QAction;
class QButtonGroup;
class QComboBox;
class QDialog;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QRadioButton;
class QScrollArea;
class QSpinBox;
class QToolBar;
class QHideEvent;
class QShowEvent;
class QTableWidget;
class QThread;
class QTimer;

namespace mbne {

/* whether to start from the navigation edits saved for a file when it is opened */
enum class SavedEdits { Ask, Apply, Ignore };
/* what to do with a file locked by the same user on the same computer */
enum class LockChoice { Refuse, OpenAnyway };
/* Window: the rows of controls of the program mbnavedit-qt. Panel: a compact form for
   part of another program's window, with one toolbar. */
enum class Presentation { Window, Panel };

/* The whole navigation editor as a widget, for a window of its own or a panel of another program. Before deleting it with a file open, call
   prepareToClose: deleting it discards edits that have not been saved. */
class NavEditWidget : public QWidget {
  Q_OBJECT

 public:
  explicit NavEditWidget(Presentation presentation = Presentation::Window, QWidget *parent = nullptr);
  ~NavEditWidget() override;

  QSize sizeHint() const override { return presentation_ == Presentation::Panel ? QSize(900, 620) : QSize(1300, 950); }

  Session &session() { return session_; }
  Canvas *canvas() { return canvas_; }
  bool busy() const { return worker_ != nullptr; }

  /* the command line */
  void addInput(const std::string &path, int format);
  void setBrowse(bool browse);
  void setRunMbprocess(bool run) { run_mbprocess_ = run; }
  void setGuiMode(bool gui_mode) { session_.gui_mode = gui_mode; }
  void loadFirstFile();

  /* set before opening a file */
  void setTimeWindow(const int begin[7], const int end[7]);
  void setProgramDirectory(const QString &directory) { program_directory_ = directory; }  // where to look for mbprocess
  /* decides about a lock instead of the question to the user */
  void setLockChoice(std::function<LockChoice(const QString &path, const QString &holder)> choose) { lock_choice_ = std::move(choose); }

  /* These return false if the editor cannot do it now; the work of openFile runs in the background, so watch the signals. */
  bool openFile(const QString &path, int format, SavedEdits edits = SavedEdits::Ask);
  bool save();
  bool goToTime(double time_d);  // only within the records in memory

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

 protected:
  void hideEvent(QHideEvent *event) override;
  void showEvent(QShowEvent *event) override;

 private:
  struct ShowToggle {
    QAction *action;
    bool Show::*field;
  };

  void buildControls();
  void buildMenus();
  void buildPanel();
  void buildFileListDialog();
  void buildTimeSteppingDialog();
  void buildModelDialog();
  void buildTimeToolDialogs();
  void buildOffsetDialog();
  void buildDespikeDialog();
  QDialog *newDialog(const QString &title, const QString &name);
  QPushButton *newButton(const QString &text, const QString &name, std::function<void()> action);

  void setWorking(bool working);
  void runJob(const QString &message, std::function<void()> work, std::function<void()> done);
  void startMbprocess();
  void showError(const QString &a, const QString &b, const QString &c);
  void openDialog();
  void loadFile(int index, bool ask_saved_edits = true, bool use_saved_edits = false);
  void afterCurrentFileClosed(std::function<void()> next);
  void doneOrNextFile();
  void nextBuffer();
  void editSelectedFile();
  void removeSelectedFile();
  void about();

  void syncControls();
  void syncUndo();
  void syncFileList();
  void reportState();
  void edit(std::function<int()> action);
  void move(std::function<int()> action);
  void applyModel(bool mode_changed);
  void showChanged(bool Show::*field, bool on);
  void sliderChanged(int value);
  void runDespike();
  void interpolateSpikes();

  Session session_;
  QThread *worker_ = nullptr;
  std::vector<InputFile> files_;
  int current_file_ = -1;
  bool browse_ = false;
  bool run_mbprocess_ = false;
  std::string saved_path_;
  QList<QMenu *> menus_;
  QString title_;
  bool unsaved_ = false;
  bool quiet_ = false;
  const Presentation presentation_;
  QString display_name_ = PROGRAM_NAME;
  QString program_directory_;
  std::function<LockChoice(const QString &, const QString &)> lock_choice_;
  QString open_path_;
  long view_first_ = -1, view_last_ = -1, reported_changes_ = -1;
  bool save_reported_ = false;
  bool job_had_focus_ = false;
  QList<QDialog *> hidden_dialogs_;

  QWidget *central_;
  QWidget *controls_;
  QToolBar *panel_bar_ = nullptr;
  QWidget *scales_panel_ = nullptr;
  QMenu *edit_menu_ = nullptr;
  Canvas *canvas_;
  QScrollArea *scroll_;
  TimeSlider *slider_;
  QLabel *position_;
  QButtonGroup *mode_group_;
  QPushButton *pick_zoom_;
  QList<QWidget *> need_file_;
  QAction *open_action_, *filelist_action_, *save_action_, *browse_action_, *undo_action_, *redo_action_;
  QList<QAction *> need_file_actions_;
  std::vector<ShowToggle> show_toggles_;

  QDialog *filelist_dialog_ = nullptr;
  QTableWidget *filelist_;
  QRadioButton *filelist_edit_, *filelist_browse_;
  QTimer *filelist_timer_;

  QDialog *timestep_dialog_ = nullptr;
  ScaleControl *time_span_, *time_step_;

  QDialog *model_dialog_ = nullptr;
  QComboBox *model_mode_;
  ScaleControl *model_window_;
  QSpinBox *drift_lon_, *drift_lat_;
  QLineEdit *weight_speed_, *weight_acceleration_;

  QDialog *time_interpolation_dialog_ = nullptr;
  QDialog *delete_bad_dialog_ = nullptr;
  QPushButton *time_interpolation_apply_, *delete_bad_apply_;

  QDialog *offset_dialog_ = nullptr;
  QDoubleSpinBox *offset_lon_, *offset_lat_;

  QDialog *despike_dialog_ = nullptr;
  QDoubleSpinBox *despike_window_, *despike_threshold_;
  QLabel *despike_result_;
  int despike_found_ = 0;
};

}  // namespace mbne

#endif
