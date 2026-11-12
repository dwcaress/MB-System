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
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <vector>

#include "mbv_ui.h"
#include "mbvq.h"
#include "mbvq_backend.h"

namespace mbvq {

namespace {

SoundingsWidget *the_widget = nullptr;
SoundingsWindow *the_window = nullptr;
std::function<void(SoundingsWidget *)> placement;

/* Destroyed windows or widgets, hidden; deleted at a safe point (the next
   one, or when the program quits), never by a posted deferred deletion, for
   the same reason as the maps (mbvq_ui.cc). */
std::vector<QPointer<QWidget>> closed;
bool quit_connected = false;

void delete_closed() {
  for (const QPointer<QWidget> &widget : closed)
    delete widget.data();
  closed.clear();
}

/* the standalone window if there is one, otherwise the widget */
QWidget *outermost() { return the_window != nullptr ? static_cast<QWidget *>(the_window) : the_widget; }

/* the bias sliders in the order of the Motif window */
const int kBiasOrder[MBVQ_SDG_BIASES] = {MBVQ_SDG_BIAS_HEADING, MBVQ_SDG_BIAS_ROLL, MBVQ_SDG_BIAS_PITCH,
                                         MBVQ_SDG_BIAS_TIMELAG, MBVQ_SDG_BIAS_SNELL};
const char *const kBiasLabels[MBVQ_SDG_BIASES] = {"Roll Bias (degrees)", "Pitch Bias (degrees)",
                                                  "Heading Bias (degrees)", "Time Lag (seconds)", "Snell Correction"};

/* the slider value as Motif shows it: hundredths, snell ten-thousandths */
QString bias_text(int bias, int value) {
  return bias == MBVQ_SDG_BIAS_SNELL ? QString::number(value / 10000.0, 'f', 4)
                                     : QString::number(value / 100.0, 'f', 2);
}

/* the range around a value, as the Motif mb3dsoundings_updategui() */
void set_bias_range(QSlider *slider, int value) {
  const QSignalBlocker blocker(slider);
  slider->setRange(100 * (value / 100) - 100, 100 * (value / 100) + 100);
  slider->setValue(value);
}

/* the edit modes in the order of the Motif radio box */
const char *const kEditLabels[MBVQ_SDG_EDIT_MODES] = {"Toggle", "Pick", "Erase", "Restore", "Grab", "Info"};

}  // namespace

SoundingsWindow *soundings_window() { return the_window; }

SoundingsWidget *soundings_widget() { return the_widget; }

void set_soundings_placement(std::function<void(SoundingsWidget *)> place) { placement = std::move(place); }

SoundingsWidget::SoundingsWidget(QWidget *parent) : QWidget(parent), title_(tr("3D Soundings")) {

  /* View menu, as in Motif */
  QMenu *view_menu = new_menu(tr("View"));
  const auto add_view = [this, view_menu](int toggle, const char *label) {
    view_toggle_[toggle] = view_menu->addAction(tr(label));
    view_toggle_[toggle]->setCheckable(true);
    connect(view_toggle_[toggle], &QAction::triggered, this,
            [toggle](bool on) { mbvq_c_soundings_viewtoggle(toggle, on); });
  };
  add_view(MBVQ_SDG_VIEW_BOUNDINGBOX, "Show Bounding Box");
  view_menu->addSeparator();
  add_view(MBVQ_SDG_VIEW_FLAGGED, "Show Flagged Soundings");
  add_view(MBVQ_SDG_VIEW_SECONDARY, "View Secondary Pick Soundings");
  view_menu->addSeparator();
  auto *profile_group = new QActionGroup(this);
  const char *const profile_labels[3] = {"Show No Profiles", "Show Good Profiles", "Show All Profiles"};
  for (int mode = 0; mode < 3; mode++) {
    profiles_[mode] = view_menu->addAction(tr(profile_labels[mode]));
    profiles_[mode]->setCheckable(true);
    profile_group->addAction(profiles_[mode]);
    connect(profiles_[mode], &QAction::triggered, this, [mode] { mbvq_c_soundings_profiles(mode); });
  }
  view_menu->addSeparator();
  add_view(MBVQ_SDG_VIEW_SCALEWITHFLAGGED, "Scale with Flagged Soundings");
  view_menu->addSeparator();
  auto *color_group = new QActionGroup(this);
  const int color_modes[4] = {MBVQ_SDG_COLOR_FLAG, MBVQ_SDG_COLOR_TOPO, MBVQ_SDG_COLOR_SOUNDING, MBVQ_SDG_COLOR_AMP};
  const char *const color_labels[4] = {"Color by Flag State", "Color by Topography", "Color by Soundings",
                                       "Color by Amplitude"};
  for (int i = 0; i < 4; i++) {
    const int mode = color_modes[i];
    color_[mode] = view_menu->addAction(tr(color_labels[i]));
    color_[mode]->setCheckable(true);
    color_group->addAction(color_[mode]);
    connect(color_[mode], &QAction::triggered, this, [mode] { mbvq_c_soundings_color(mode); });
  }

  QMenu *mouse_menu = new_menu(tr("Mouse"));
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
  /* Action menu, as in Motif */
  QMenu *action_menu = new_menu(tr("Action"));
  const char *const action_labels[MBVQ_SDG_ACTIONS] = {"Apply Bias Values to Grid",
                                                       "Filter by sparse voxels (1 X cell, n<10)",
                                                       "Filter by sparse voxels (1 X cell, n<2)",
                                                       "Filter by sparse voxels (4 X cell, n<10)",
                                                       "Filter by sparse voxels (4 X cell, n<2)",
                                                       "Filter by sparse voxels (8 X cell, n<10)",
                                                       "Filter by sparse voxels (8 X cell, n<2)",
                                                       "Color Unflagged Soundings Black",
                                                       "Color Unflagged Soundings Red",
                                                       "Color Unflagged Soundings Yellow",
                                                       "Color Unflagged Soundings Green",
                                                       "Color Unflagged Soundings Bluegreen",
                                                       "Color Unflagged Soundings Blue",
                                                       "Color Unflagged Soundings Purple",
                                                       "Optimize Bias Values (roll)",
                                                       "Optimize Bias Values (pitch)",
                                                       "Optimize Bias Values (heading)",
                                                       "Optimize Bias Values (roll-pitch)",
                                                       "Optimize Bias Values (roll-pitch-heading)",
                                                       "Optimize Time Lag Values",
                                                       "Optimize Snell Correction Values"};
  for (int action = 0; action < MBVQ_SDG_ACTIONS; action++) {
    if (action == MBVQ_SDG_ACTION_SPARSEVOXELS || action == MBVQ_SDG_ACTION_COLOR ||
        action == MBVQ_SDG_ACTION_OPTIMIZE)
      action_menu->addSeparator();
    connect(action_menu->addAction(tr(action_labels[action])), &QAction::triggered, this,
            [action] { mbvq_c_soundings_action(action); });
  }

  QMenu *dismiss_menu = new_menu(tr("Dismiss"));
  connect(dismiss_menu->addAction(tr("Dismiss")), &QAction::triggered, this, [this] { dismiss(); });

  /* compact: the controls take a few lines, the drawing the rest */
  layout_ = new QVBoxLayout(this);
  auto *layout = layout_;
  layout->setContentsMargins(8, 4, 8, 4);
  layout->setSpacing(2);

  /* first line: the edit modes (the Motif radio box), Reset View and the
     mouse mode buttons */
  auto *edit_row = new QHBoxLayout;
  auto *edit_group = new QButtonGroup(this);
  for (int mode = 0; mode < MBVQ_SDG_EDIT_MODES; mode++) {
    edit_[mode] = new QRadioButton(tr(kEditLabels[mode]));
    edit_group->addButton(edit_[mode], mode);
    edit_row->addWidget(edit_[mode]);
    connect(edit_[mode], &QRadioButton::clicked, this, [mode] { mbvq_c_soundings_edit_mode(mode); });
  }
  edit_row->addStretch();
  auto *reset = new QPushButton(tr("Reset View"));
  connect(reset, &QPushButton::clicked, this, [] { mbvq_c_soundings_resetview(); });
  edit_row->addWidget(reset);
  auto *mouse_group = new QButtonGroup(this);
  const char *const button_labels[2] = {"Pan and Zoom", "Rotate Soundings"};
  const int button_modes[2] = {MBVQ_SDG_MOUSE_PANZOOM, MBVQ_SDG_MOUSE_ROTATE};
  for (int i = 0; i < 2; i++) {
    const int mode = button_modes[i];
    mouse_[mode] = new QRadioButton(tr(button_labels[i]));
    mouse_group->addButton(mouse_[mode], mode);
    edit_row->addWidget(mouse_[mode]);
    connect(mouse_[mode], &QRadioButton::clicked, this, [mode] { mbvq_c_soundings_mouse_mode(mode); });
  }
  layout->addLayout(edit_row);

  /* second line: the status and the mouse mode help, each on one line; a
     long status is cut rather than widening the editor */
  auto *info_row = new QHBoxLayout;
  status_ = new QLabel;
  status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  status_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  info_row->addWidget(status_, 1);
  /* the default text of the Motif label until the first mode change */
  mousemode_ = new QLabel;
  mousemode_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  set_mousemode(label_text(":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Rotate Soundings\"\"R: Exageration\""));
  info_row->addWidget(mousemode_);
  layout->addLayout(info_row);

  /* the bias and time lag sliders, released values go to the core */
  sliders_ = new QWidget;
  auto *slider_row = new QHBoxLayout(sliders_);
  slider_row->setContentsMargins(0, 0, 0, 0);
  slider_row->setSpacing(16);
  for (int bias : kBiasOrder) {
    auto *box = new QVBoxLayout;
    box->setSpacing(0);
    box->addWidget(new QLabel(tr(kBiasLabels[bias])));
    bias_[bias] = new QSlider(Qt::Horizontal);
    bias_[bias]->setTracking(false);
    bias_value_[bias] = new QLabel;
    auto *line = new QHBoxLayout;
    line->addWidget(bias_[bias], 1);
    line->addWidget(bias_value_[bias]);
    box->addLayout(line);
    slider_row->addLayout(box);
    QSlider *slider = bias_[bias];
    QLabel *value_label = bias_value_[bias];
    connect(slider, &QSlider::sliderMoved, value_label,
            [bias, value_label](int value) { value_label->setText(bias_text(bias, value)); });
    connect(slider, &QSlider::valueChanged, this, [bias, slider, value_label](int value) {
      value_label->setText(bias_text(bias, value));
      mbvq_c_soundings_bias(bias, value);
      /* at an end of the range, centre the range on the value (Motif) */
      if (value == slider->minimum() || value == slider->maximum()) {
        const QSignalBlocker blocker(slider);
        slider->setRange(value - 100, value + 100);
        slider->setValue(value);
      }
    });
  }
  layout->addWidget(sliders_);

  view_ = backend() == Backend::LegacyGL ? gl_backend::create_soundings_view(this)
                                         : null_backend::create_soundings_view(this);
  view_->setMinimumSize(200, 150);
  layout->addWidget(view_, 1);
  set_view_cursor(MBV_CURSOR_TARGET_RED);
  refresh();
}

QMenu *SoundingsWidget::new_menu(const QString &title) {
  auto *menu = new QMenu(title, this);
  menus_.append(menu);
  return menu;
}

void SoundingsWidget::setTitle(const QString &title) {
  if (title == title_)
    return;
  title_ = title;
  emit titleChanged(title_);
}

void SoundingsWidget::setMenuBarShown(bool shown) {
  if (menu_bar_ == nullptr && shown) {
    menu_bar_ = new QMenuBar(this);
    /* on macOS a native bar of a widget inside a window would not be seen */
    menu_bar_->setNativeMenuBar(false);
    for (QMenu *menu : menus_)
      menu_bar_->addMenu(menu);
    layout_->insertWidget(0, menu_bar_);
  }
  if (menu_bar_ != nullptr)
    menu_bar_->setVisible(shown);
}

void SoundingsWidget::dismiss() { mbvq_c_soundings_dismiss(); }

bool SoundingsWidget::prepareToClose(std::function<void()> closed_later) {
  (void)closed_later;
  dismiss();
  return true;
}

void SoundingsWidget::refresh() {
  int edit_mode = 0;
  int mouse_mode = 0;
  mbvq_c_soundings_modes(&edit_mode, &mouse_mode);
  if (edit_mode >= 0 && edit_mode < MBVQ_SDG_EDIT_MODES)
    edit_[edit_mode]->setChecked(true);
  if (mouse_mode == MBVQ_SDG_MOUSE_ROTATE || mouse_mode == MBVQ_SDG_MOUSE_PANZOOM) {
    mouse_[mouse_mode]->setChecked(true);
    mouse_action_[mouse_mode]->setChecked(true);
  }
  struct mbvq_soundings_state state;
  mbvq_c_soundings_state(&state);
  for (int toggle = 0; toggle < 4; toggle++)
    view_toggle_[toggle]->setChecked(state.view[toggle]);
  if (state.profiles >= 0 && state.profiles < 3)
    profiles_[state.profiles]->setChecked(true);
  if (state.color >= 0 && state.color < 4)
    color_[state.color]->setChecked(true);
  for (int bias = 0; bias < MBVQ_SDG_BIASES; bias++) {
    set_bias_range(bias_[bias], state.bias[bias]);
    bias_value_[bias]->setText(bias_text(bias, state.bias[bias]));
  }
}

void SoundingsWidget::set_mousemode(const QString &text) {
  /* the Motif label has a line per button; here they share one line */
  mousemode_->setText(QString(text).replace(QLatin1Char('\n'), QStringLiteral("   ")));
}

void SoundingsWidget::set_status(const QString &text, bool sliders) {
  status_->setText(text);
  emit statusMessage(text);
  /* as in Motif, the sliders give way to the sounding information */
  sliders_->setVisible(sliders);
}

void SoundingsWidget::set_view_cursor(int cursor) {
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

void SoundingsWidget::delete_view() {
  delete view_;
  view_ = nullptr;
}

SoundingsWindow::SoundingsWindow(SoundingsWidget *widget) : widget_(widget) {
  setWindowTitle(widget->title());
  connect(widget, &SoundingsWidget::titleChanged, this, &QWidget::setWindowTitle);
  for (QMenu *menu : widget->menus())
    menuBar()->addMenu(menu);
  setCentralWidget(widget);
  resize(1040, 600);
}

void SoundingsWindow::closeEvent(QCloseEvent *event) {
  /* as Dismiss: hide the window and tell the calling program */
  event->ignore();
  widget_->dismiss();
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
  if (mbvq::the_widget != nullptr)
    return;
  mbvq::the_widget = new mbvq::SoundingsWidget;
  if (mbvq::placement)
    mbvq::placement(mbvq::the_widget);
  else
    mbvq::the_window = new mbvq::SoundingsWindow(mbvq::the_widget);
}

void mbvq_qt_soundings_show(void) {
  if (mbvq::the_widget == nullptr)
    return;
  QWidget *top = mbvq::outermost();
  top->show();
  top->raise();
  if (mbvq::the_widget->view() != nullptr)
    mbvq::the_widget->view()->setFocus();
  emit mbvq::the_widget->opened();
}

void mbvq_qt_soundings_hide(void) {
  if (mbvq::the_widget == nullptr)
    return;
  mbvq::outermost()->hide();
  emit mbvq::the_widget->closed();
}

void mbvq_qt_soundings_destroy(void) {
  /* possibly inside an event of the window: hide it now, free its OpenGL
     view now, delete the window at a safe point */
  mbvq::SoundingsWidget *widget = mbvq::the_widget;
  if (widget == nullptr)
    return;
  QWidget *top = mbvq::outermost();
  mbvq::the_widget = nullptr;
  mbvq::the_window = nullptr;
  top->hide();
  widget->delete_view();
  mbvq::closed.push_back(top);
  emit widget->closed();
}

void mbvq_qt_soundings_refresh(void) {
  if (mbvq::the_widget != nullptr)
    mbvq::the_widget->refresh();
}

void mbvq_qt_soundings_mousemode(const char *bxstring) {
  if (mbvq::the_widget != nullptr)
    mbvq::the_widget->set_mousemode(mbvq::label_text(bxstring));
}

void mbvq_qt_soundings_status(const char *text, bool sliders) {
  if (mbvq::the_widget != nullptr)
    mbvq::the_widget->set_status(QString::fromLocal8Bit(text), sliders);
}

void mbvq_qt_soundings_cursor(int cursor) {
  if (mbvq::the_widget != nullptr)
    mbvq::the_widget->set_view_cursor(cursor);
}

}  // extern "C"
