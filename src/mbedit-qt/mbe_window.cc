/*--------------------------------------------------------------------
 *    The MB-system:  mbe_window.cc  8/11/2026
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

#include "mbe_window.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

#include <cstdio>

namespace mbe {

EditorWindow::EditorWindow(QWidget *parent) : QMainWindow(parent) {
  editor_ = new EditorWidget;
  setCentralWidget(editor_);

  for (QMenu *menu : editor_->menus())
    menuBar()->addMenu(menu);
  QMenu *file = editor_->menus().front();
  file->addSeparator();
  QAction *quit = file->addAction("Quit", this, &QWidget::close);
  quit->setShortcut(QKeySequence::Quit);
  quit->setMenuRole(QAction::QuitRole);

  setWindowTitle(editor_->title());
  connect(editor_, &EditorWidget::titleChanged, this, &QWidget::setWindowTitle);
  connect(editor_, &EditorWidget::unsavedChanged, this, &QWidget::setWindowModified);
  connect(editor_, &EditorWidget::statusMessage, statusBar(), &QStatusBar::showMessage);
  connect(editor_, &EditorWidget::busyChanged, this, [this](bool busy) { menuBar()->setEnabled(!busy); });
  connect(editor_, &EditorWidget::closeRequested, this, &QWidget::close);
  connect(editor_, &EditorWidget::soundingInfo, this, [](const QStringList &lines) {
    printf("\nSelected sounding:\n");
    for (const QString &line : lines)
      printf("%s\n", line.toUtf8().constData());
    fflush(stdout);
  });

  statusBar()->showMessage("Ready");
  resize(1400, 900);
}

void EditorWindow::closeEvent(QCloseEvent *event) {
  if (editor_->prepareToClose([this] { close(); }))
    event->accept();
  else
    event->ignore();
}

}  // namespace mbe
