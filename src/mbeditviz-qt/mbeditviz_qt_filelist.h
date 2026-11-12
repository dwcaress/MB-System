/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_filelist.h  11/10/2026
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
 * The content of the Motif mbeditviz main window as a widget: the status,
 * the edit or browse mode, the Remove Selected, View Selected and View All
 * buttons and the list of swath files. It shows the state of a
 * SwathController (mbeditviz_qt_controller.h) and passes the buttons and the
 * selection to it.
 */

#ifndef MBEDITVIZ_QT_FILELIST_H
#define MBEDITVIZ_QT_FILELIST_H

#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;

namespace mbevq {

class SwathController;

class FileListWidget final : public QWidget {
 public:
  explicit FileListWidget(SwathController *controller, QWidget *parent = nullptr);

  SwathController *controller() const { return controller_; }
  QListWidget *list() const { return list_; }

 private:
  void show_mode();
  void show_sensitivity();
  void show_files();

  SwathController *controller_;
  QLabel *status_ = nullptr;
  QRadioButton *edit_ = nullptr;
  QRadioButton *browse_ = nullptr;
  QPushButton *delete_ = nullptr;
  QPushButton *view_selected_ = nullptr;
  QPushButton *view_all_ = nullptr;
  QListWidget *list_ = nullptr;
};

}  // namespace mbevq

#endif /* MBEDITVIZ_QT_FILELIST_H */
