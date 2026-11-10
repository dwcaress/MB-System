/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_controller.h  11/10/2026
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
 * mbeditviz without a window of its own: the swath files, loading them,
 * gridding them, the map and the 3D soundings view that a pick opens, the
 * menus of the Motif main window (File, Options, Help) and the Open Swath
 * Data, Grid Parameters, About, error and "Please Wait..." dialogs. A
 * program shows the menus and the state (status, mode, file list) where it
 * likes; the standalone mbeditviz-qt shows them in its main window with a
 * FileListWidget (mbeditviz_qt_window.h). The map and the soundings view are
 * mbview-qt widgets, placed by mbvq::set_map_placement() and
 * mbvq::set_soundings_placement(). One controller per program, as the
 * mbeditviz core keeps global state.
 */

#ifndef MBEDITVIZ_QT_CONTROLLER_H
#define MBEDITVIZ_QT_CONTROLLER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

class QAction;
class QDialog;
class QLabel;
class QMenu;
class QWidget;

class OpenDialog;
class GridDialog;

namespace mbevq {

class SwathController final : public QObject {
  Q_OBJECT

 public:
  /* starts mbview and the 3D soundings view (mbvq::start) */
  explicit SwathController(QObject *parent = nullptr);
  ~SwathController() override;

  /* the controller of the program, nullptr if there is none */
  static SwathController *current();

  /* Sets the starting values of mbeditviz and reads the options of the
     Motif mbeditviz (mbeditviz_init of mbeditviz_prog.c); call it once,
     before files are opened. A program passes its command line; --help or a
     bad option end the program there, as in Motif. A host passes nothing. */
  int init(int argc = 0, char **argv = nullptr);

  /* File, Options and Help, as the Motif main window */
  QList<QMenu *> menus() const { return menus_; }
  /* the window the dialogs belong to */
  void setDialogParent(QWidget *parent) { dialog_parent_ = parent; }
  QWidget *dialogParent() const { return dialog_parent_; }

  /* what the menus and the buttons of the main window do */
  /* a swath file or a datalist, as Open Swath Data (format 0: from the file) */
  bool openFile(const QString &path, int format = 0);
  void showOpenDialog();
  /* load all files, then ask for the grid parameters */
  void viewAll();
  /* load only the selected files, then ask for the grid parameters */
  void viewSelected();
  void removeSelected();
  /* Options > Generate New Grid: close the map, ask for the grid parameters */
  void updateGrid();
  /* as the Motif Quit: closes the map and the soundings view and unloads the
     files (writing the edits and removing the locks); true, nothing has to
     wait */
  bool prepareToClose();
  void about();

  /* the state, for the program that shows it */
  QString status() const { return status_; }
  bool editMode() const;
  void setEditMode(bool edit);
  /* open: a file can be opened; files: there are files and no grid (the
     view and remove buttons, the mode) */
  bool openEnabled() const { return open_enabled_; }
  bool filesEnabled() const { return files_enabled_; }
  /* the rows of the file list, as the Motif list */
  QStringList files() const { return files_; }
  /* the selected rows, 0-based, ascending */
  QList<int> selectedRows() const { return selected_; }
  void setSelectedRows(const QList<int> &rows);

  /* for the library (mbeditviz_qt_ui.c through the bridge) */
  void setStatus(const QString &text);
  void setSensitivity(bool open, bool files);
  void listBegin();
  void listAdd(const QString &label);
  void listEnd();
  void showMessage(const QString &message, bool on);
  void showError(const QString &text);
  void showGridParameters();

 signals:
  void statusChanged(const QString &text);
  void sensitivityChanged(bool open, bool files);
  void modeChanged(bool edit);
  /* the rows of the file list were rebuilt; the selection is in selectedRows() */
  void filesChanged();
  /* File > Quit */
  void closeRequested();

 private:
  QList<QMenu *> menus_;
  QWidget *dialog_parent_ = nullptr;
  QAction *open_ = nullptr;
  QAction *update_grid_ = nullptr;
  QPointer<OpenDialog> open_dialog_;
  QPointer<GridDialog> grid_dialog_;
  QPointer<QDialog> message_;
  QLabel *message_text_ = nullptr;
  QString status_;
  bool open_enabled_ = true;
  bool files_enabled_ = false;
  QStringList files_;
  QList<int> selected_;
  /* the row count kept during a rebuild */
  int saved_count_ = 0;
};

}  // namespace mbevq

#endif /* MBEDITVIZ_QT_CONTROLLER_H */
