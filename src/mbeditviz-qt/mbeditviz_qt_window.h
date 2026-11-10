/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_window.h  11/8/2026
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
 * The windows of mbeditviz-qt, as the Motif mbeditviz: the main window (status,
 * mode, file list and buttons, File, Options and Help menus) and the Open Swath
 * Data, Grid Parameters, About, error and "Please Wait..." dialogs.
 */

#ifndef MBEDITVIZ_QT_WINDOW_H
#define MBEDITVIZ_QT_WINDOW_H

#include <QDialog>
#include <QMainWindow>
#include <QStringList>

class QAction;
class QButtonGroup;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSlider;

class OpenDialog;
class GridDialog;

class MbeditvizWindow final : public QMainWindow {
 public:
  MbeditvizWindow();
  static MbeditvizWindow *instance();

  void set_status(const QString &text);
  void set_sensitivity(bool open, bool files);
  /* the edit or browse mode, shown in the main window and the open dialog */
  void show_mode();
  QListWidget *list() const { return list_; }
  void show_message(const QString &message, bool on);
  void about();
  GridDialog *grid_dialog();

  /* the file list rebuild (mbeditviz_qt_list_begin/end) */
  void list_begin();
  void list_add(const QString &label);
  void list_end();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  QLabel *status_ = nullptr;
  QRadioButton *edit_ = nullptr;
  QRadioButton *browse_ = nullptr;
  QPushButton *delete_ = nullptr;
  QPushButton *view_selected_ = nullptr;
  QPushButton *view_all_ = nullptr;
  QAction *open_ = nullptr;
  QAction *update_grid_ = nullptr;
  QListWidget *list_ = nullptr;
  OpenDialog *open_dialog_ = nullptr;
  GridDialog *grid_dialog_ = nullptr;
  QDialog *message_ = nullptr;
  QLabel *message_text_ = nullptr;
  /* the selection and row count kept during a rebuild */
  QList<int> saved_rows_;
  int saved_count_ = 0;
  bool quitting_ = false;
};

/* "Open Swath Data": output mode, MBIO format and the file */
class OpenDialog final : public QDialog {
 public:
  explicit OpenDialog(QWidget *parent);
  void show_mode();

 private:
  void browse();
  void open_file();

  QRadioButton *edit_ = nullptr;
  QRadioButton *browse_ = nullptr;
  QLineEdit *format_ = nullptr;
  QLineEdit *path_ = nullptr;
  QString directory_;
};

/* "Grid Parameters": cell size, algorithm, interpolation; Apply makes the grid */
class GridDialog final : public QDialog {
 public:
  explicit GridDialog(QWidget *parent);
  /* show the values of the core (do_mbeditviz_gridparameters) */
  void load();

 private:
  void changed();
  void show_texts();

  bool loading_ = false;
  QSlider *cellsize_ = nullptr;
  QLabel *cellsize_value_ = nullptr;
  QLabel *current_ = nullptr;
  QLabel *implied_ = nullptr;
  QRadioButton *algorithm_[3] = {};
  QLineEdit *interpolation_ = nullptr;
};

/* initialize the user interface and the views, as do_mbeditviz_init and the
   Motif main: modes, status, mbview and the 3D soundings view, and the file
   list timer; before mbeditviz_init() reads the command line */
void mbeditviz_qt_start(MbeditvizWindow *window);

#endif /* MBEDITVIZ_QT_WINDOW_H */
