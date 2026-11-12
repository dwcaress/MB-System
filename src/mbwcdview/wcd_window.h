/*--------------------------------------------------------------------
 *    The MB-system:  wcd_window.h  9/11/2026
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

#ifndef MBWCDVIEW_WCD_WINDOW_H
#define MBWCDVIEW_WCD_WINDOW_H

#include <QMainWindow>

#include "wcd_widget.h"

class QDockWidget;

namespace wcd {

/* The window of the program mbwcdview: the tool with a status bar and a title. */
class WaterColumnWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit WaterColumnWindow(QWidget *parent = nullptr);

  WaterColumnWidget *editor() { return editor_; }

 protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

 private:
  void showPickDock();

  WaterColumnWidget *editor_;
  PickTable *pick_table_;
  QDockWidget *pick_dock_ = nullptr;
  bool pick_shown_once_ = false;
};

}  // namespace wcd

#endif
