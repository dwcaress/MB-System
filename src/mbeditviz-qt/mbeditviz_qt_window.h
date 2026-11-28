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
 * The main window of mbeditviz-qt, as the Motif mbeditviz main window: the
 * File, Options and Help menus of the controller (mbeditviz_qt_controller.h)
 * and its status, mode, buttons and file list (mbeditviz_qt_filelist.h).
 * Closing it unloads the files.
 */

#ifndef MBEDITVIZ_QT_WINDOW_H
#define MBEDITVIZ_QT_WINDOW_H

#include <QMainWindow>
#include <memory>

namespace mbevq {
class FileListWidget;
class SwathController;
}  // namespace mbevq

class MbeditvizWindow final : public QMainWindow {
  Q_OBJECT

 public:
  /* without a controller the window makes its own */
  explicit MbeditvizWindow(mbevq::SwathController *controller = nullptr);
  ~MbeditvizWindow() override;

  mbevq::SwathController *controller() const { return controller_; }
  mbevq::FileListWidget *fileList() const { return file_list_; }

 signals:
  /* the window has closed, and the files are unloaded */
  void closed();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  std::unique_ptr<mbevq::SwathController> own_controller_;
  mbevq::SwathController *controller_;
  mbevq::FileListWidget *file_list_ = nullptr;
  bool quitting_ = false;
};

#endif /* MBEDITVIZ_QT_WINDOW_H */
