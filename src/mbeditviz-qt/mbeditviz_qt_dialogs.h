/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_dialogs.h  11/10/2026
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
 * The Open Swath Data and Grid Parameters dialogs of mbeditviz-qt, as the
 * Motif mbeditviz. The controller (mbeditviz_qt_controller.h) opens them.
 */

#ifndef MBEDITVIZ_QT_DIALOGS_H
#define MBEDITVIZ_QT_DIALOGS_H

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QRadioButton;
class QSlider;

namespace mbevq {
class SwathController;
}

/* "Open Swath Data": output mode, MBIO format and the file */
class OpenDialog final : public QDialog {
 public:
  OpenDialog(mbevq::SwathController *controller, QWidget *parent);
  void show_mode();

 private:
  void browse();
  void open_file();

  mbevq::SwathController *controller_;
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

#endif /* MBEDITVIZ_QT_DIALOGS_H */
