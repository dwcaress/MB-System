/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_window.cc  9/11/2026
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

#include "mbvt_window.h"

#include <QAction>
#include <QCloseEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>

namespace mbvt {

VelocityWindow::VelocityWindow(int verbose, int nthreads, int anglemode, const int btime_i[7], const int etime_i[7], bool quiet) {
  editor_ = new VelocityWidget(verbose, nthreads, anglemode, btime_i, etime_i, quiet);
  setCentralWidget(editor_);

  for (QMenu *menu : editor_->menus())
    menuBar()->addMenu(menu);
  QMenu *file = editor_->menus().front();
  file->addSeparator();
  QAction *quit = file->addAction("&Quit", this, &QWidget::close);
  quit->setShortcut(QKeySequence::Quit);
  quit->setMenuRole(QAction::QuitRole);

  QToolBar *tools = addToolBar("Main");
  tools->setMovable(false);
  tools->setToolButtonStyle(Qt::ToolButtonTextOnly);
  editor_->addToolActions(tools);
  setUnifiedTitleAndToolBarOnMac(true);

  statusBar()->addWidget(editor_->pointerLabel(), 1);
  statusBar()->addPermanentWidget(editor_->resultLabel());
  statusBar()->addPermanentWidget(editor_->progressBar());

  setWindowTitle(editor_->title());
  connect(editor_, &VelocityWidget::titleChanged, this, &QWidget::setWindowTitle);
  connect(editor_, &VelocityWidget::unsavedChanged, this, &QWidget::setWindowModified);
  connect(editor_, &VelocityWidget::statusMessage, statusBar(), &QStatusBar::showMessage);

  resize(VelocityWidget::initialSize());
}

bool VelocityWindow::saveImage(const QString &path, int width, int height) {
  resize(width, height);
  /* the window of the macOS style may be transparent where the desktop would show through */
  QPixmap image(size());
  image.fill(palette().color(QPalette::Window));
  render(&image);
  return image.save(path);
}

void VelocityWindow::closeEvent(QCloseEvent *event) {
  if (editor_->prepareToClose())
    event->accept();
  else
    event->ignore();
}

}  // namespace mbvt
