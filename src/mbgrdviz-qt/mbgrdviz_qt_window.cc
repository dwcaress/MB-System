/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_window.cc  11/7/2026
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

#include "mbgrdviz_qt_window.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMenuBar>

#include "mbgrdviz_qt_controller.h"

extern "C" {
#include "mb_define.h"
}

MbgrdvizWindow::MbgrdvizWindow(mbgvq::Controller *controller) {
  if (controller == nullptr) {
    own_controller_ = std::make_unique<mbgvq::Controller>();
    controller = own_controller_.get();
  }
  controller_ = controller;
  controller_->setDialogParent(this);
  setWindowTitle(QStringLiteral("MBgrdviz"));

  /* the File, Realtime and Help menus, as in Motif */
  for (QMenu *menu : controller_->menus())
    menuBar()->addMenu(menu);
  connect(controller_, &mbgvq::Controller::closeRequested, this, &QWidget::close);

  auto *label = new QLabel(QStringLiteral("MBgrdviz\nMB-System %1").arg(QString::fromLatin1(MB_VERSION)));
  label->setAlignment(Qt::AlignCenter);
  setCentralWidget(label);
  move(108, 241);
  resize(260, 215);
}

MbgrdvizWindow::~MbgrdvizWindow() {
  if (controller_->dialogParent() == this)
    controller_->setDialogParent(nullptr);
}

void MbgrdvizWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif Quit: close the mbview windows, then the program */
  controller_->prepareToClose();
  event->accept();
  emit closed();
}
