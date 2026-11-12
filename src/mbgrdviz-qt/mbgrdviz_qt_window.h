/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_window.h  11/7/2026
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
 * The main window of mbgrdviz-qt, as the Motif mbgrdviz main window: the
 * File, Realtime and Help menus of the controller (mbgrdviz_qt_controller.h)
 * and the program name. Closing it closes the maps.
 */

#ifndef MBGRDVIZ_QT_WINDOW_H
#define MBGRDVIZ_QT_WINDOW_H

#include <QMainWindow>
#include <memory>

namespace mbgvq {
class Controller;
}

class MbgrdvizWindow final : public QMainWindow {
  Q_OBJECT

 public:
  /* without a controller the window makes its own */
  explicit MbgrdvizWindow(mbgvq::Controller *controller = nullptr);
  ~MbgrdvizWindow() override;

  mbgvq::Controller *controller() const { return controller_; }

 signals:
  /* the window has closed, and with it the maps */
  void closed();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  std::unique_ptr<mbgvq::Controller> own_controller_;
  mbgvq::Controller *controller_;
};

#endif /* MBGRDVIZ_QT_WINDOW_H */
