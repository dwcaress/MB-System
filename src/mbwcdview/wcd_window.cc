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

#include <QDockWidget>
#include <QEvent>
#include <QStatusBar>

#include <algorithm>
#include <cstdio>

namespace wcd {

WaterColumnWindow::WaterColumnWindow(QWidget *parent) : QMainWindow(parent) {
  editor_ = new WaterColumnWidget;
  setCentralWidget(editor_);
  setWindowTitle(editor_->title());
  connect(editor_, &WaterColumnWidget::statusMessage, statusBar(), &QStatusBar::showMessage);
  connect(editor_, &WaterColumnWidget::diagnostic, this,
          [](const QString &message) { fprintf(stderr, "[diag] %s\n", message.toStdString().c_str()); });
  pick_table_ = editor_->takePickTable();
  pick_table_->setContentsMargins(6, 6, 6, 6);
  connect(editor_, &WaterColumnWidget::pickTableWanted, this, &WaterColumnWindow::showPickDock);

  statusBar()->showMessage("Ready");
  resize(1400, 900);
}

/* the table opens as a floating window, and the user can dock it to keep it in view */
void WaterColumnWindow::showPickDock() {
  if (pick_dock_ == nullptr) {
    pick_dock_ = new QDockWidget("Picked Positions", this);
    pick_dock_->setObjectName("dock_picks");
    pick_dock_->setWidget(pick_table_);
    pick_table_->show();
    addDockWidget(Qt::BottomDockWidgetArea, pick_dock_);
    pick_dock_->setFloating(true);
    pick_dock_->resize(std::max(600, width()), std::max(240, height() / 3));
    pick_dock_->installEventFilter(this);
    connect(pick_table_, &PickTable::closeRequested, pick_dock_, &QWidget::hide);
  }
  pick_dock_->show();
  if (!pick_shown_once_) {
    pick_dock_->raise();
    pick_dock_->activateWindow();
    pick_shown_once_ = true;
  }
}

bool WaterColumnWindow::eventFilter(QObject *watched, QEvent *event) {
  if (watched == pick_dock_ && event->type() == QEvent::Close)
    editor_->pickTableClosed();
  return QMainWindow::eventFilter(watched, event);
}

}  // namespace wcd
