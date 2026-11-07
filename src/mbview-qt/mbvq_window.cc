/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_window.cc  11/7/2026
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

#include "mbvq_window.h"

#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QRadioButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <algorithm>

#include "mbv_ui.h"
#include "mbvq_backend.h"
#include "mbvq.h"
#include "mbvq_bridge.h"

namespace mbvq {

namespace {

/* the mouse modes in the order of the Motif menu, with their MBV_MOUSE_* values */
struct MouseMode {
  int mode;
  const char *label;
};
constexpr MouseMode kMouseMode[] = {{0, "Pan and Zoom"}, {1, "Rotate Model"}, {3, "Rotate View"},
                                    {2, "Shading"},      {4, "Pick Area"},    {5, "Edit Sites"},
                                    {6, "Edit Routes"},  {7, "Select Nav"},   {8, "Select Nav File"}};
constexpr int kMouseRotate = 1;    /* MBV_MOUSE_ROTATE */
constexpr int kMouseViewpoint = 3; /* MBV_MOUSE_VIEWPOINT */

}  // namespace

MapWindow::MapWindow(size_t instance) : instance_(instance) {
  setAttribute(Qt::WA_DeleteOnClose, false);
  setWindowTitle(QString::fromLocal8Bit(mbvq_c_title(instance)));

  /* mouse modes: a menu and radio buttons, as in the Motif window */
  QMenu *mouse_menu = menuBar()->addMenu(tr("&Mouse"));
  mouse_actions_ = new QActionGroup(this);
  mouse_actions_->setExclusive(true);
  auto *mouse_box = new QGroupBox(tr("Mouse Mode"));
  auto *mouse_layout = new QVBoxLayout(mouse_box);
  mouse_buttons_ = new QButtonGroup(this);
  for (int i = 0; i < kMouseModes; i++) {
    const int mode = kMouseMode[i].mode;
    mouse_action_[i] = mouse_menu->addAction(tr(kMouseMode[i].label));
    mouse_action_[i]->setCheckable(true);
    mouse_actions_->addAction(mouse_action_[i]);
    connect(mouse_action_[i], &QAction::triggered, this, [this, mode] { mbvq_c_set_mouse_mode(instance_, mode); });
    mouse_button_[i] = new QRadioButton(tr(kMouseMode[i].label));
    mouse_buttons_->addButton(mouse_button_[i], mode);
    mouse_layout->addWidget(mouse_button_[i]);
    connect(mouse_button_[i], &QRadioButton::clicked, this, [this, mode] { mbvq_c_set_mouse_mode(instance_, mode); });
  }

  /* the pick information never changes the panel width: a width change would
     resize the map view and redraw it at low resolution (flicker) */
  pick_info_ = new QLabel;
  pick_info_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  pick_info_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pick_info_->setWordWrap(true);
  pick_info_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  auto *left = new QWidget;
  auto *left_layout = new QVBoxLayout(left);
  left_layout->addWidget(mouse_box);
  left_layout->addWidget(pick_info_);
  left_layout->addStretch(1);
  const QMargins margins = left_layout->contentsMargins();
  const int text_width = pick_info_->fontMetrics().horizontalAdvance(QStringLiteral(" Lat 2:  -89  59.999999 S "));
  left->setFixedWidth(std::max(mouse_box->sizeHint().width(), text_width) + margins.left() + margins.right());

  int xo = 0, yo = 0, width = 0, height = 0;
  mbvq_c_geometry(instance, &xo, &yo, &width, &height);
  auto *central = new QWidget;
  auto *layout = new QHBoxLayout(central);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(left);
  map_view_ = backend() == Backend::LegacyGL ? gl_backend::create_map_view(instance, central)
                                             : null_backend::create_map_view(instance, central);
  map_view_->setMinimumSize(100, 100);
  map_view_->resize(width, height);
  layout->addWidget(map_view_, 1);
  setCentralWidget(central);

  status_ = new QLabel;
  statusBar()->addWidget(status_, 1);

  move(xo, yo);
  resize(width + left->sizeHint().width(), height + menuBar()->sizeHint().height() + statusBar()->sizeHint().height());
  set_map_cursor(MBV_CURSOR_TARGET_BLACK);
}

void MapWindow::refresh_modes() {
  const int current = mbvq_c_mouse_mode(instance_);
  const bool display_3d = mbvq_c_display_3d(instance_);
  for (int i = 0; i < kMouseModes; i++) {
    const int mode = kMouseMode[i].mode;
    const bool needs_3d = mode == kMouseRotate || mode == kMouseViewpoint;
    mouse_action_[i]->setEnabled(display_3d || !needs_3d);
    mouse_button_[i]->setEnabled(display_3d || !needs_3d);
    mouse_action_[i]->setChecked(mode == current);
    mouse_button_[i]->setChecked(mode == current);
  }
}

void MapWindow::set_status(const QString &text) { status_->setText(text); }

void MapWindow::set_label(int label, const QString &text) {
  if (label < 0 || label >= MBVQ_LABEL_OTHER)
    return;
  labels_[label] = text;
  if (label == MBVQ_LABEL_PICKINFO)
    pick_info_->setText(text);
}

QString MapWindow::label(int label) const {
  return label >= 0 && label < MBVQ_LABEL_OTHER ? labels_[label] : QString();
}

void MapWindow::set_map_cursor(int cursor) {
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
  map_view_->setCursor(shape);
}

void MapWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif dismiss button: the program decides what follows */
  event->accept();
  mbvq_c_dismiss(instance_);
}

}  // namespace mbvq
