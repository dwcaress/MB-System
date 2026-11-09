/*--------------------------------------------------------------------
 *    The MB-system:  mbne_widget.cc  9/11/2026
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

#include "mbne_widget.h"

#include <algorithm>
#include <memory>

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHideEvent>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMetaMethod>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QSpinBox>
#include <QTableWidget>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "mb_define.h"
#include "mb_status.h"

namespace mbne {

namespace {

constexpr int TIME_LIMIT = 10000000;  // seconds
constexpr double DESPIKE_WINDOW = 60.0;
constexpr double DESPIKE_THRESHOLD = 2.0;

QString file_name(const std::string &path) { return QFileInfo(QString::fromStdString(path)).fileName(); }

QFrame *separator() {
  auto *line = new QFrame;
  line->setFrameShape(QFrame::VLine);
  line->setFrameShadow(QFrame::Sunken);
  return line;
}

}  // namespace

/*--------------------------------------------------------------------*/
NavEditWidget::NavEditWidget(Presentation presentation, QWidget *parent) : QWidget(parent), presentation_(presentation) {
  session_.on_progress = [this](const std::string &text) {
    const QString message = QString::fromStdString(text);
    QMetaObject::invokeMethod(this, [this, message] { emit statusMessage(message, 0); }, Qt::QueuedConnection);
  };
  session_.on_error = [this](const std::string &a, const std::string &b, const std::string &c) {
    const QString qa = QString::fromStdString(a), qb = QString::fromStdString(b), qc = QString::fromStdString(c);
    QMetaObject::invokeMethod(this, [this, qa, qb, qc] { showError(qa, qb, qc); }, Qt::QueuedConnection);
  };
  session_.on_saved = [this](const std::string &path) { saved_path_ = path; };

  canvas_ = new Canvas(session_);
  buildControls();
  buildMenus();
  buildFileListDialog();
  buildTimeSteppingDialog();
  buildModelDialog();
  buildTimeToolDialogs();
  buildOffsetDialog();
  buildDespikeDialog();
  if (presentation_ == Presentation::Panel)
    buildPanel();

  connect(canvas_, &Canvas::modeChanged, this, [this](int mode) {
    mode_group_->button(mode)->setChecked(true);
    syncControls();
  });
  connect(canvas_, &Canvas::viewChanged, this, &NavEditWidget::syncControls);
  connect(canvas_, &Canvas::selectionChanged, this, &NavEditWidget::syncUndo);
  syncControls();
}

NavEditWidget::~NavEditWidget() {
  if (worker_ != nullptr)
    worker_->wait();
}

QPushButton *NavEditWidget::newButton(const QString &text, const QString &name, std::function<void()> action) {
  auto *button = new QPushButton(text);
  button->setObjectName(name);
  button->setFocusPolicy(Qt::NoFocus);
  connect(button, &QPushButton::clicked, this, [action = std::move(action)] { action(); });
  return button;
}

QDialog *NavEditWidget::newDialog(const QString &title, const QString &name) {
  auto *dialog = new QDialog(this);
  dialog->setObjectName(name);
  dialog->setWindowTitle(title);
  return dialog;
}

void NavEditWidget::buildControls() {
  central_ = new QWidget;
  auto *top = new QVBoxLayout(this);
  top->setContentsMargins(0, 0, 0, 0);
  top->addWidget(central_);
  auto *layout = new QVBoxLayout(central_);
  controls_ = new QWidget;
  controls_->setObjectName("controls");
  auto *controls_layout = new QVBoxLayout(controls_);
  controls_layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(controls_);

  /* moving through the data, the modes, and the files */
  auto *row1 = new QHBoxLayout;
  const auto movement = [this, row1](const QString &text, const QString &name, std::function<int()> action) {
    QPushButton *button = newButton(text, name, [this, action] { move(action); });
    row1->addWidget(button);
    need_file_.push_back(button);
  };
  movement("Start", "start", [this] { return session_.start(); });
  movement("Reverse", "reverse", [this] { return session_.step(-session_.data_step_size); });
  movement("Forward", "forward", [this] { return session_.step(session_.data_step_size); });
  movement("End", "end", [this] { return session_.end(); });
  row1->addWidget(separator());
  movement("Show All", "show_all", [this] { return session_.showAll(); });
  pick_zoom_ = new QPushButton("Pick Zoom");
  pick_zoom_->setObjectName("pick_zoom");
  pick_zoom_->setCheckable(true);
  pick_zoom_->setFocusPolicy(Qt::NoFocus);
  connect(pick_zoom_, &QPushButton::toggled, canvas_, &Canvas::setIntervalMode);
  row1->addWidget(pick_zoom_);
  need_file_.push_back(pick_zoom_);
  row1->addWidget(separator());
  mode_group_ = new QButtonGroup(this);
  const struct {
    const char *text, *name;
    Canvas::Mode mode;
  } modes[] = {{"Pick", "mode_pick", Canvas::MODE_PICK},
               {"Select", "mode_select", Canvas::MODE_SELECT},
               {"Deselect", "mode_deselect", Canvas::MODE_DESELECT},
               {"Select All", "mode_selectall", Canvas::MODE_SELECTALL},
               {"Deselect All", "mode_deselectall", Canvas::MODE_DESELECTALL}};
  for (const auto &m : modes) {
    auto *radio = new QRadioButton(m.text);
    radio->setObjectName(m.name);
    radio->setFocusPolicy(Qt::NoFocus);
    radio->setChecked(m.mode == Canvas::MODE_PICK);
    mode_group_->addButton(radio, m.mode);
    row1->addWidget(radio);
  }
  connect(mode_group_, &QButtonGroup::idClicked, this, [this](int mode) {
    canvas_->setMode((Canvas::Mode)mode);
    syncControls();
  });
  row1->addStretch(1);
  QPushButton *next = newButton("Next Buffer", "next_buffer", [this] { nextBuffer(); });
  QPushButton *done = newButton("Done / Next File", "done", [this] { doneOrNextFile(); });
  row1->addWidget(next);
  row1->addWidget(done);
  need_file_.push_back(next);
  need_file_.push_back(done);
  row1->addWidget(newButton("Quit", "quit", [this] { emit closeRequested(); }));
  controls_layout->addLayout(row1);

  /* the edits */
  auto *row2 = new QHBoxLayout;
  const auto editing = [this, row2](const QString &text, const QString &name, std::function<int()> action) {
    QPushButton *button = newButton(text, name, [this, action] { edit(action); });
    row2->addWidget(button);
    need_file_.push_back(button);
  };
  editing("Interpolate", "interpolate", [this] { return session_.interpolate(); });
  editing("Interpolate Repeats", "interpolate_repeats", [this] { return session_.interpolateRepeats(); });
  editing("Revert", "revert", [this] { return session_.revert(); });
  editing("Flag", "flag", [this] { return session_.flag(); });
  editing("Unflag", "unflag", [this] { return session_.unflag(); });
  row2->addWidget(separator());
  editing("Use Solution", "use_solution", [this] { return session_.useDr(); });
  editing("Use Speed-Made-Good", "use_smg", [this] { return session_.useSmg(); });
  editing("Use Course-Made-Good", "use_cmg", [this] { return session_.useCmg(); });
  row2->addStretch(1);
  controls_layout->addLayout(row2);

  /* the plots share the height, with a scroll bar only below their minimum height */
  scroll_ = new QScrollArea;
  scroll_->setObjectName("plots");
  scroll_->setWidgetResizable(true);
  scroll_->setFrameShape(QFrame::NoFrame);
  scroll_->setWidget(canvas_);
  scroll_->setFocusProxy(canvas_);
  layout->addWidget(scroll_, 1);

  auto *row3 = new QHBoxLayout;
  slider_ = new TimeSlider(Qt::Horizontal);
  slider_->setObjectName("time_slider");
  slider_->setFocusPolicy(Qt::NoFocus);
  connect(slider_, &QSlider::valueChanged, this, &NavEditWidget::sliderChanged);
  position_ = new QLabel;
  position_->setObjectName("position");
  row3->addWidget(slider_, 1);
  row3->addWidget(position_);
  layout->addLayout(row3);
  setFocusProxy(canvas_);
}

void NavEditWidget::buildMenus() {
  const auto new_menu = [this](const QString &title) {
    auto *menu = new QMenu(title, this);
    menus_.push_back(menu);
    return menu;
  };

  QMenu *file = new_menu("File");
  open_action_ = file->addAction("Open...", this, &NavEditWidget::openDialog);
  open_action_->setShortcut(QKeySequence::Open);
  filelist_action_ = file->addAction("File Selection List...", this, [this] {
    syncFileList();
    filelist_dialog_->show();
    filelist_dialog_->raise();
  });
  save_action_ = file->addAction("Save", this, &NavEditWidget::save);
  save_action_->setShortcut(QKeySequence::Save);
  file->addSeparator();
  browse_action_ = file->addAction("Browse Only");
  browse_action_->setCheckable(true);
  connect(browse_action_, &QAction::toggled, this, [this](bool on) { setBrowse(on); });

  edit_menu_ = new_menu("Edit");
  undo_action_ = edit_menu_->addAction("Undo", this, [this] { edit([this] { return session_.undo(); }); });
  undo_action_->setShortcut(QKeySequence::Undo);
  redo_action_ = edit_menu_->addAction("Redo", this, [this] { edit([this] { return session_.redo(); }); });
  redo_action_->setShortcut(QKeySequence::Redo);

  QMenu *view = new_menu("View");
  const auto toggle = [this, view](const QString &text, const QString &name, bool Show::*field) {
    QAction *action = view->addAction(text);
    action->setObjectName(name);
    action->setCheckable(true);
    connect(action, &QAction::toggled, this, [this, field](bool on) { showChanged(field, on); });
    show_toggles_.push_back({action, field});
  };
  toggle("Time Interval", "show_tint", &Show::tint);
  toggle("Longitude", "show_lon", &Show::lon);
  toggle("Latitude", "show_lat", &Show::lat);
  toggle("Speed", "show_speed", &Show::speed);
  toggle("Heading", "show_heading", &Show::heading);
  toggle("Sonar Depth", "show_draft", &Show::draft);
  toggle("Roll, Pitch and Heave", "show_vru", &Show::roll);
  view->addSeparator();
  toggle("Original Time Interval", "show_tint_org", &Show::tint_org);
  toggle("Original Longitude", "show_lon_org", &Show::lon_org);
  toggle("Original Latitude", "show_lat_org", &Show::lat_org);
  toggle("Original Speed", "show_speed_org", &Show::speed_org);
  toggle("Original Heading", "show_heading_org", &Show::heading_org);
  toggle("Original Sonar Depth", "show_draft_org", &Show::draft_org);
  view->addSeparator();
  toggle("Longitude Model", "show_lon_dr", &Show::lon_dr);
  toggle("Latitude Model", "show_lat_dr", &Show::lat_dr);
  toggle("Speed-Made-Good", "show_smg", &Show::smg);
  toggle("Course-Made-Good", "show_cmg", &Show::cmg);

  QMenu *controls = new_menu("Controls");
  const auto opens = [this, controls](const QString &text, QDialog **dialog, bool needs_file) {
    QAction *action = controls->addAction(text, this, [this, dialog] {
      syncControls();
      (*dialog)->show();
      (*dialog)->raise();
    });
    if (needs_file)
      need_file_actions_.push_back(action);
  };
  opens("Time Stepping...", &timestep_dialog_, false);
  opens("Nav Modeling...", &model_dialog_, false);
  opens("Time Interpolation...", &time_interpolation_dialog_, true);
  opens("Delete Bad Times...", &delete_bad_dialog_, true);
  opens("Position Offset...", &offset_dialog_, true);
  opens("Despike Sonar Depth...", &despike_dialog_, true);

  QMenu *help = new_menu("Help");
  QAction *about_action = help->addAction("About mbnavedit-qt", this, &NavEditWidget::about);
  about_action->setMenuRole(QAction::AboutRole);
}

void NavEditWidget::buildFileListDialog() {
  filelist_dialog_ = newDialog("Files Available for Editing", "dialog_filelist");
  filelist_dialog_->resize(800, 360);
  auto *layout = new QVBoxLayout(filelist_dialog_);
  filelist_ = new QTableWidget(0, 4);
  filelist_->setObjectName("filelist");
  filelist_->setHorizontalHeaderLabels({"State", "Edits", "File", "Format"});
  filelist_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  filelist_->setSelectionBehavior(QAbstractItemView::SelectRows);
  filelist_->setSelectionMode(QAbstractItemView::SingleSelection);
  filelist_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  layout->addWidget(filelist_, 1);
  auto *row = new QHBoxLayout;
  row->addWidget(newButton("Edit Selected File", "filelist_edit", [this] { editSelectedFile(); }));
  row->addWidget(newButton("Remove Selected File", "filelist_remove", [this] { removeSelectedFile(); }));
  row->addSpacing(16);
  row->addWidget(new QLabel("Output Mode:"));
  filelist_edit_ = new QRadioButton("Output Edits");
  filelist_browse_ = new QRadioButton("Browse Only");
  filelist_edit_->setChecked(true);
  row->addWidget(filelist_edit_);
  row->addWidget(filelist_browse_);
  row->addStretch(1);
  row->addWidget(newButton("Dismiss", "filelist_dismiss", [this] { filelist_dialog_->hide(); }));
  layout->addLayout(row);
  connect(filelist_browse_, &QRadioButton::toggled, this, [this](bool on) { browse_action_->setChecked(on); });

  /* other programs may lock files and write edited navigation meanwhile */
  filelist_timer_ = new QTimer(this);
  filelist_timer_->setInterval(1000);
  connect(filelist_timer_, &QTimer::timeout, this, [this] {
    if (filelist_dialog_->isVisible())
      syncFileList();
  });
  filelist_timer_->start();
}

void NavEditWidget::buildTimeSteppingDialog() {
  timestep_dialog_ = newDialog("Time Stepping", "dialog_timestepping");
  timestep_dialog_->resize(460, 120);
  auto *layout = new QVBoxLayout(timestep_dialog_);
  scales_panel_ = new QWidget;
  auto *form = new QFormLayout(scales_panel_);
  time_span_ = new ScaleControl(1, session_.data_show_max, TIME_LIMIT, 0);
  time_span_->setObjectName("time_span");
  time_step_ = new ScaleControl(1, session_.data_step_max, TIME_LIMIT, 0);
  time_step_->setObjectName("time_step");
  form->addRow("Time Span Shown (seconds):", time_span_);
  form->addRow("Time Step (seconds):", time_step_);
  if (presentation_ == Presentation::Window)
    layout->addWidget(scales_panel_);
  layout->addWidget(newButton("Dismiss", "timestepping_dismiss", [this] { timestep_dialog_->hide(); }), 0, Qt::AlignRight);
  connect(time_span_, &ScaleControl::valueChanged, this, [this](int value) {
    session_.data_show_size = value;
    session_.data_show_max = std::max(session_.data_show_max, value);
    move([this] { return session_.plotAll(); });
  });
  connect(time_step_, &ScaleControl::valueChanged, this, [this](int value) {
    session_.data_step_size = value;
    session_.data_step_max = std::max(session_.data_step_max, value);
  });
}

void NavEditWidget::buildModelDialog() {
  model_dialog_ = newDialog("Nav Modeling", "dialog_modeling");
  model_dialog_->resize(480, 260);
  auto *layout = new QVBoxLayout(model_dialog_);
  auto *form = new QFormLayout;
  model_mode_ = new QComboBox;
  model_mode_->setObjectName("model_mode");
  model_mode_->addItems({"Off", "Gaussian Mean", "Dead Reckoning", "Inversion"});
  form->addRow("Model:", model_mode_);
  model_window_ = new ScaleControl(1, 10000, 1000000, 1);
  model_window_->setObjectName("model_window");
  form->addRow("Gaussian Window (seconds):", model_window_);
  const auto drift = [](const char *name) {
    auto *box = new QSpinBox;
    box->setObjectName(name);
    box->setRange(-1000, 1000);
    box->setKeyboardTracking(false);
    box->setFocusPolicy(Qt::ClickFocus);
    return box;
  };
  drift_lon_ = drift("drift_lon");
  drift_lat_ = drift("drift_lat");
  form->addRow("Longitude Drift (1e-5 degrees/hour):", drift_lon_);
  form->addRow("Latitude Drift (1e-5 degrees/hour):", drift_lat_);
  weight_speed_ = new QLineEdit;
  weight_speed_->setObjectName("weight_speed");
  weight_acceleration_ = new QLineEdit;
  weight_acceleration_->setObjectName("weight_acceleration");
  form->addRow("Speed Weight:", weight_speed_);
  form->addRow("Acceleration Weight:", weight_acceleration_);
  layout->addLayout(form);
  auto *row = new QHBoxLayout;
  row->addStretch(1);
  row->addWidget(newButton("Apply", "model_apply", [this] { applyModel(false); }));
  row->addWidget(newButton("Dismiss", "model_dismiss", [this] { model_dialog_->hide(); }));
  layout->addLayout(row);
  connect(model_mode_, &QComboBox::currentIndexChanged, this, [this] {
    if (!quiet_)
      applyModel(true);
  });
  connect(model_window_, &ScaleControl::valueChanged, this, [this] { applyModel(false); });
  connect(drift_lon_, &QSpinBox::valueChanged, this, [this] {
    if (!quiet_)
      applyModel(false);
  });
  connect(drift_lat_, &QSpinBox::valueChanged, this, [this] {
    if (!quiet_)
      applyModel(false);
  });
}

void NavEditWidget::buildTimeToolDialogs() {
  const auto tool = [this](const QString &title, const QString &name, const QString &text, QPushButton **apply, std::function<int()> action) {
    auto *dialog = newDialog(title, name);
    auto *layout = new QVBoxLayout(dialog);
    auto *label = new QLabel(text);
    label->setWordWrap(true);
    layout->addWidget(label);
    auto *row = new QHBoxLayout;
    row->addStretch(1);
    *apply = newButton("Apply", name + "_apply", [this, action] { edit(action); });
    row->addWidget(*apply);
    row->addWidget(newButton("Dismiss", name + "_dismiss", [dialog] { dialog->hide(); }));
    layout->addLayout(row);
    return dialog;
  };
  time_interpolation_dialog_ =
      tool("Time Interpolation", "dialog_time_interpolation",
           "Replaces repeated or reversed time stamps by times interpolated between the good ones. It can be undone.", &time_interpolation_apply_,
           [this] { return session_.fixTime(); });
  delete_bad_dialog_ = tool("Delete Bad Times", "dialog_delete_bad", "Removes the records with repeated or reversed time stamps. It can be undone.",
                            &delete_bad_apply_, [this] { return session_.deleteBadTime(); });
}

void NavEditWidget::buildOffsetDialog() {
  offset_dialog_ = newDialog("Position Offset", "dialog_offset");
  auto *layout = new QVBoxLayout(offset_dialog_);
  auto *form = new QFormLayout;
  const auto box = [](const char *name, double limit) {
    auto *spin = new QDoubleSpinBox;
    spin->setObjectName(name);
    spin->setDecimals(8);
    spin->setRange(-limit, limit);
    spin->setSingleStep(0.00001);
    spin->setFocusPolicy(Qt::ClickFocus);
    return spin;
  };
  offset_lon_ = box("offset_lon", 360.0);
  offset_lat_ = box("offset_lat", 90.0);
  form->addRow("Longitude Offset (degrees):", offset_lon_);
  form->addRow("Latitude Offset (degrees):", offset_lat_);
  layout->addLayout(form);
  auto *note = new QLabel("Applied to the records in memory and to those read later.");
  layout->addWidget(note);
  auto *row = new QHBoxLayout;
  row->addStretch(1);
  row->addWidget(newButton("Apply", "offset_apply", [this] { edit([this] { return session_.applyOffset(offset_lon_->value(), offset_lat_->value()); }); }));
  row->addWidget(newButton("Dismiss", "offset_dismiss", [this] { offset_dialog_->hide(); }));
  layout->addLayout(row);
}

void NavEditWidget::buildDespikeDialog() {
  despike_dialog_ = newDialog("Despike Sonar Depth", "dialog_despike");
  auto *layout = new QVBoxLayout(despike_dialog_);
  auto *note = new QLabel("Selects the sonar depths in view that are further than the threshold from the running median. "
                          "Check them in the plot, then Interpolate replaces them. A spike must be shorter than half of the window.");
  note->setWordWrap(true);
  layout->addWidget(note);
  auto *form = new QFormLayout;
  despike_window_ = new QDoubleSpinBox;
  despike_window_->setObjectName("despike_window");
  despike_window_->setRange(1.0, 100000.0);
  despike_window_->setDecimals(0);
  despike_window_->setValue(DESPIKE_WINDOW);
  despike_window_->setFocusPolicy(Qt::ClickFocus);
  despike_threshold_ = new QDoubleSpinBox;
  despike_threshold_->setObjectName("despike_threshold");
  despike_threshold_->setRange(0.01, 10000.0);
  despike_threshold_->setDecimals(2);
  despike_threshold_->setValue(DESPIKE_THRESHOLD);
  despike_threshold_->setFocusPolicy(Qt::ClickFocus);
  form->addRow("Window (seconds):", despike_window_);
  form->addRow("Threshold (meters):", despike_threshold_);
  layout->addLayout(form);
  despike_result_ = new QLabel;
  despike_result_->setObjectName("despike_result");
  layout->addWidget(despike_result_);
  auto *row = new QHBoxLayout;
  row->addStretch(1);
  row->addWidget(newButton("Select Spikes", "despike_select", [this] { runDespike(); }));
  row->addWidget(newButton("Interpolate", "despike_interpolate", [this] { interpolateSpikes(); }));
  row->addWidget(newButton("Dismiss", "despike_dismiss", [this] { despike_dialog_->hide(); }));
  layout->addLayout(row);
}

void NavEditWidget::buildPanel() {
  central_->layout()->setContentsMargins(2, 2, 2, 2);
  panel_bar_ = new QToolBar;
  panel_bar_->setObjectName("panel_bar");
  panel_bar_->setToolButtonStyle(Qt::ToolButtonTextOnly);
  qobject_cast<QVBoxLayout *>(central_->layout())->insertWidget(0, panel_bar_);

  const auto put = [this](const char *name, const char *text = nullptr, const char *tip = nullptr) {
    auto *button = findChild<QAbstractButton *>(name);
    if (text != nullptr)
      button->setText(text);
    if (tip != nullptr)
      button->setToolTip(tip);
    if (auto *push = qobject_cast<QPushButton *>(button)) {
      push->setFlat(true);
      push->setFixedWidth(push->fontMetrics().horizontalAdvance(push->text()) + 16);
    }
    panel_bar_->addWidget(button);
  };
  /* first, so that they do not end up behind the overflow of a narrow toolbar */
  const auto popup = [this](const char *text, const char *name, QMenu *menu) {
    auto *button = new QToolButton;
    button->setText(text);
    button->setObjectName(name);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setMenu(menu);
    panel_bar_->addWidget(button);
  };
  auto *scales_menu = new QMenu(this);
  auto *scales_action = new QWidgetAction(scales_menu);
  scales_action->setDefaultWidget(scales_panel_);
  scales_menu->addAction(scales_action);
  popup("Scales", "panel_scales", scales_menu);
  auto *panel_menu = new QMenu(this);
  for (QMenu *menu : menus_)
    panel_menu->addMenu(menu);
  popup("Menu", "panel_menu", panel_menu);
  panel_bar_->addSeparator();
  put("start", "|<", "Start");
  put("reverse", "<", "Reverse");
  put("forward", ">", "Forward");
  put("end", ">|", "End");
  put("next_buffer", ">>", "Next Buffer");
  panel_bar_->addSeparator();
  put("show_all");
  put("pick_zoom");
  panel_bar_->addSeparator();
  put("mode_pick");
  put("mode_select");
  put("mode_deselect");
  put("mode_selectall", "All", "Select All");
  put("mode_deselectall", "None", "Deselect All");
  panel_bar_->addSeparator();
  panel_bar_->addAction(undo_action_);
  panel_bar_->addAction(redo_action_);
  panel_bar_->addSeparator();
  put("interpolate");
  put("revert");
  put("flag");
  put("unflag");
  panel_bar_->addSeparator();

  /* what has no room in the toolbar */
  edit_menu_->addSeparator();
  for (const char *name : {"interpolate_repeats", "use_solution", "use_smg", "use_cmg"}) {
    auto *button = findChild<QPushButton *>(name);
    QAction *action = edit_menu_->addAction(button->text(), button, &QPushButton::click);
    need_file_actions_.push_back(action);
  }

  /* the program around the panel chooses the files and closes it */
  controls_->hide();
  filelist_action_->setVisible(false);
  for (QMenu *menu : menus_)
    for (QAction *action : menu->actions())
      if (action->text() == "Time Stepping...")
        action->setVisible(false);
  setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

/* the dialogs are windows of their own: they go and come with the panel */
void NavEditWidget::hideEvent(QHideEvent *event) {
  if (presentation_ == Presentation::Panel && !event->spontaneous()) {
    hidden_dialogs_.clear();
    for (QDialog *dialog : findChildren<QDialog *>(QString(), Qt::FindDirectChildrenOnly)) {
      if (dialog->isVisible()) {
        hidden_dialogs_.push_back(dialog);
        dialog->hide();
      }
    }
  }
  QWidget::hideEvent(event);
}

void NavEditWidget::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!event->spontaneous()) {
    for (QDialog *dialog : hidden_dialogs_)
      dialog->show();
    hidden_dialogs_.clear();
  }
}

void NavEditWidget::setTimeWindow(const int begin[7], const int end[7]) {
  std::copy(begin, begin + 7, session_.btime_i);
  std::copy(end, end + 7, session_.etime_i);
}

void NavEditWidget::setDisplayName(const QString &name) {
  display_name_ = name;
  syncControls();
}

void NavEditWidget::setShortcutContext(Qt::ShortcutContext context) {
  for (QAction *action : {open_action_, save_action_, undo_action_, redo_action_}) {
    action->setShortcutContext(context);
    /* with a smaller scope than the window the key works only if the action is on this widget */
    if (context != Qt::WindowShortcut)
      addAction(action);
  }
}

bool NavEditWidget::goToTime(double time_d) {
  if (busy() || !session_.isOpen() || session_.goToTime(time_d) != MB_SUCCESS)
    return false;
  canvas_->redraw();
  syncControls();
  return true;
}

/*--------------------------------------------------------------------*/
void NavEditWidget::setWorking(bool working) {
  central_->setEnabled(!working);
  for (QMenu *menu : menus_)
    menu->setEnabled(!working);
  emit busyChanged(working);
}

/* Reading and writing a whole file may take long, so it happens in a thread. Nothing else touches the session meanwhile. */
void NavEditWidget::runJob(const QString &message, std::function<void()> work, std::function<void()> done) {
  canvas_->setIntervalMode(false);
  const QWidget *focus = QApplication::focusWidget();
  job_had_focus_ = focus != nullptr && (focus == this || isAncestorOf(focus));
  emit statusMessage(message, 0);
  worker_ = QThread::create(std::move(work));
  setWorking(true);
  connect(worker_, &QThread::finished, this, [this, done = std::move(done)] {
    worker_->deleteLater();
    worker_ = nullptr;
    setWorking(false);
    if (presentation_ == Presentation::Window || job_had_focus_)
      canvas_->setFocus();
    canvas_->layoutChanged();
    startMbprocess();
    done();
    syncControls();
  });
  worker_->start();
}

void NavEditWidget::startMbprocess() {
  if (!saved_path_.empty() && run_mbprocess_) {
    const QString program = program_directory_.isEmpty() ? QString("mbprocess") : program_directory_ + "/mbprocess";
    if (!QProcess::startDetached(program, {"-I", QString::fromStdString(saved_path_)}))
      showError("Unable to start mbprocess", QString::fromStdString(saved_path_), "");
    else
      emit statusMessage("Edits being applied using mbprocess...", 5000);
  }
  /* once for each save: closing after a Save with nothing new writes nothing */
  if (!saved_path_.empty() && !save_reported_)
    emit editsSaved(QString::fromStdString(saved_path_));
  if (!saved_path_.empty())
    save_reported_ = false;
  saved_path_.clear();
}

void NavEditWidget::showError(const QString &a, const QString &b, const QString &c) { QMessageBox::warning(this, display_name_, a + "\n" + b + "\n" + c); }

void NavEditWidget::addInput(const std::string &path, int format) {
  std::string message;
  if (!parse_input(path, format, files_, message))
    showError(QString::fromStdString(message), "", "");
  syncFileList();
}

void NavEditWidget::setBrowse(bool browse) {
  browse_ = browse;
  browse_action_->blockSignals(true);
  browse_action_->setChecked(browse);
  browse_action_->blockSignals(false);
  filelist_browse_->blockSignals(true);
  (browse ? filelist_browse_ : filelist_edit_)->setChecked(true);
  filelist_browse_->blockSignals(false);
}

void NavEditWidget::loadFirstFile() {
  if (!files_.empty())
    loadFile(0);
}

bool NavEditWidget::openFile(const QString &path, int format, SavedEdits edits) {
  if (busy())
    return false;
  if (session_.isOpen() && path == QString::fromUtf8(session_.file()))
    return true;
  const std::string name = path.toStdString();
  afterCurrentFileClosed([this, name, format, edits] {
    int index = -1;
    for (int i = 0; i < (int)files_.size(); i++)
      if (files_[i].path == name)
        index = i;
    if (index < 0) {
      const size_t before = files_.size();
      std::string message;
      if (!parse_input(name, format, files_, message) || files_.size() == before) {
        showError(QString::fromStdString(message), QString::fromStdString(name), "");
        return;
      }
      index = (int)before;
      syncFileList();
    }
    loadFile(index, edits == SavedEdits::Ask, edits == SavedEdits::Apply);
  });
  return true;
}

void NavEditWidget::openDialog() {
  const QString path = QFileDialog::getOpenFileName(this, "Open Swath File or Datalist");
  if (path.isEmpty())
    return;
  afterCurrentFileClosed([this, path] {
    const size_t before = files_.size();
    std::string message;
    bool ok = parse_input(path.toStdString(), 0, files_, message);
    if (!ok) {
      /* the name does not tell the format */
      const int format =
          QInputDialog::getInt(this, "MBIO Format ID", file_name(path.toStdString()) + ": MBIO Format ID:", 0, -1, 9999, 1, &ok);
      if (ok)
        ok = parse_input(path.toStdString(), format, files_, message);
      if (!ok && !message.empty())
        showError(QString::fromStdString(message), "", "");
    }
    syncFileList();
    if (files_.size() > before)
      loadFile((int)before);
  });
}

void NavEditWidget::loadFile(int index, bool ask_saved_edits, bool use_saved_edits) {
  if (index < 0 || index >= (int)files_.size())
    return;
  const std::string path = files_[index].path;
  const int format = files_[index].format;

  /* a lock of this user on this computer may be left from a crash */
  std::string description;
  bool mine = false;
  if (Session::lockInfo(path, description, mine) && mine) {
    const bool open_anyway =
        lock_choice_ ? lock_choice_(QString::fromStdString(path), QString::fromStdString(description)) == LockChoice::OpenAnyway
                     : QMessageBox::question(this, display_name_,
                                             QString("%1 is locked by %2.\nIf that program is no longer running, the lock was left by a crash.\nOpen it anyway?")
                                                 .arg(file_name(path), QString::fromStdString(description))) == QMessageBox::Yes;
    if (open_anyway)
      Session::removeLock(path);
  }

  bool use_previous = use_saved_edits && Session::hasSavedEdits(path);
  if (ask_saved_edits && Session::hasSavedEdits(path))
    use_previous = QMessageBox::question(this, display_name_,
                                         QString("%1 has edited navigation saved.\nDo you want to start from those edits?").arg(file_name(path))) ==
                   QMessageBox::Yes;

  current_file_ = index;
  const OutputMode mode = browse_ ? OUTPUT_BROWSE : OUTPUT_EDIT;
  auto status = std::make_shared<int>(MB_FAILURE);
  runJob("Loading " + file_name(path) + "...", [this, path, format, use_previous, mode, status] { *status = session_.open(path, format, use_previous, mode); },
         [this, status] {
           if (*status != MB_SUCCESS) {
             current_file_ = -1;
             QApplication::beep();
           }
           else {
             emit statusMessage(QString("%1 records loaded").arg(session_.nbuff()), 5000);
           }
           canvas_->layoutChanged();
           syncFileList();
         });
}

/* before another file is loaded */
void NavEditWidget::afterCurrentFileClosed(std::function<void()> next) {
  if (!session_.isOpen()) {
    next();
    return;
  }
  if (session_.unsaved()) {
    const auto answer = QMessageBox::question(this, display_name_, QString("Save changes to the navigation of %1?").arg(file_name(session_.file())),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Cancel)
      return;
    if (answer == QMessageBox::Save) {
      auto status = std::make_shared<int>(MB_FAILURE);
      runJob("Saving edits...", [this, status] { *status = session_.closeSave(); }, [this, status, next = std::move(next)] {
        canvas_->layoutChanged();
        if (*status == MB_SUCCESS) {
          current_file_ = -1;
          next();
        }
      });
      return;
    }
    session_.closeDiscard();
  }
  else {
    session_.closeSave();
  }
  current_file_ = -1;
  canvas_->layoutChanged();
  next();
}

bool NavEditWidget::save() {
  if (busy() || !session_.canSave())
    return false;
  const bool saved = session_.save() == MB_SUCCESS;
  if (saved) {
    emit statusMessage("Edits saved", 3000);
    emit editsSaved(QString::fromUtf8(session_.file()));
    save_reported_ = true;
  }
  startMbprocess();
  syncControls();
  syncFileList();
  return saved;
}

bool NavEditWidget::prepareToClose(std::function<void()> closed_later) {
  if (busy())
    return false;
  if (session_.isOpen()) {
    if (session_.unsaved()) {
      const auto answer = QMessageBox::question(this, display_name_, QString("Save changes to the navigation of %1?").arg(file_name(session_.file())),
                                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
      if (answer == QMessageBox::Cancel)
        return false;
      if (answer == QMessageBox::Save) {
        /* close again when the edits are written; if that failed the editor stays with the error */
        auto status = std::make_shared<int>(MB_FAILURE);
        runJob("Saving edits...", [this, status] { *status = session_.closeSave(); }, [this, status, closed_later = std::move(closed_later)] {
          current_file_ = -1;
          canvas_->layoutChanged();
          if (*status == MB_SUCCESS && closed_later)
            closed_later();
        });
        return false;
      }
      session_.closeDiscard();
    }
    else {
      session_.closeSave();
    }
  }
  startMbprocess();
  syncControls();
  return true;
}

void NavEditWidget::doneOrNextFile() {
  auto quit = std::make_shared<bool>(false);
  runJob("Saving edits...", [this, quit] { session_.done(quit.get()); }, [this, quit] {
    canvas_->layoutChanged();
    syncFileList();
    if (*quit) {
      emit closeRequested();
    }
    else if (current_file_ >= 0 && current_file_ < (int)files_.size() - 1) {
      const int next = current_file_ + 1;
      QTimer::singleShot(0, this, [this, next] { loadFile(next); });
    }
    else {
      current_file_ = -1;
    }
  });
}

void NavEditWidget::nextBuffer() {
  auto quit = std::make_shared<bool>(false);
  runJob("Reading the next buffer...", [this, quit] { session_.nextBuffer(quit.get()); }, [this, quit] {
    canvas_->layoutChanged();
    if (*quit)
      emit closeRequested();
  });
}

void NavEditWidget::editSelectedFile() {
  const int row = filelist_->currentRow();
  if (row < 0 || row == current_file_ || busy())
    return;
  afterCurrentFileClosed([this, row] { loadFile(row); });
}

void NavEditWidget::removeSelectedFile() {
  const int row = filelist_->currentRow();
  if (row < 0 || row == current_file_)
    return;
  files_.erase(files_.begin() + row);
  if (current_file_ > row)
    current_file_--;
  syncFileList();
}

void NavEditWidget::about() {
  QMessageBox::about(this, "About mbnavedit-qt",
                     QString("<b>mbnavedit-qt</b><br>Interactive Navigation Editor<br>One component of the MB-System open source software "
                             "for processing and display of swath sonar data.<br><br>MB-System Release %1, %2<br><br>Created by David W. Caress "
                             "and Dale N. Chayes.<br>Qt6 port by Christian dos Santos Ferreira.")
                         .arg(MB_VERSION, MB_VERSION_DATE));
}

/*--------------------------------------------------------------------*/
void NavEditWidget::syncUndo() {
  const bool panel = presentation_ == Presentation::Panel;
  undo_action_->setEnabled(session_.canUndo());
  const QString undo_text = session_.canUndo() ? "Undo " + QString::fromStdString(session_.undoLabel()) : "Undo";
  undo_action_->setText(panel ? "Undo" : undo_text);
  undo_action_->setToolTip(undo_text);
  redo_action_->setEnabled(session_.canRedo());
  const QString redo_text = session_.canRedo() ? "Redo " + QString::fromStdString(session_.redoLabel()) : "Redo";
  redo_action_->setText(panel ? "Redo" : redo_text);
  redo_action_->setToolTip(redo_text);
}

void NavEditWidget::syncControls() {
  if (busy())
    return;
  const bool open = session_.isOpen() && session_.nbuff() > 0;
  for (QWidget *widget : need_file_)
    widget->setEnabled(open);
  for (QAction *action : need_file_actions_)
    action->setEnabled(open);
  syncUndo();
  const bool panel = presentation_ == Presentation::Panel;
  save_action_->setEnabled(open && session_.canSave() && session_.unsaved());
  time_interpolation_apply_->setEnabled(open && session_.timestampProblem());
  delete_bad_apply_->setEnabled(open && session_.timestampProblem());

  pick_zoom_->blockSignals(true);
  pick_zoom_->setChecked(canvas_->intervalMode());
  pick_zoom_->blockSignals(false);

  quiet_ = true;
  slider_->setEnabled(open);
  slider_->setRange(0, std::max(session_.nbuff() - 1, 0));
  slider_->setValue(std::max(session_.current(), 0));
  time_span_->setValueQuietly(std::max(session_.data_show_size, 1));
  time_step_->setValueQuietly(std::max(session_.data_step_size, 1));
  model_mode_->setCurrentIndex((int)session_.model_mode);
  model_window_->setValueQuietly(session_.mean_time_window);
  drift_lon_->setValue(session_.drift_lon);
  drift_lat_->setValue(session_.drift_lat);
  weight_speed_->setText(QString::number(session_.weight_speed, 'f', 2));
  weight_acceleration_->setText(QString::number(session_.weight_acceleration, 'f', 2));
  offset_lon_->setValue(session_.offsetLon());
  offset_lat_->setValue(session_.offsetLat());
  for (const ShowToggle &t : show_toggles_) {
    t.action->blockSignals(true);
    t.action->setChecked(session_.show.*t.field);
    t.action->blockSignals(false);
  }
  quiet_ = false;

  const QString position_format = panel ? "%1-%2 of %3" : "Records %1 to %2 of %3";
  position_->setText(open ? position_format.arg(session_.current() + 1).arg(session_.current() + session_.nplot()).arg(session_.nbuff()) : QString("No file open"));

  const QString title = open ? QString("%1[*] - %2").arg(file_name(session_.file()), display_name_) : display_name_;
  if (title != title_) {
    title_ = title;
    emit titleChanged(title_);
  }
  const bool unsaved = open && session_.unsaved();
  if (unsaved != unsaved_) {
    unsaved_ = unsaved;
    emit unsavedChanged(unsaved_);
  }
  if (unsaved)
    save_reported_ = false;
  reportState();
}

/* tells the surrounding program what changed since the last time */
void NavEditWidget::reportState() {
  const QString path = session_.isOpen() ? QString::fromUtf8(session_.file()) : QString();
  if (path != open_path_) {
    if (!open_path_.isEmpty())
      emit fileClosed(open_path_);
    open_path_ = path;
    view_first_ = view_last_ = reported_changes_ = -1;
    if (!path.isEmpty())
      emit fileOpened(path, session_.format());
  }
  if (path.isEmpty() || session_.nbuff() == 0 || session_.nplot() <= 0)
    return;
  const Record &first = session_.records()[session_.current()];
  const Record &last = session_.records()[std::min(session_.current() + session_.nplot() - 1, session_.nbuff() - 1)];
  if (first.record != view_first_ || last.record != view_last_) {
    view_first_ = first.record;
    view_last_ = last.record;
    emit viewChanged(path, view_first_, view_last_, first.time_d, last.time_d);
  }
  /* counting the changes takes time when the buffer is large */
  if (isSignalConnected(QMetaMethod::fromSignal(&NavEditWidget::editsChanged))) {
    const long changes = session_.changes();
    if (changes != reported_changes_) {
      reported_changes_ = changes;
      emit editsChanged(path, changes);
    }
  }
}

void NavEditWidget::syncFileList() {
  const int selected = filelist_->currentRow();
  filelist_->setRowCount((int)files_.size());
  for (int i = 0; i < (int)files_.size(); i++) {
    std::string description;
    bool mine = false;
    QString state;
    if (i == current_file_ && session_.isOpen())
      state = "<loaded>";
    else if (Session::lockInfo(files_[i].path, description, mine))
      state = "<Locked>";
    const QStringList cells = {state, Session::hasSavedEdits(files_[i].path) ? "<nve>" : "", QString::fromStdString(files_[i].path),
                               QString::number(files_[i].format)};
    for (int c = 0; c < 4; c++) {
      auto *item = filelist_->item(i, c);
      if (item == nullptr) {
        item = new QTableWidgetItem;
        filelist_->setItem(i, c, item);
      }
      item->setText(cells[c]);
    }
  }
  if (selected >= 0 && selected < filelist_->rowCount())
    filelist_->selectRow(selected);
}

void NavEditWidget::edit(std::function<int()> action) {
  if (busy() || !session_.isOpen())
    return;
  action();
  canvas_->redraw();
  syncControls();
}

void NavEditWidget::move(std::function<int()> action) {
  if (busy() || !session_.isOpen()) {
    syncControls();
    return;
  }
  action();
  canvas_->redraw();
  syncControls();
}

void NavEditWidget::applyModel(bool mode_changed) {
  if (busy())
    return;
  session_.model_mode = (ModelMode)model_mode_->currentIndex();
  if (mode_changed && session_.model_mode != MODEL_OFF)
    session_.show.lon_dr = session_.show.lat_dr = true;
  session_.mean_time_window = model_window_->value();
  session_.drift_lon = drift_lon_->value();
  session_.drift_lat = drift_lat_->value();
  bool ok = false;
  const double speed = weight_speed_->text().toDouble(&ok);
  if (ok && speed >= 0.0)
    session_.weight_speed = speed;
  const double acceleration = weight_acceleration_->text().toDouble(&ok);
  if (ok && acceleration >= 0.0)
    session_.weight_acceleration = acceleration;
  if (session_.isOpen() && session_.nbuff() > 0) {
    session_.getModel();
    session_.plotAll();
  }
  canvas_->redraw();
  syncControls();
}

void NavEditWidget::showChanged(bool Show::*field, bool on) {
  session_.show.*field = on;
  if (field == &Show::roll)
    session_.show.pitch = session_.show.heave = on;
  session_.recountPlots();
  canvas_->layoutChanged();
  syncControls();
}

void NavEditWidget::sliderChanged(int value) {
  if (quiet_ || busy() || !session_.isOpen() || value >= session_.nbuff())
    return;
  session_.goToTime(session_.records()[value].time_d);
  canvas_->redraw();
  syncControls();
}

void NavEditWidget::runDespike() {
  if (busy() || !session_.isOpen())
    return;
  if (!session_.show.draft) {
    despike_result_->setText("Show the Sonar Depth plot first.");
    return;
  }
  int count = 0;
  double largest = 0.0;
  session_.beginSelection();
  session_.despikeDepth(despike_window_->value(), despike_threshold_->value(), &count, &largest);
  session_.finishSelection();
  despike_found_ = count;
  canvas_->redraw();
  despike_result_->setText(QString("%1. Largest distance from the median in view: %2 m.")
                               .arg(count == 1 ? QString("1 record selected") : QString("%1 records selected").arg(count))
                               .arg(largest, 0, 'f', 2));
  syncControls();
}

/* The original line and the range it keeps would still show the spikes as if nothing had changed. */
void NavEditWidget::interpolateSpikes() {
  if (busy() || !session_.isOpen())
    return;
  const int found = despike_found_;
  despike_found_ = 0;
  if (found <= 0) {
    despike_result_->setText("Nothing selected: use Select Spikes first.");
    return;
  }
  session_.show.draft_org = false;
  edit([this] { return session_.interpolate(); });
  despike_result_->setText(QString("%1 interpolated. The original line is now hidden: View > Original Sonar Depth shows it.")
                               .arg(found == 1 ? QString("1 record") : QString("%1 records").arg(found)));
}

}  // namespace mbne
