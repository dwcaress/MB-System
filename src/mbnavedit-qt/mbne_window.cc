/*--------------------------------------------------------------------
 *    The MB-system:  mbne_window.cc  9/11/2026
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

#include "mbne_window.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

namespace mbne {

NavEditWindow::NavEditWindow(QWidget *parent) : QMainWindow(parent) {
  editor_ = new NavEditWidget;
  setCentralWidget(editor_);

  for (QMenu *menu : editor_->menus())
    menuBar()->addMenu(menu);
  QMenu *file = editor_->menus().front();
  file->addSeparator();
  QAction *quit = file->addAction("Quit", this, &QWidget::close);
  quit->setShortcut(QKeySequence::Quit);
  quit->setMenuRole(QAction::QuitRole);

  setWindowTitle(editor_->title());
  connect(editor_, &NavEditWidget::titleChanged, this, &QWidget::setWindowTitle);
  connect(editor_, &NavEditWidget::unsavedChanged, this, &QWidget::setWindowModified);
  connect(editor_, &NavEditWidget::statusMessage, statusBar(), &QStatusBar::showMessage);
  connect(editor_, &NavEditWidget::busyChanged, this, [this](bool busy) { menuBar()->setEnabled(!busy); });
  connect(editor_, &NavEditWidget::closeRequested, this, &QWidget::close);

  statusBar()->showMessage("Ready");
  resize(1300, 1000);
}

void NavEditWindow::closeEvent(QCloseEvent *event) {
  if (editor_->prepareToClose([this] { close(); }))
    event->accept();
  else
    event->ignore();
}

}  // namespace mbne
