/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_window.cc  11/8/2026
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

#include "mbeditviz_qt_window.h"

#include <QCloseEvent>
#include <QMenuBar>

#include "mbeditviz_qt_controller.h"
#include "mbeditviz_qt_filelist.h"

MbeditvizWindow::MbeditvizWindow(mbevq::SwathController *controller) {
  if (controller == nullptr) {
    own_controller_ = std::make_unique<mbevq::SwathController>();
    controller = own_controller_.get();
  }
  controller_ = controller;
  controller_->setDialogParent(this);
  setWindowTitle(QStringLiteral("MBeditviz"));

  /* the File, Options and Help menus, as in Motif */
  for (QMenu *menu : controller_->menus())
    menuBar()->addMenu(menu);
  connect(controller_, &mbevq::SwathController::closeRequested, this, &QWidget::close);

  file_list_ = new mbevq::FileListWidget(controller_);
  setCentralWidget(file_list_);
  resize(453, 557);
}

MbeditvizWindow::~MbeditvizWindow() {
  if (controller_->dialogParent() == this)
    controller_->setDialogParent(nullptr);
}

void MbeditvizWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif Quit: unload the files (unlocking them), then the program */
  if (!quitting_) {
    quitting_ = true;
    controller_->prepareToClose();
  }
  event->accept();
  emit closed();
}
