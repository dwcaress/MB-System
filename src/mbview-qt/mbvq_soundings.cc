/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_soundings.cc  11/7/2026
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

#include "mbvq_soundings.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <vector>

#include "mbv_ui.h"
#include "mbvq.h"
#include "mbvq_backend.h"

namespace mbvq {

namespace {

SoundingsWindow *the_window = nullptr;

/* Destroyed windows, hidden; deleted at a safe point (the next window, or
   when the program quits), never by a posted deferred deletion, for the same
   reason as the map windows (mbvq_ui.cc). */
std::vector<SoundingsWindow *> closed;
bool quit_connected = false;

void delete_closed() {
  for (SoundingsWindow *window : closed)
    delete window;
  closed.clear();
}

/* the edit modes in the order of the Motif radio box */
const char *const kEditLabels[MBVQ_SDG_EDIT_MODES] = {"Toggle", "Pick", "Erase", "Restore", "Grab", "Info"};

}  // namespace

SoundingsWindow *soundings_window() { return the_window; }

SoundingsWindow::SoundingsWindow() {
  setWindowTitle(tr("3D Soundings"));

  /* menus: View and Action come with the soundings controls */
  QMenu *mouse_menu = menuBar()->addMenu(tr("Mouse"));
  auto *mouse_actions = new QActionGroup(this);
  mouse_actions->setExclusive(true);
  const int mouse_modes[2] = {MBVQ_SDG_MOUSE_ROTATE, MBVQ_SDG_MOUSE_PANZOOM};
  const char *const mouse_labels[2] = {"Rotate Soundings", "Pan and Zoom"};
  for (int i = 0; i < 2; i++) {
    const int mode = mouse_modes[i];
    mouse_action_[mode] = mouse_menu->addAction(tr(mouse_labels[i]));
    mouse_action_[mode]->setCheckable(true);
    mouse_actions->addAction(mouse_action_[mode]);
    connect(mouse_action_[mode], &QAction::triggered, this, [mode] { mbvq_c_soundings_mouse_mode(mode); });
  }
  QMenu *dismiss_menu = menuBar()->addMenu(tr("Dismiss"));
  connect(dismiss_menu->addAction(tr("Dismiss")), &QAction::triggered, this, [] { mbvq_c_soundings_dismiss(); });

  auto *central = new QWidget;
  auto *layout = new QVBoxLayout(central);
  auto *top = new QGridLayout;

  /* the edit modes, as the Motif radio box */
  auto *edit_row = new QHBoxLayout;
  auto *edit_group = new QButtonGroup(this);
  for (int mode = 0; mode < MBVQ_SDG_EDIT_MODES; mode++) {
    edit_[mode] = new QRadioButton(tr(kEditLabels[mode]));
    edit_group->addButton(edit_[mode], mode);
    edit_row->addWidget(edit_[mode]);
    connect(edit_[mode], &QRadioButton::clicked, this, [mode] { mbvq_c_soundings_edit_mode(mode); });
  }
  edit_row->addStretch();
  top->addLayout(edit_row, 0, 0);

  /* the status line */
  status_ = new QLabel;
  status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  top->addWidget(status_, 1, 0);

  /* Reset View, the mouse mode buttons and label */
  auto *right = new QVBoxLayout;
  auto *reset = new QPushButton(tr("Reset View"));
  connect(reset, &QPushButton::clicked, this, [] { mbvq_c_soundings_resetview(); });
  right->addWidget(reset);
  auto *mouse_group = new QButtonGroup(this);
  const char *const button_labels[2] = {"Pan and Zoom", "Rotate Soundings"};
  const int button_modes[2] = {MBVQ_SDG_MOUSE_PANZOOM, MBVQ_SDG_MOUSE_ROTATE};
  for (int i = 0; i < 2; i++) {
    const int mode = button_modes[i];
    mouse_[mode] = new QRadioButton(tr(button_labels[i]));
    mouse_group->addButton(mouse_[mode], mode);
    right->addWidget(mouse_[mode]);
    connect(mouse_[mode], &QRadioButton::clicked, this, [mode] { mbvq_c_soundings_mouse_mode(mode); });
  }
  /* the default text of the Motif label until the first mode change */
  mousemode_ = new QLabel(label_text(
      ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Rotate Soundings\"\"R: Exageration\""));
  /* a fixed width, so that a longer label does not resize the drawing */
  mousemode_->setFixedWidth(220);
  mousemode_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  right->addWidget(mousemode_);
  top->addLayout(right, 0, 1, 3, 1);
  top->setColumnStretch(0, 1);
  layout->addLayout(top);

  view_ = backend() == Backend::LegacyGL ? gl_backend::create_soundings_view(central)
                                         : null_backend::create_soundings_view(central);
  view_->setMinimumSize(200, 150);
  layout->addWidget(view_, 1);
  setCentralWidget(central);
  resize(1040, 600);
  set_view_cursor(MBV_CURSOR_TARGET_RED);
  refresh();
}

void SoundingsWindow::refresh() {
  int edit_mode = 0;
  int mouse_mode = 0;
  mbvq_c_soundings_modes(&edit_mode, &mouse_mode);
  if (edit_mode >= 0 && edit_mode < MBVQ_SDG_EDIT_MODES)
    edit_[edit_mode]->setChecked(true);
  if (mouse_mode == MBVQ_SDG_MOUSE_ROTATE || mouse_mode == MBVQ_SDG_MOUSE_PANZOOM) {
    mouse_[mouse_mode]->setChecked(true);
    mouse_action_[mouse_mode]->setChecked(true);
  }
}

void SoundingsWindow::set_mousemode(const QString &text) { mousemode_->setText(text); }

void SoundingsWindow::set_status(const QString &text, bool sliders) {
  status_->setText(text);
  /* the bias sliders come with the soundings controls */
  (void)sliders;
}

void SoundingsWindow::set_view_cursor(int cursor) {
  /* Qt has no coloured cursors; the shape carries the meaning */
  Qt::CursorShape shape = Qt::CrossCursor;
  switch (cursor) {
  case MBV_CURSOR_FLEUR_BLACK:
  case MBV_CURSOR_FLEUR_RED:
    shape = Qt::SizeAllCursor;
    break;
  case MBV_CURSOR_SIZING_BLACK:
  case MBV_CURSOR_SIZING_RED:
    shape = Qt::SizeFDiagCursor;
    break;
  case MBV_CURSOR_EXCHANGE_BLACK:
  case MBV_CURSOR_EXCHANGE_GREEN:
  case MBV_CURSOR_EXCHANGE_RED:
    shape = Qt::PointingHandCursor;
    break;
  default:
    shape = Qt::CrossCursor;
    break;
  }
  if (view_ != nullptr)
    view_->setCursor(shape);
}

void SoundingsWindow::delete_view() {
  delete view_;
  view_ = nullptr;
}

void SoundingsWindow::closeEvent(QCloseEvent *event) {
  /* as Dismiss: hide the window and tell the calling program */
  event->ignore();
  mbvq_c_soundings_dismiss();
}

}  // namespace mbvq

extern "C" {

void mbvq_qt_soundings_create(void) {
  /* a safe point: called by the program */
  mbvq::delete_closed();
  if (!mbvq::quit_connected) {
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [] { mbvq::delete_closed(); });
    mbvq::quit_connected = true;
  }
  if (mbvq::the_window == nullptr)
    mbvq::the_window = new mbvq::SoundingsWindow;
}

void mbvq_qt_soundings_show(void) {
  if (mbvq::the_window == nullptr)
    return;
  mbvq::the_window->show();
  mbvq::the_window->raise();
  if (mbvq::the_window->view() != nullptr)
    mbvq::the_window->view()->setFocus();
}

void mbvq_qt_soundings_hide(void) {
  if (mbvq::the_window != nullptr)
    mbvq::the_window->hide();
}

void mbvq_qt_soundings_destroy(void) {
  /* possibly inside an event of the window: hide it now, free its OpenGL
     view now, delete the window at a safe point */
  mbvq::SoundingsWindow *window = mbvq::the_window;
  if (window == nullptr)
    return;
  mbvq::the_window = nullptr;
  window->hide();
  window->delete_view();
  mbvq::closed.push_back(window);
}

void mbvq_qt_soundings_refresh(void) {
  if (mbvq::the_window != nullptr)
    mbvq::the_window->refresh();
}

void mbvq_qt_soundings_mousemode(const char *bxstring) {
  if (mbvq::the_window != nullptr)
    mbvq::the_window->set_mousemode(mbvq::label_text(bxstring));
}

void mbvq_qt_soundings_status(const char *text, bool sliders) {
  if (mbvq::the_window != nullptr)
    mbvq::the_window->set_status(QString::fromLocal8Bit(text), sliders);
}

void mbvq_qt_soundings_cursor(int cursor) {
  if (mbvq::the_window != nullptr)
    mbvq::the_window->set_view_cursor(cursor);
}

}  // extern "C"
