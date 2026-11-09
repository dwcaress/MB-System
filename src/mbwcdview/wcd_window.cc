/*--------------------------------------------------------------------
 *    The MB-system:  wcd_window.cc  9/11/2026
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

#include "wcd_window.h"

#include <QStatusBar>

#include <cstdio>

namespace wcd {

WaterColumnWindow::WaterColumnWindow(QWidget *parent) : QMainWindow(parent) {
  editor_ = new WaterColumnWidget;
  setCentralWidget(editor_);
  setWindowTitle(editor_->title());
  connect(editor_, &WaterColumnWidget::statusMessage, statusBar(), &QStatusBar::showMessage);
  connect(editor_, &WaterColumnWidget::diagnostic, this,
          [](const QString &message) { fprintf(stderr, "[diag] %s\n", message.toStdString().c_str()); });
  statusBar()->showMessage("Ready");
  resize(1400, 900);
}

}  // namespace wcd
