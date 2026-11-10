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
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <algorithm>

#include "mbv_ui.h"
#include "mbvq.h"
#include "mbvq_backend.h"
#include "mbvq_dialogs.h"
#include "mbvq_lists.h"
#include "mbvq_profile.h"

namespace mbvq {

namespace {

/* the mouse modes in the order of the Motif window (MBV_MOUSE_* values) */
constexpr int kMouseOrder[MBVQ_MOUSE_MODES] = {0, 1, 3, 2, 4, 5, 6, 7, 8};

/* a label for Qt menus and buttons, where '&' marks a shortcut key */
QString plain(const char *text) { return QString::fromLatin1(text).replace(QLatin1Char('&'), QStringLiteral("&&")); }

/* the View menu, in the order of the Motif View menu; 0 marks a separator */
struct ViewItem {
  int item;
  const char *label;
  int group; /* items with the same nonzero group are exclusive */
};
constexpr ViewItem kViewMenu[] = {
    {MBVQ_VIEW_DISPLAY_2D, "Map Display", 1},
    {MBVQ_VIEW_DISPLAY_3D, "3D Display", 1},
    {-1, nullptr, 0},
    {MBVQ_VIEW_GRID_PRIMARY, "Topography", 2},
    {MBVQ_VIEW_GRID_PRIMARYSLOPE, "Topography Slope", 2},
    {MBVQ_VIEW_GRID_SECONDARY, "Overlay", 2},
    {-1, nullptr, 0},
    {MBVQ_VIEW_HISTOGRAM, "Histogram Equalization", 0},
    {-1, nullptr, 0},
    {MBVQ_VIEW_SHADE_NONE, "No Shading", 3},
    {MBVQ_VIEW_SHADE_ILLUMINATION, "Shading by Illumination", 3},
    {MBVQ_VIEW_SHADE_SLOPE, "Shading by Slope", 3},
    {MBVQ_VIEW_SHADE_OVERLAY, "Shading by Overlay", 3},
    {-1, nullptr, 0},
    {MBVQ_VIEW_CONTOUR, "Topography Contours", 0},
    {MBVQ_VIEW_SITE, "Sites", 0},
    {MBVQ_VIEW_ROUTE, "Routes", 0},
    {-1, nullptr, 0},
    {MBVQ_VIEW_NAV, "Navigation", 0},
    {MBVQ_VIEW_NAVSWATHBOUNDS, "Navigation Swath Bounds", 0},
    {MBVQ_VIEW_NAVDRAPE, "Draped Navigation", 0},
    {MBVQ_VIEW_VECTOR, "Vector", 0},
    {MBVQ_VIEW_POLYLINE, "Polylines", 0},
    {MBVQ_VIEW_POLYGON, "Polygons", 0},
    {MBVQ_VIEW_GISPOINT, "Points", 0},
    {-1, nullptr, 0},
    {MBVQ_VIEW_COLORTABLE_HAXBY, "Haxby Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_BRIGHT, "Bright Rainbow Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_MUTED, "Muted Rainbow Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_GRAY, "Grayscale Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_FLAT, "Flat Gray Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_SEALEVEL1, "Sealevel1 Colortable", 4},
    {MBVQ_VIEW_COLORTABLE_SEALEVEL2, "Sealevel2 Colortable", 4},
    {-1, nullptr, 0},
    {MBVQ_VIEW_PROFILE, "Profile", 0},
};

}  // namespace

MapWidget::MapWidget(size_t instance, QWidget *parent) : QWidget(parent), instance_(instance) {
  title_ = QString::fromLocal8Bit(mbvq_c_title(instance));

  struct mbvq_state state;
  mbvq_c_state(instance_, &state);

  /* menus in the order of the Motif window */
  profile_ = new ProfileWindow(instance_, this);
  build_view_menu();
  dialogs_ = new ControlDialogs(instance_, this);
  QMenu *controls_menu = new_menu(tr("Controls"));
  struct ControlItem {
    const char *label;
    int dialog;
  };
  const ControlItem controls[] = {{"Colors & Contours", MBVQ_DIALOG_COLORBOUNDS}, {"2D Controls", MBVQ_DIALOG_VIEW2D},
                                  {"3D Controls", MBVQ_DIALOG_VIEW3D},         {"Shading", MBVQ_DIALOG_SHADE},
                                  {"Resolution", MBVQ_DIALOG_RESOLUTION},      {"Projections", MBVQ_DIALOG_PROJECTION}};
  for (const ControlItem &item : controls) {
    const int dialog = item.dialog;
    connect(controls_menu->addAction(plain(item.label)), &QAction::triggered, this,
            [this, dialog] { dialogs_->show_dialog(dialog); });
  }
  controls_menu->addSeparator();
  /* the list windows, shared by all instances */
  for (int list = 0; list < MBVQ_LISTS; list++)
    connect(controls_menu->addAction(QString::fromLatin1(mbvq_c_list_title(list))), &QAction::triggered, this,
            [list] { show_list(list); });
  QMenu *mouse_menu = new_menu(tr("Mouse"));
  action_menu_ = new_menu(tr("Action"));
  /* as in Motif, About is the first item of the Action menu */
  connect(action_menu_->addAction(tr("About MBview...")), &QAction::triggered, this, [this] { show_about(); });
  QMenu *dismiss_menu = new_menu(tr("Dismiss"));
  connect(dismiss_menu->addAction(tr("Dismiss")), &QAction::triggered, this, [this] { dismiss(); });

  /* mouse modes: the menu and the radio buttons, as in the Motif window */
  auto *mouse_actions = new QActionGroup(this);
  mouse_actions->setExclusive(true);
  auto *mouse_box = new QGroupBox(tr("Mouse Mode"));
  auto *mouse_layout = new QVBoxLayout(mouse_box);
  auto *mouse_buttons = new QButtonGroup(this);
  for (int mode : kMouseOrder) {
    const QString text = plain(state.mouse_label[mode]);
    mouse_action_[mode] = mouse_menu->addAction(text);
    mouse_action_[mode]->setCheckable(true);
    mouse_actions->addAction(mouse_action_[mode]);
    connect(mouse_action_[mode], &QAction::triggered, this, [this, mode] { mbvq_c_set_mouse_mode(instance_, mode); });
    mouse_button_[mode] = new QRadioButton(text);
    mouse_buttons->addButton(mouse_button_[mode], mode);
    mouse_layout->addWidget(mouse_button_[mode]);
    connect(mouse_button_[mode], &QRadioButton::clicked, this, [this, mode] { mbvq_c_set_mouse_mode(instance_, mode); });
  }

  /* the Clear, Reset and Full buttons of the Motif window */
  auto *buttons = new QWidget;
  auto *buttons_layout = new QHBoxLayout(buttons);
  buttons_layout->setContentsMargins(0, 0, 0, 0);
  auto *clear = new QPushButton(tr("Clear"));
  clear->setToolTip(tr("Clear the picks"));
  auto *reset = new QPushButton(tr("Reset"));
  reset->setToolTip(tr("Reset the view"));
  auto *full = new QPushButton(tr("Full"));
  full->setToolTip(tr("Draw at full resolution"));
  buttons_layout->addWidget(clear);
  buttons_layout->addWidget(reset);
  buttons_layout->addWidget(full);
  connect(clear, &QPushButton::clicked, this, [this] { mbvq_c_clearpicks(instance_); });
  connect(reset, &QPushButton::clicked, this, [this] { mbvq_c_resetview(instance_); });
  connect(full, &QPushButton::clicked, this, [this] { mbvq_c_fullrender(instance_); });

  /* the pick information never changes the panel width: a width change would
     resize the map view and redraw it at low resolution (flicker) */
  pick_info_ = new QLabel;
  pick_info_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  pick_info_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pick_info_->setWordWrap(true);
  pick_info_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  auto *left = new QWidget;
  control_panel_ = left;
  auto *left_layout = new QVBoxLayout(left);
  left_layout->addWidget(buttons);
  left_layout->addWidget(mouse_box);
  left_layout->addWidget(pick_info_);
  left_layout->addStretch(1);
  const QMargins margins = left_layout->contentsMargins();
  const int text_width = pick_info_->fontMetrics().horizontalAdvance(QStringLiteral(" Lat 2:  -89  59.999999 S "));
  left->setFixedWidth(std::max({mouse_box->sizeHint().width(), buttons->sizeHint().width(), text_width}) +
                      margins.left() + margins.right());

  int xo = 0, yo = 0, width = 0, height = 0;
  mbvq_c_geometry(instance, &xo, &yo, &width, &height);
  outer_ = new QVBoxLayout(this);
  outer_->setContentsMargins(0, 0, 0, 0);
  outer_->setSpacing(0);
  auto *central = new QWidget;
  auto *layout = new QHBoxLayout(central);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(left);
  map_view_ = backend() == Backend::LegacyGL ? gl_backend::create_map_view(instance, central)
                                             : null_backend::create_map_view(instance, central);
  map_view_->setMinimumSize(100, 100);
  map_view_->resize(width, height);
  layout->addWidget(map_view_, 1);
  outer_->addWidget(central, 1);

  status_ = new QLabel;
  outer_->addWidget(status_);

  set_map_cursor(MBV_CURSOR_TARGET_BLACK);
}

QSize MapWidget::sizeHint() const {
  int xo = 0, yo = 0, width = 0, height = 0;
  mbvq_c_geometry(instance_, &xo, &yo, &width, &height);
  return QSize(width + control_panel_->width(), height + status_->sizeHint().height());
}

QMenu *MapWidget::new_menu(const QString &title) {
  auto *menu = new QMenu(title, this);
  menus_.append(menu);
  return menu;
}

void MapWidget::setTitle(const QString &title) {
  if (title == title_)
    return;
  title_ = title;
  emit titleChanged(title_);
}

void MapWidget::setMenuBarShown(bool shown) {
  if (menu_bar_ == nullptr && shown) {
    menu_bar_ = new QMenuBar(this);
    /* on macOS a native bar of a widget inside a window would not be seen */
    menu_bar_->setNativeMenuBar(false);
    for (QMenu *menu : menus_)
      menu_bar_->addMenu(menu);
    outer_->insertWidget(0, menu_bar_);
  }
  if (menu_bar_ != nullptr)
    menu_bar_->setVisible(shown);
}

void MapWidget::dismiss() { mbvq_c_destroy(instance_); }

QLabel *MapWidget::takeStatusLabel() {
  outer_->removeWidget(status_);
  return status_;
}

void MapWidget::build_view_menu() {
  QMenu *menu = new_menu(tr("View"));
  QActionGroup *groups[5] = {};
  for (const ViewItem &entry : kViewMenu) {
    if (entry.item < 0) {
      menu->addSeparator();
      continue;
    }
    QAction *action = menu->addAction(tr(entry.label));
    action->setCheckable(true);
    if (entry.group > 0) {
      if (groups[entry.group] == nullptr) {
        groups[entry.group] = new QActionGroup(this);
        groups[entry.group]->setExclusive(true);
      }
      groups[entry.group]->addAction(action);
    }
    const int item = entry.item;
    if (item == MBVQ_VIEW_PROFILE)
      connect(action, &QAction::triggered, this, [this] { profile_->toggle(); });
    else
      connect(action, &QAction::triggered, this, [this, item](bool checked) { mbvq_c_view(instance_, item, checked); });
    view_action_[item] = action;
  }
}

void MapWidget::refresh_modes() {
  struct mbvq_state state;
  mbvq_c_state(instance_, &state);
  for (int item = 0; item < MBVQ_VIEW_ITEMS; item++) {
    if (view_action_[item] == nullptr)
      continue;
    view_action_[item]->setChecked(state.view_checked[item]);
    view_action_[item]->setEnabled(state.view_enabled[item]);
  }
  for (int mode = 0; mode < MBVQ_MOUSE_MODES; mode++) {
    const QString text = plain(state.mouse_label[mode]);
    mouse_action_[mode]->setText(text);
    mouse_button_[mode]->setText(text);
    mouse_action_[mode]->setEnabled(state.mouse_enabled[mode]);
    mouse_button_[mode]->setEnabled(state.mouse_enabled[mode]);
    mouse_action_[mode]->setChecked(mode == state.mouse_mode);
    mouse_button_[mode]->setChecked(mode == state.mouse_mode);
  }
}

void MapWidget::delete_map_view() {
  /* the OpenGL views go now: the map view and the profile window */
  delete profile_;
  profile_ = nullptr;
  delete map_view_;
  map_view_ = nullptr;
}

void MapWidget::show_message(const QString &text, bool on) {
  if (message_ == nullptr) {
    message_ = new QDialog(this);
    message_->setWindowTitle(title_);
    auto *layout = new QVBoxLayout(message_);
    message_label_ = new QLabel;
    layout->addWidget(message_label_);
    layout->addWidget(new QLabel(tr("Thank you for your patience.")));
  }
  if (on) {
    message_label_->setText(text);
    message_->show();
    message_->raise();
    /* as XSync and XmUpdateDisplay: show the message before the work goes on */
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
  else {
    message_->hide();
  }
}

void MapWidget::show_about() {
  if (about_ == nullptr) {
    /* the texts of the Motif About dialog */
    about_ = new QDialog(this);
    about_->setWindowTitle(tr("About MBview"));
    auto *layout = new QVBoxLayout(about_);
    auto add = [layout](const QString &text, int points) {
      auto *label = new QLabel(text);
      label->setAlignment(Qt::AlignCenter);
      QFont font = label->font();
      font.setBold(true);
      font.setPointSize(points);
      label->setFont(font);
      layout->addWidget(label);
    };
    add(label_text("::#TimesBold24\"MBview\":t\"\"#TimesBold18\"A 2D/3D Visualization API\""), 20);
    add(label(MBVQ_LABEL_ABOUTVERSION), 13);
    add(label_text("::#TimesBold14:t\"One component of the\"#TimesBold24\"MB-System\"#TimesBold14:t\"\"#TimesBold14:t\"An open "
                   "Source Software Package\"#TimesBold14:t\"for Processing and Display\"#TimesBold14\"of Swath Sonar Data\""),
        13);
    add(label_text("::#TimesBold14:t\"Created By:\"#TimesBold14\"David W. Caress   and   Dale N. Chayes \""), 13);
    /* the Qt6 version */
    add(tr("Qt6 user interface by:\nChristian dos Santos Ferreira"), 13);
    auto *places = new QHBoxLayout;
    for (const char *text : {"::#TimesBold12:t\"Monterey Bay\"#TimesBold12:t\"Aquarium\"#TimesBold12\"Research Institute\"",
                             "::#TimesBold12:t\"Lamont-Doherty\"#TimesBold12:t\"Earth Observatory\"#TimesBold12\"of Columbia University\"",
                             "::#TimesBold12:t\"MARUM\"#TimesBold12:t\"Center for Marine\"#TimesBold12:t\"Environmental Sciences\"#TimesBold12\"University of Bremen\""}) {
      auto *label = new QLabel(label_text(text));
      label->setAlignment(Qt::AlignCenter);
      places->addWidget(label);
    }
    layout->addLayout(places);
    auto *box = new QDialogButtonBox;
    connect(box->addButton(tr("Dismiss"), QDialogButtonBox::RejectRole), &QPushButton::clicked, about_, &QDialog::hide);
    layout->addWidget(box);
  }
  about_->show();
  about_->raise();
}

void MapWidget::set_status(const QString &text) {
  status_->setText(text);
  emit statusMessage(text);
}

void MapWidget::set_label(int label, const QString &text) {
  if (label < 0 || label >= MBVQ_LABEL_OTHER)
    return;
  labels_[label] = text;
  if (label == MBVQ_LABEL_PICKINFO)
    pick_info_->setText(text);
  else if (label == MBVQ_LABEL_PROJECTION && dialogs_ != nullptr)
    dialogs_->set_projection_label(text);
  else if (label == MBVQ_LABEL_PROFILEINFO && profile_ != nullptr)
    profile_->set_info(text);
}

QString MapWidget::label(int label) const {
  return label >= 0 && label < MBVQ_LABEL_OTHER ? labels_[label] : QString();
}

void MapWidget::set_map_cursor(int cursor) {
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
  if (map_view_ != nullptr)
    map_view_->setCursor(shape);
}

void MapWidget::add_action(int index, const QString &label) {
  QAction *action = action_menu_->addAction(QString(label).replace(QLatin1Char('&'), QStringLiteral("&&")));
  connect(action, &QAction::triggered, this, [this, index] { mbvq_c_action(instance_, index); });
  if (index >= static_cast<int>(actions_.size()))
    actions_.resize(index + 1, nullptr);
  actions_[index] = action;
}

void MapWidget::add_action_separator() { action_menu_->addSeparator(); }

void MapWidget::set_action_enabled(int index, bool enabled) {
  if (index >= 0 && index < static_cast<int>(actions_.size()) && actions_[index] != nullptr)
    actions_[index]->setEnabled(enabled);
}

MapWindow::MapWindow(MapWidget *widget) : widget_(widget) {
  setAttribute(Qt::WA_DeleteOnClose, false);
  setWindowTitle(widget->title());
  connect(widget, &MapWidget::titleChanged, this, &QWidget::setWindowTitle);
  for (QMenu *menu : widget->menus())
    menuBar()->addMenu(menu);
  setCentralWidget(widget);
  statusBar()->addWidget(widget->takeStatusLabel(), 1);

  int xo = 0, yo = 0, width = 0, height = 0;
  mbvq_c_geometry(widget->instance(), &xo, &yo, &width, &height);
  move(xo, yo);
  resize(width + widget->controlPanel()->width(), height + menuBar()->sizeHint().height() + statusBar()->sizeHint().height());
}

void MapWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif Dismiss menu: mbview_destroy() closes the instance and lets
     the program know; it removes this window afterwards */
  event->accept();
  if (!closing_) {
    closing_ = true;
    widget_->dismiss();
  }
}

}  // namespace mbvq
