/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_window.h  9/11/2026
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

#ifndef MBVELOCITYTOOL_QT_MBVT_WINDOW_H
#define MBVELOCITYTOOL_QT_MBVT_WINDOW_H

#include <QMainWindow>

#include "mbvt_widget.h"

namespace mbvt {

/* The window of the program mbvelocitytool-qt: the tool with a menu bar, tool bar, status bar and title. */
class VelocityWindow : public QMainWindow {
  Q_OBJECT

 public:
  /* quiet: errors go to stderr instead of dialogs, and nothing asks anything */
  VelocityWindow(int verbose, int nthreads, int anglemode, const int btime_i[7], const int etime_i[7], bool quiet);

  VelocityWidget *editor() { return editor_; }
  bool saveImage(const QString &path, int width, int height);

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  VelocityWidget *editor_;
};

}  // namespace mbvt

#endif
