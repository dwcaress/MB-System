/*--------------------------------------------------------------------
 *    The MB-system:  mbe_editor.cc  2/11/2026
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

#include "mbe_editor.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>

#include <QAbstractItemView>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QSettings>
#include <QSplitter>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QThread>
#include <QTimeZone>
#include <QTimer>
#include <QVBoxLayout>

#include "mb_define.h"
#include "mb_status.h"

namespace mbe {

namespace {

QString file_name(const std::string &path) { return QFileInfo(QString::fromStdString(path)).fileName(); }

QAction *checkable(QMenu *menu, QActionGroup *group, const QString &text, int id, bool checked = false) {
  QAction *action = menu->addAction(text);
  action->setCheckable(true);
  action->setChecked(checked);
  action->setData(id);
  group->addAction(action);
  return action;
}

}  // namespace

/*--------------------------------------------------------------------*/
EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent) {
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
  panel_ = new TimeSeriesPanel(session_);
  map_ = new TrackMap(session_);
  map_->setObjectName("track_map");
  map_->hide();
  buildControls();
  buildMenus();
  buildGotoDialog();
  buildBufferDialog();
  buildAnnotationDialog();
  buildFiltersDialog();
  buildFileListDialog();
  buildReasonsDialog();
  buildHistoryDialog();
  buildMbcleanDialog();

  connect(canvas_, &Canvas::suspectKey, this, &EditorWidget::nextSuspect);
  connect(canvas_, &Canvas::edited, this, &EditorWidget::afterEdit);
  connect(canvas_, &Canvas::moved, this, &EditorWidget::afterMove);
  connect(canvas_, &Canvas::modeKey, this, [this](int mode) { mode_group_->button(mode)->setChecked(true); });
  connect(canvas_, &Canvas::viewKey, this, [this](int view) { setViewMode(view); });
  connect(canvas_, &Canvas::viewChanged, this, &EditorWidget::syncControls);
  connect(canvas_, &Canvas::contextMenuAt, this, &EditorWidget::pingContextMenu);
  connect(panel_, &TimeSeriesPanel::pingClicked, this, &EditorWidget::showPing);
  connect(map_, &TrackMap::pingClicked, this, &EditorWidget::showPing);

  syncControls();
}

EditorWidget::~EditorWidget() {
  if (worker_ != nullptr) {
    worker_->wait();
  }
  stopBackground();
  if (mbclean_process_ != nullptr) {
    mbclean_process_->kill();
    mbclean_process_->waitForFinished(2000);
  }
}

/*--------------------------------------------------------------------*/
QMenu *EditorWidget::newMenu(const QString &title) {
  auto *menu = new QMenu(title, this);
  menus_.push_back(menu);
  return menu;
}

void EditorWidget::setWorking(bool working) {
  central_->setEnabled(!working);
  for (QMenu *menu : menus_)
    menu->setEnabled(!working);
  emit busyChanged(working);
}

void EditorWidget::setDisplayName(const QString &name) {
  display_name_ = name;
  syncControls();
}

void EditorWidget::setShortcutContext(Qt::ShortcutContext context) {
  for (QAction *action : {open_action_, save_action_, undo_action_, redo_action_})
    action->setShortcutContext(context);
}

void EditorWidget::buildControls() {
  /* the controls are disabled during a job, but not the dialogs, which are children of the editor */
  central_ = new QWidget;
  auto *outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->addWidget(central_);
  auto *layout = new QVBoxLayout(central_);

  /* edit mode */
  auto *mode_row = new QHBoxLayout;
  mode_row->addWidget(new QLabel("Mode:"));
  mode_group_ = new QButtonGroup(this);
  const struct {
    const char *name;
    Canvas::Mode mode;
  } mode_buttons[] = {{"Toggle", Canvas::MODE_TOGGLE}, {"Pick", Canvas::MODE_PICK},   {"Erase", Canvas::MODE_ERASE}, {"Restore", Canvas::MODE_RESTORE},
                      {"Grab", Canvas::MODE_GRAB},     {"Lasso", Canvas::MODE_LASSO}, {"Info", Canvas::MODE_INFO}};
  for (const auto &m : mode_buttons) {
    auto *button = new QRadioButton(m.name);
    button->setObjectName(QString("mode_%1").arg(QString(m.name).toLower()));
    button->setFocusPolicy(Qt::NoFocus);
    mode_group_->addButton(button, m.mode);
    mode_row->addWidget(button);
  }
  mode_group_->button(0)->setChecked(true);
  connect(mode_group_, &QButtonGroup::idClicked, this, &EditorWidget::setModeFromUi);
  mode_row->addStretch(1);
  layout->addLayout(mode_row);

  /* scales */
  width_ = new ScaleControl(1, 20000, mbe::MAX_PLOT_WIDTH, 0);
  width_->setObjectName("scale_width");
  exagger_ = new ScaleControl(1, 2000, 10000000, 2);
  exagger_->setObjectName("scale_exaggeration");
  pings_ = new ScaleControl(1, 20, mbe::MAX_PLOT_PINGS, 0);
  pings_->setObjectName("scale_pings");
  step_ = new ScaleControl(1, 20, mbe::MAX_PLOT_PINGS, 0);
  step_->setObjectName("scale_step");
  auto *scales = new QGridLayout;
  scales->addWidget(new QLabel("Acrosstrack Width (m):"), 0, 0);
  scales->addWidget(width_, 0, 1);
  scales->addWidget(new QLabel("Vertical Exaggeration:"), 0, 2);
  scales->addWidget(exagger_, 0, 3);
  scales->addWidget(new QLabel("Pings Shown:"), 1, 0);
  scales->addWidget(pings_, 1, 1);
  scales->addWidget(new QLabel("Pings to Step:"), 1, 2);
  scales->addWidget(step_, 1, 3);
  layout->addLayout(scales);
  connect(width_, &ScaleControl::valueChanged, this, [this](int v) {
    session_.settings.plot_width = v;
    applySettings();
  });
  connect(exagger_, &ScaleControl::valueChanged, this, [this](int v) {
    session_.settings.exagger = v;
    applySettings();
  });
  connect(pings_, &ScaleControl::valueChanged, this, &EditorWidget::pingsChanged);
  connect(step_, &ScaleControl::valueChanged, this, &EditorWidget::stepChanged);

  /* plot */
  splitter_ = new QSplitter(Qt::Horizontal);
  splitter_->addWidget(panel_);
  splitter_->addWidget(canvas_);
  splitter_->addWidget(map_);
  splitter_->setStretchFactor(0, 0);
  splitter_->setStretchFactor(1, 1);
  splitter_->setStretchFactor(2, 0);
  splitter_->setCollapsible(2, false);
  splitter_->setSizes({240, 1100});
  panel_->hide();
  layout->addWidget(splitter_, 1);

  /* buttons */
  auto *buttons = new QHBoxLayout;
  auto button = [&](QPushButton *&member, const char *text, const char *name) {
    member = new QPushButton(text);
    member->setObjectName(name);
    member->setFocusPolicy(Qt::NoFocus);
    buttons->addWidget(member);
  };
  button(start_, "Start", "button_start");
  button(reverse_, "Reverse", "button_reverse");
  button(forward_, "Forward", "button_forward");
  button(end_, "End", "button_end");
  buttons->addSpacing(16);
  button(next_, "Next Buffer", "button_next");
  button(done_, "Done", "button_done");
  buttons->addSpacing(16);
  button(flag_view_, "Flag View", "button_flag_view");
  button(unflag_view_, "Unflag View", "button_unflag_view");
  button(unflag_forward_, "Unflag Forward", "button_unflag_forward");
  buttons->addStretch(1);
  button(quit_, "Quit", "button_quit");
  layout->addLayout(buttons);

  auto move = [this](int nsteps) {
    if (session_.step(nsteps) != MB_SUCCESS)
      QApplication::beep();
    canvas_->redraw();
    afterMove();
  };
  connect(start_, &QPushButton::clicked, this, [this, move] { move(-session_.current()); });
  connect(end_, &QPushButton::clicked, this, [this, move] { move(session_.nbuff() - session_.current() - 1); });
  connect(reverse_, &QPushButton::clicked, this, [this, move] { move(-canvas_->stepSize); });
  connect(forward_, &QPushButton::clicked, this, [this, move] { move(canvas_->stepSize); });
  connect(next_, &QPushButton::clicked, this, &EditorWidget::nextBuffer);
  connect(done_, &QPushButton::clicked, this, &EditorWidget::doneOrNextFile);
  connect(flag_view_, &QPushButton::clicked, this, [this] { flagAction([this] { return session_.flagView(); }); });
  connect(unflag_view_, &QPushButton::clicked, this, [this] { flagAction([this] { return session_.unflagView(); }); });
  connect(unflag_forward_, &QPushButton::clicked, this, [this] { flagAction([this] { return session_.unflagForward(); }); });
  connect(quit_, &QPushButton::clicked, this, &EditorWidget::closeRequested);

  /* position in the buffer */
  auto *slider_row = new QHBoxLayout;
  slider_row->addWidget(new QLabel("Ping:"));
  slider_ = new PingSlider(Qt::Horizontal);
  slider_->setObjectName("ping_slider");
  slider_->setFocusPolicy(Qt::NoFocus);
  strip_ = new SuspectStrip;
  strip_->setObjectName("suspect_strip");
  connect(strip_, &SuspectStrip::pingClicked, this, &EditorWidget::showPing);
  auto *slider_column = new QVBoxLayout;
  slider_column->setSpacing(1);
  slider_column->addWidget(strip_);
  slider_column->addWidget(slider_);
  slider_row->addLayout(slider_column, 1);
  position_ = new QLabel;
  position_->setObjectName("ping_position");
  slider_row->addWidget(position_);
  layout->addLayout(slider_row);
  connect(slider_, &QSlider::valueChanged, this, &EditorWidget::sliderMoved);
}

void EditorWidget::buildMenus() {
  QMenu *file = newMenu("File");
  open_action_ = file->addAction("Open...", this, &EditorWidget::openDialog);
  open_action_->setShortcut(QKeySequence::Open);
  filelist_action_ = file->addAction("File Selection List...", this, [this] {
    syncFileList();
    filelist_dialog_->show();
    filelist_dialog_->raise();
  });
  file->addSeparator();
  browse_action_ = file->addAction("Browse Only");
  browse_action_->setCheckable(true);
  connect(browse_action_, &QAction::toggled, this, &EditorWidget::setBrowse);
  file->addSeparator();
  save_action_ = file->addAction("Save", this, &EditorWidget::saveNow);
  save_action_->setShortcut(QKeySequence::Save);

  QMenu *edit = newMenu("Edit");
  undo_action_ = edit->addAction("Undo", this, [this] {
    session_.undo();
    canvas_->redraw();
    syncControls();
  });
  undo_action_->setShortcut(QKeySequence::Undo);
  redo_action_ = edit->addAction("Redo", this, [this] {
    session_.redo();
    canvas_->redraw();
    syncControls();
  });
  redo_action_->setShortcut(QKeySequence::Redo);
  edit->addSeparator();
  edit->addAction("Flags by Reason...", this, [this] {
    updateReasonCount();
    reasons_dialog_->show();
    reasons_dialog_->raise();
  });

  QMenu *view = newMenu("View");
  view_group_ = new QActionGroup(this);
  checkable(view, view_group_, "Waterfall View", mbe::VIEW_WATERFALL, true);
  checkable(view, view_group_, "Alongtrack View", mbe::VIEW_ALONGTRACK);
  checkable(view, view_group_, "Acrosstrack View", mbe::VIEW_ACROSSTRACK);
  connect(view_group_, &QActionGroup::triggered, this, [this](QAction *a) { setViewMode(a->data().toInt()); });
  view->addSeparator();
  flagged_soundings_ = view->addAction("Show Flagged Soundings");
  flagged_soundings_->setCheckable(true);
  flagged_soundings_->setChecked(true);
  connect(flagged_soundings_, &QAction::toggled, this, [this](bool on) {
    session_.settings.show_flaggedsoundings = on;
    applySettings();
  });
  flagged_profile_ = view->addAction("Show Flagged Profile");
  flagged_profile_->setCheckable(true);
  connect(flagged_profile_, &QAction::toggled, this, [this](bool on) {
    session_.settings.show_flaggedprofiles = on;
    applySettings();
  });
  view->addSeparator();
  show_group_ = new QActionGroup(this);
  checkable(view, show_group_, "Show Flag States", mbe::SHOW_FLAG, true);
  checkable(view, show_group_, "Show Bottom Detect Algorithms", mbe::SHOW_DETECT);
  checkable(view, show_group_, "Show Source Pulse Types", mbe::SHOW_PULSE);
  show_amplitude_ = checkable(view, show_group_, "Show Amplitude", mbe::SHOW_AMPLITUDE);
  checkable(view, show_group_, "Show Beam Angle", mbe::SHOW_ANGLE);
  show_residual_ = checkable(view, show_group_, "Show Difference from Reference", mbe::SHOW_RESIDUAL);
  show_residual_->setEnabled(false);
  connect(show_group_, &QActionGroup::triggered, this, [this](QAction *a) { setShowMode(a->data().toInt()); });
  view->addSeparator();
  plot_group_ = new QActionGroup(this);
  checkable(view, plot_group_, "Wide Bathymetry Profiles", mbe::PLOT_WIDE);
  checkable(view, plot_group_, "Print Time Stamps", mbe::PLOT_TIME, true);
  checkable(view, plot_group_, "Plot Ping Interval", mbe::PLOT_INTERVAL);
  checkable(view, plot_group_, "Plot Longitude", mbe::PLOT_LON);
  checkable(view, plot_group_, "Plot Latitude", mbe::PLOT_LAT);
  checkable(view, plot_group_, "Plot Heading", mbe::PLOT_HEADING);
  checkable(view, plot_group_, "Plot Speed", mbe::PLOT_SPEED);
  checkable(view, plot_group_, "Plot Center Beam Depth", mbe::PLOT_DEPTH);
  checkable(view, plot_group_, "Plot Sonar Altitude", mbe::PLOT_ALTITUDE);
  checkable(view, plot_group_, "Plot Sonar Depth", mbe::PLOT_SENSORDEPTH);
  checkable(view, plot_group_, "Plot Roll", mbe::PLOT_ROLL);
  checkable(view, plot_group_, "Plot Pitch", mbe::PLOT_PITCH);
  checkable(view, plot_group_, "Plot Heave", mbe::PLOT_HEAVE);
  connect(plot_group_, &QActionGroup::triggered, this, [this](QAction *a) { setPlotMode(a->data().toInt()); });

  view->addSeparator();
  show_map_ = view->addAction("Show Track Map");
  show_map_->setCheckable(true);
  connect(show_map_, &QAction::toggled, this, [this](bool on) {
    /* the splitter would keep it at no width: it takes its room from the soundings */
    QList<int> sizes = splitter_->sizes();
    map_->setVisible(on);
    if (on) {
      const int room = map_->sizeHint().width();
      sizes[1] = std::max(sizes[1] - room, 200);
      sizes[2] = room;
      splitter_->setSizes(sizes);
    }
    QTimer::singleShot(0, canvas_, [this] { canvas_->layoutChanged(); });
  });

  QMenu *controls = newMenu("Controls");
  auto show_dialog = [this](QDialog *&dialog) {
    dialog->show();
    dialog->raise();
  };
  controls->addAction("Go To Specified Time...", this, [this, show_dialog] {
    if (session_.isOpen() && session_.nbuff() > 0) {
      const int *t = session_.pings()[session_.current()].time_i;
      for (int i = 0; i < 6; i++)
        goto_fields_[i]->setValue(t[i]);
    }
    show_dialog(goto_dialog_);
  });
  controls->addAction("Buffer Controls...", this, [this, show_dialog] { show_dialog(buffer_dialog_); });
  controls->addAction("Annotation...", this, [this, show_dialog] {
    x_interval_->setValueQuietly(session_.settings.x_interval);
    y_interval_->setValueQuietly(session_.settings.y_interval);
    show_dialog(annotation_dialog_);
  });
  controls->addAction("Filters...", this, [this, show_dialog] {
    filtersToDialog();
    show_dialog(filters_dialog_);
  });
  controls->addSeparator();
  reverse_keys_ = controls->addAction("Reverse Right/Left Key Macros");
  reverse_keys_->setCheckable(true);
  connect(reverse_keys_, &QAction::toggled, this, [this](bool on) { canvas_->reverseKeys = on; });
  reverse_mouse_ = controls->addAction("Reverse Mouse Buttons");
  reverse_mouse_->setCheckable(true);
  connect(reverse_mouse_, &QAction::toggled, this, [this](bool on) { canvas_->reverseMouse = on; });

  view->addSeparator();
  view->addAction("Reset Zoom", this, [this] {
    session_.resetView();
    canvas_->redraw();
    syncControls();
  });

  QMenu *tools = newMenu("Tools");
  tools->addAction("Next Suspect Ping", this, [this] { nextSuspect(1); });
  tools->addAction("Previous Suspect Ping", this, [this] { nextSuspect(-1); });
  tools->addSeparator();
  tools->addAction("Preview mbclean...", this, [this] {
    mbclean_dialog_->show();
    mbclean_dialog_->raise();
  });
  tools->addSeparator();
  tools->addAction("Load Reference Surface...", this, [this] {
    const QString path = QFileDialog::getOpenFileName(this, "Load Reference Surface", "", "Grids (*.grd *.nc);;All Files (*)");
    if (!path.isEmpty())
      loadReference(path);
  });
  clear_reference_ = tools->addAction("Clear Reference Surface", this, &EditorWidget::clearReference);
  tools->addSeparator();
  history_action_ = tools->addAction("Edit History...", this, &EditorWidget::showHistory);
  /* mbwcdview is optional: the item is there only if the program is */
  water_column_ = tools->addAction("Water Column at Center Ping", this, [this] {
    openWaterColumn(session_.current() + session_.nplot() / 2);
  });
  water_column_->setVisible(!findProgram("mbwcdview").isEmpty());
  mbeditviz_ = tools->addAction("Save and Open in mbeditviz", this, &EditorWidget::openMbeditviz);

  QMenu *help = newMenu("Help");
  QAction *about_action = help->addAction("About mbedit-qt", this, &EditorWidget::about);
  about_action->setMenuRole(QAction::AboutRole);
}

void EditorWidget::buildGotoDialog() {
  goto_dialog_ = new QDialog(this);
  goto_dialog_->setObjectName("dialog_goto");
  goto_dialog_->setWindowTitle("Go To Specified Time");
  auto *form = new QFormLayout(goto_dialog_);
  const char *names[] = {"Year:", "Month:", "Day:", "Hour:", "Minute:", "Second:"};
  const int low[] = {1930, 1, 1, 0, 0, 0}, high[] = {3000, 12, 31, 23, 59, 59};
  for (int i = 0; i < 6; i++) {
    goto_fields_[i] = new QSpinBox;
    goto_fields_[i]->setRange(low[i], high[i]);
    form->addRow(names[i], goto_fields_[i]);
  }
  auto *apply = new QPushButton("Apply");
  auto *cancel = new QPushButton("Cancel");
  auto *row = new QHBoxLayout;
  row->addStretch(1);
  row->addWidget(cancel);
  row->addWidget(apply);
  form->addRow(row);
  connect(cancel, &QPushButton::clicked, goto_dialog_, &QDialog::hide);
  connect(apply, &QPushButton::clicked, this, &EditorWidget::gotoApply);
}

void EditorWidget::buildBufferDialog() {
  buffer_dialog_ = new QDialog(this);
  buffer_dialog_->setObjectName("dialog_buffer");
  buffer_dialog_->setWindowTitle("Buffer Controls");
  buffer_dialog_->setMinimumWidth(420);
  auto *form = new QFormLayout(buffer_dialog_);
  /* the 30000 pings of mbedit is the starting size, not a limit */
  buffer_size_ = new ScaleControl(1, 2 * mbe::DEFAULT_BUFFER_SIZE, 10000000, 0);
  buffer_hold_ = new ScaleControl(0, 2 * mbe::DEFAULT_BUFFER_SIZE, 10000000, 0);
  buffer_size_->setValueQuietly(mbe::DEFAULT_BUFFER_SIZE);
  buffer_hold_->setValueQuietly(mbe::DEFAULT_BUFFER_SIZE / 1000);
  form->addRow("Data Buffer Size:", buffer_size_);
  form->addRow("Buffer Retain Size:", buffer_hold_);
  auto *dismiss = new QPushButton("Dismiss");
  form->addRow(dismiss);
  connect(dismiss, &QPushButton::clicked, buffer_dialog_, &QDialog::hide);
  auto apply = [this] { session_.setBuffer(buffer_size_->value(), buffer_hold_->value()); };
  connect(buffer_size_, &ScaleControl::valueChanged, this, apply);
  connect(buffer_hold_, &ScaleControl::valueChanged, this, apply);
}

void EditorWidget::buildAnnotationDialog() {
  annotation_dialog_ = new QDialog(this);
  annotation_dialog_->setObjectName("dialog_annotation");
  annotation_dialog_->setWindowTitle("Annotation");
  annotation_dialog_->setMinimumWidth(420);
  auto *form = new QFormLayout(annotation_dialog_);
  x_interval_ = new ScaleControl(1, 5000, 10000000, 0);
  y_interval_ = new ScaleControl(1, 1000, 10000000, 0);
  form->addRow("X Axis Tick Interval (m):", x_interval_);
  form->addRow("Y Axis Tick Interval (m):", y_interval_);
  auto *dismiss = new QPushButton("Dismiss");
  form->addRow(dismiss);
  connect(dismiss, &QPushButton::clicked, annotation_dialog_, &QDialog::hide);
  connect(x_interval_, &ScaleControl::valueChanged, this, [this](int v) {
    session_.settings.x_interval = v;
    canvas_->redraw();
  });
  connect(y_interval_, &ScaleControl::valueChanged, this, [this](int v) {
    session_.settings.y_interval = v;
    canvas_->redraw();
  });
}

void EditorWidget::buildFiltersDialog() {
  filters_dialog_ = new QDialog(this);
  filters_dialog_->setObjectName("dialog_filters");
  filters_dialog_->setWindowTitle("Filters");
  auto *form = new QFormLayout(filters_dialog_);
  auto spin = [](int low, int high) {
    auto *box = new QSpinBox;
    box->setRange(low, high);
    return box;
  };
  auto real = [](double low, double high) {
    auto *box = new QDoubleSpinBox;
    box->setDecimals(2);
    box->setRange(low, high);
    return box;
  };
  f_median_ = new QCheckBox("Median Spike Filter");
  f_median_threshold_ = spin(1, 100);
  f_median_ltrack_ = spin(1, 2 * mbe::MAX_PLOT_PINGS);
  f_median_ltrack_->setSingleStep(2);
  f_median_xtrack_ = spin(1, 100000);
  f_median_xtrack_->setSingleStep(2);
  form->addRow(f_median_);
  form->addRow("% Median Depth Threshold:", f_median_threshold_);
  form->addRow("Median Alongtrack Dimension:", f_median_ltrack_);
  form->addRow("Median Acrosstrack Dimension:", f_median_xtrack_);
  f_wrongside_ = new QCheckBox("Wrong Side Filter");
  f_wrongside_threshold_ = spin(0, 100000);
  form->addRow(f_wrongside_);
  form->addRow("Beams from Center Threshold:", f_wrongside_threshold_);
  f_cutbeam_ = new QCheckBox("Flag by Beam Number");
  f_cutbeam_begin_ = spin(0, 100000);
  f_cutbeam_end_ = spin(0, 100000);
  form->addRow(f_cutbeam_);
  form->addRow("Start Flagging Beam Number:", f_cutbeam_begin_);
  form->addRow("End Flagging Beam Number:", f_cutbeam_end_);
  f_cutdistance_ = new QCheckBox("Flag by Distance");
  f_cutdistance_begin_ = real(-1.0e7, 1.0e7);
  f_cutdistance_end_ = real(-1.0e7, 1.0e7);
  form->addRow(f_cutdistance_);
  form->addRow("Start Flagging Distance (m):", f_cutdistance_begin_);
  form->addRow("End Flagging Distance (m):", f_cutdistance_end_);
  f_cutangle_ = new QCheckBox("Flag by Beam Angle");
  f_cutangle_begin_ = real(-90.0, 90.0);
  f_cutangle_end_ = real(-90.0, 90.0);
  form->addRow(f_cutangle_);
  form->addRow("Start Flagging Angle (deg):", f_cutangle_begin_);
  form->addRow("End Flagging Angle (deg):", f_cutangle_end_);

  /* the windows of the median have odd sizes */
  for (QSpinBox *box : {f_median_ltrack_, f_median_xtrack_}) {
    connect(box, &QSpinBox::valueChanged, this, [box](int value) {
      if (value % 2 == 0)
        box->setValue(value + 1);
    });
  }

  f_preview_ = new QCheckBox("Preview: outline what Apply would change");
  f_preview_->setObjectName("filters_preview");
  f_preview_->setChecked(true);
  f_preview_label_ = new QLabel;
  f_preview_label_->setObjectName("filters_preview_label");
  form->addRow(f_preview_);
  form->addRow(f_preview_label_);
  preview_timer_ = new QTimer(this);
  preview_timer_->setSingleShot(true);
  preview_timer_->setInterval(200);
  connect(preview_timer_, &QTimer::timeout, this, &EditorWidget::startFilterPreview);
  for (QCheckBox *box : {f_preview_, f_median_, f_wrongside_, f_cutbeam_, f_cutdistance_, f_cutangle_})
    connect(box, &QCheckBox::toggled, preview_timer_, qOverload<>(&QTimer::start));
  for (QSpinBox *box : {f_median_threshold_, f_median_ltrack_, f_median_xtrack_, f_wrongside_threshold_, f_cutbeam_begin_, f_cutbeam_end_})
    connect(box, &QSpinBox::valueChanged, preview_timer_, qOverload<>(&QTimer::start));
  for (QDoubleSpinBox *box : {f_cutdistance_begin_, f_cutdistance_end_, f_cutangle_begin_, f_cutangle_end_})
    connect(box, &QDoubleSpinBox::valueChanged, preview_timer_, qOverload<>(&QTimer::start));
  connect(filters_dialog_, &QDialog::finished, this, &EditorWidget::clearPreview);

  auto *apply = new QPushButton("Apply");
  apply->setObjectName("filters_apply");
  auto *reset = new QPushButton("Reset");
  auto *dismiss = new QPushButton("Dismiss");
  auto *row = new QHBoxLayout;
  row->addWidget(apply);
  row->addWidget(reset);
  row->addStretch(1);
  row->addWidget(dismiss);
  form->addRow(row);
  connect(apply, &QPushButton::clicked, this, &EditorWidget::filtersApply);
  connect(reset, &QPushButton::clicked, this, &EditorWidget::filtersToDialog);
  connect(dismiss, &QPushButton::clicked, this, [this] {
    filters_dialog_->hide();
    clearPreview();
  });
}

void EditorWidget::buildFileListDialog() {
  filelist_dialog_ = new QDialog(this);
  filelist_dialog_->setObjectName("dialog_filelist");
  filelist_dialog_->setWindowTitle("Files Available for Editing");
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
  auto *edit = new QPushButton("Edit Selected File");
  auto *remove = new QPushButton("Remove Selected File");
  row->addWidget(edit);
  row->addWidget(remove);
  row->addSpacing(16);
  row->addWidget(new QLabel("Output Mode:"));
  filelist_edit_ = new QRadioButton("Output Edits");
  filelist_browse_ = new QRadioButton("Browse Only");
  filelist_edit_->setChecked(true);
  row->addWidget(filelist_edit_);
  row->addWidget(filelist_browse_);
  row->addStretch(1);
  auto *dismiss = new QPushButton("Dismiss");
  row->addWidget(dismiss);
  layout->addLayout(row);
  connect(edit, &QPushButton::clicked, this, &EditorWidget::editSelectedFile);
  connect(remove, &QPushButton::clicked, this, &EditorWidget::removeSelectedFile);
  connect(dismiss, &QPushButton::clicked, filelist_dialog_, &QDialog::hide);
  connect(filelist_browse_, &QRadioButton::toggled, this, [this](bool on) { browse_action_->setChecked(on); });

  /* other programs may lock files and write edit save files meanwhile */
  filelist_timer_ = new QTimer(this);
  filelist_timer_->setInterval(1000);
  connect(filelist_timer_, &QTimer::timeout, this, [this] {
    if (filelist_dialog_->isVisible())
      syncFileList();
  });
  filelist_timer_->start();
}

/*--------------------------------------------------------------------*/
/* Reading and writing a whole file may take long, so it happens in a thread. Nothing else touches the session meanwhile. */
void EditorWidget::runJob(const QString &message, std::function<void()> work, std::function<void()> done) {
  stopBackground();
  clearPreview();
  canvas_->setMarkedPing(-1);
  emit statusMessage(message, 0);
  worker_ = QThread::create(std::move(work));
  setWorking(true);
  connect(worker_, &QThread::finished, this, [this, done = std::move(done)] {
    worker_->deleteLater();
    worker_ = nullptr;
    setWorking(false);
    canvas_->setFocus();
    canvas_->layoutChanged();
    startMbprocess();
    done();
    startSuspects();
    syncControls();
  });
  worker_->start();
}

void EditorWidget::startMbprocess() {
  if (!saved_path_.empty() && run_mbprocess_) {
    if (!QProcess::startDetached("mbprocess", {"-I", QString::fromStdString(saved_path_)}))
      showError("Unable to start mbprocess", QString::fromStdString(saved_path_), "");
    else
      emit statusMessage("Edits being applied using mbprocess...", 5000);
  }
  saved_path_.clear();
}

void EditorWidget::showError(const QString &a, const QString &b, const QString &c) { QMessageBox::warning(this, display_name_, a + "\n" + b + "\n" + c); }

void EditorWidget::addInput(const std::string &path, int format) {
  std::string message;
  if (!mbe::parse_input(path, format, files_, message))
    showError(QString::fromStdString(message), "", "");
  syncFileList();
}

void EditorWidget::setBrowse(bool browse) {
  browse_ = browse;
  browse_action_->blockSignals(true);
  browse_action_->setChecked(browse);
  browse_action_->blockSignals(false);
  (browse ? filelist_browse_ : filelist_edit_)->setChecked(true);
}

void EditorWidget::loadFirstFile(bool use_saved_edits) {
  if (!files_.empty()) {
    loadFile(0, !use_saved_edits, use_saved_edits);
  }
  else if (!start_reference_.isEmpty()) {
    const QString path = start_reference_;
    start_reference_.clear();
    loadReference(path);
  }
}

void EditorWidget::openDialog() {
  const QString path = QFileDialog::getOpenFileName(this, "Open Swath File or Datalist");
  if (path.isEmpty())
    return;
  afterCurrentFileClosed([this, path] {
    const size_t before = files_.size();
    std::string message;
    bool ok = mbe::parse_input(path.toStdString(), 0, files_, message);
    if (!ok) {
      /* the name does not tell the format */
      const int format = QInputDialog::getInt(this, "MBIO Format ID", file_name(path.toStdString()) + ": MBIO Format ID:", 0, -1, 9999, 1, &ok);
      if (ok)
        ok = mbe::parse_input(path.toStdString(), format, files_, message);
      if (!ok && !message.empty())
        showError(QString::fromStdString(message), "", "");
    }
    syncFileList();
    if (files_.size() > before)
      loadFile((int)before, true, false);
  });
}

void EditorWidget::loadFile(int index, bool ask_saved_edits, bool use_saved_edits) {
  if (index < 0 || index >= (int)files_.size())
    return;
  const std::string path = files_[index].path;
  const int format = files_[index].format;
  bool recover = false;

  /* a lock of this user on this computer may be left from a crash */
  std::string description;
  bool mine = false;
  if (mbe::Session::lockInfo(path, description, mine) && mine) {
    if (QMessageBox::question(this, display_name_,
                              QString("%1 is locked by %2.\nIf that program is no longer running, the lock was left by a crash. Open the file anyway?")
                                  .arg(file_name(path), QString::fromStdString(description))) == QMessageBox::Yes)
      mbe::Session::removeLock(path);
  }

  /* the edits of a session that ended in a crash */
  if (!browse_ && mbe::Session::hasInterruptedSession(path) && !mbe::Session::lockInfo(path, description, mine)) {
    recover = QMessageBox::question(this, display_name_,
                                    QString("The last editing session of %1 did not end normally. Recover its edits?").arg(file_name(path))) == QMessageBox::Yes;
    if (recover)
      use_saved_edits = true;
  }
  const QString esf = QString::fromStdString(mbe::Session::esfName(path));
  const bool have_esf = QFile::exists(esf) || QFile::exists(QString::fromStdString(path) + ".mbesf");
  if (!recover && ask_saved_edits && have_esf)
    use_saved_edits = QMessageBox::question(this, display_name_, QString("%1 has an edit save file.\nDo you want to apply the saved edits to the data?").arg(file_name(path))) ==
                      QMessageBox::Yes;

  current_file_ = index;
  const int mode = browse_ ? mbe::OUTPUT_BROWSE : mbe::OUTPUT_EDIT;
  canvas_->fileText = QString("File %1 of %2: %3").arg(index + 1).arg((int)files_.size()).arg(file_name(path));
  auto status = std::make_shared<int>(MB_FAILURE);
  runJob("Loading " + file_name(path) + "...", [this, path, format, use_saved_edits, mode, recover, status] { *status = session_.open(path, format, use_saved_edits, mode, recover); },
         [this, status] {
           if (*status != MB_SUCCESS) {
             if (session_.isOpen())
               session_.closeDiscard();
             current_file_ = -1;
             QApplication::beep();
           }
           else {
             emit statusMessage(QString("%1 pings loaded").arg(session_.nbuff()), 5000);
             reportReference();
           }
           canvas_->layoutChanged();
           syncFileList();
           if (!start_reference_.isEmpty()) {
             QTimer::singleShot(0, this, [this] {
               const QString path = start_reference_;
               start_reference_.clear();
               loadReference(path);
             });
           }
         });
}

/* before another file is loaded */
void EditorWidget::afterCurrentFileClosed(std::function<void()> next) {
  stopBackground();
  clearPreview();
  if (!session_.isOpen()) {
    next();
    return;
  }
  if (session_.unsaved()) {
    const auto answer = QMessageBox::question(this, display_name_, QString("Save changes to %1?\n%2 soundings changed.").arg(file_name(session_.file())).arg(session_.changes()),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Cancel)
      return;
    if (answer == QMessageBox::Save) {
      runJob("Saving edits...", [this] { session_.closeSave(); }, [this, next = std::move(next)] {
        canvas_->layoutChanged();
        current_file_ = -1;
        next();
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

void EditorWidget::saveNow() {
  if (session_.save() == MB_SUCCESS)
    emit statusMessage("Edits saved", 3000);
  syncControls();
}

bool EditorWidget::prepareToClose(std::function<void()> closed_later) {
  if (busy())
    return false;
  stopBackground();
  clearPreview();
  if (session_.isOpen()) {
    if (session_.unsaved()) {
      const auto answer = QMessageBox::question(this, display_name_, QString("Save changes to %1?\n%2 soundings changed.").arg(file_name(session_.file())).arg(session_.changes()),
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
  return true;
}

void EditorWidget::doneOrNextFile() {
  auto quit = std::make_shared<bool>(false);
  runJob("Saving edits...", [this, quit] { session_.done(quit.get()); }, [this, quit] {
    canvas_->layoutChanged();
    syncFileList();
    if (*quit) {
      emit closeRequested();
    }
    else if (current_file_ >= 0 && current_file_ < (int)files_.size() - 1) {
      const int next = current_file_ + 1;
      QTimer::singleShot(0, this, [this, next] { loadFile(next, true, false); });
    }
    else {
      current_file_ = -1;
    }
  });
}

void EditorWidget::nextBuffer() {
  auto quit = std::make_shared<bool>(false);
  runJob("Reading the next buffer...", [this, quit] { session_.nextBuffer(quit.get()); }, [this, quit] {
    canvas_->layoutChanged();
    if (*quit)
      emit closeRequested();
  });
}

void EditorWidget::gotoApply() {
  int t[7] = {0, 0, 0, 0, 0, 0, 0};
  for (int i = 0; i < 6; i++)
    t[i] = goto_fields_[i]->value();
  goto_dialog_->hide();
  auto target = std::make_shared<std::vector<int>>(t, t + 7);
  runJob("Looking for the time...", [this, target] { session_.gotoTime(target->data()); }, [this] { canvas_->layoutChanged(); });
}

/*--------------------------------------------------------------------*/
void EditorWidget::syncControls() {
  const bool open = session_.isOpen();
  const bool have_data = open && session_.nbuff() > 0;
  slider_->blockSignals(true);
  slider_->setRange(0, std::max(session_.nbuff() - 1, 0));
  slider_->setValue(have_data ? session_.current() : 0);
  slider_->setPageStep(canvas_->stepSize);
  slider_->setEnabled(have_data);
  slider_->blockSignals(false);
  if (have_data) {
    const mbe::Ping &p = session_.pings()[session_.current()];
    const QDateTime time = QDateTime::fromMSecsSinceEpoch((qint64)(p.time_d * 1000.0), QTimeZone::utc());
    position_->setText(QString("%1-%2 of %3   %4")
                           .arg(session_.current() + 1)
                           .arg(session_.current() + session_.nplot())
                           .arg(session_.nbuff())
                           .arg(time.toString("yyyy-MM-dd HH:mm:ss")));
  }
  else {
    position_->clear();
  }
  for (QPushButton *button : {start_, reverse_, forward_, end_, flag_view_, unflag_view_, unflag_forward_})
    button->setEnabled(have_data);
  done_->setEnabled(open);
  done_->setText(current_file_ >= 0 && current_file_ < (int)files_.size() - 1 ? "Next File" : "Done");
  next_->setEnabled(open && session_.moreData());
  save_action_->setEnabled(open && session_.outputMode() == mbe::OUTPUT_EDIT);
  undo_action_->setEnabled(session_.canUndo());
  undo_action_->setText(session_.canUndo() ? "Undo " + QString::fromStdString(session_.undoLabel()) : "Undo");
  redo_action_->setEnabled(session_.canRedo());
  redo_action_->setText(session_.canRedo() ? "Redo " + QString::fromStdString(session_.redoLabel()) : "Redo");
  show_amplitude_->setEnabled(!have_data || session_.hasAmplitude());
  show_residual_->setEnabled(session_.hasReference());
  clear_reference_->setEnabled(session_.hasReference());
  history_action_->setEnabled(open);
  water_column_->setEnabled(have_data && !waterColumnFile().isEmpty());
  mbeditviz_->setEnabled(open);
  if (session_.unsaved() != unsaved_) {
    unsaved_ = session_.unsaved();
    emit unsavedChanged(unsaved_);
  }
  const QString title = open ? file_name(session_.file()) + "[*] - " + display_name_ : display_name_ + "[*]";
  if (title != title_) {
    title_ = title;
    emit titleChanged(title_);
  }

  width_->setValueQuietly(session_.settings.plot_width);
  exagger_->setValueQuietly(session_.settings.exagger);
  pings_->setValueQuietly(session_.settings.plot_size);
  step_->setValueQuietly(canvas_->stepSize);
  panel_->setVisible(session_.settings.show_time > mbe::PLOT_TIME);
  panel_->update();
  map_->update();
}

void EditorWidget::syncFileList() {
  if (filelist_ == nullptr)
    return;
  int selected = filelist_->currentRow();
  filelist_->setRowCount((int)files_.size());
  for (int i = 0; i < (int)files_.size(); i++) {
    std::string description;
    bool mine = false;
    QString state;
    if (i == current_file_ && session_.isOpen())
      state = "<loaded>";
    else if (mbe::Session::lockInfo(files_[i].path, description, mine))
      state = "<locked>";
    const bool esf = QFile::exists(QString::fromStdString(mbe::Session::esfName(files_[i].path)));
    const QStringList cells = {state, esf ? "<esf>" : "", QString::fromStdString(files_[i].path), QString::number(files_[i].format)};
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

void EditorWidget::applySettings() {
  if (session_.isOpen() && session_.nbuff() > 0)
    session_.plot(false);
  canvas_->redraw();
  syncControls();
}

void EditorWidget::afterEdit() {
  syncControls();
  if (filters_dialog_->isVisible())
    preview_timer_->start();
  if (reasons_dialog_->isVisible())
    updateReasonCount();
}

void EditorWidget::afterMove() {
  syncControls();
  /* Apply starts from the first ping in view */
  if (filters_dialog_->isVisible())
    preview_timer_->start();
  if (reasons_dialog_->isVisible())
    updateReasonCount();
}

void EditorWidget::setModeFromUi(int mode) {
  canvas_->setMode((Canvas::Mode)mode);
  canvas_->setFocus();
}

void EditorWidget::setViewMode(int view) {
  session_.setViewMode(view);
  for (QAction *action : view_group_->actions())
    action->setChecked(action->data().toInt() == view);
  applySettings();
}

void EditorWidget::setPlotMode(int mode) {
  session_.setPlotMode(mode);
  syncControls();
  canvas_->layoutChanged();
  panel_->update();
}

void EditorWidget::setShowMode(int mode) {
  session_.settings.show_mode = mode;
  applySettings();
}

void EditorWidget::pingsChanged(int value) {
  /* the step keeps its ratio to the number of pings shown */
  const double ratio = ((double)canvas_->stepSize) / ((double)session_.settings.plot_size);
  session_.settings.plot_size = value;
  canvas_->stepSize = std::min(std::max((int)(ratio * value), 1), value);
  applySettings();
}

void EditorWidget::stepChanged(int value) {
  canvas_->stepSize = value;
  if (value > session_.settings.plot_size) {
    session_.settings.plot_size = value;
    applySettings();
  }
  else {
    syncControls();
  }
}

void EditorWidget::sliderMoved(int value) {
  if (session_.isOpen()) {
    session_.step(value - session_.current());
    canvas_->redraw();
    syncControls();
  }
}

void EditorWidget::flagAction(std::function<int()> action) {
  action();
  canvas_->redraw();
  syncControls();
}

/*--------------------------------------------------------------------*/
void EditorWidget::filtersToDialog() {
  const mbe::Filters &f = session_.filters;
  f_median_->setChecked(f.medianspike);
  f_median_threshold_->setValue(f.medianspike_threshold);
  f_median_ltrack_->setValue(f.medianspike_ltrack);
  f_median_xtrack_->setValue(f.medianspike_xtrack);
  f_wrongside_->setChecked(f.wrongside);
  f_wrongside_threshold_->setValue(f.wrongside_threshold);
  f_cutbeam_->setChecked(f.cutbeam);
  f_cutbeam_begin_->setValue(f.cutbeam_begin);
  f_cutbeam_end_->setValue(f.cutbeam_end);
  f_cutdistance_->setChecked(f.cutdistance);
  f_cutdistance_begin_->setValue(f.cutdistance_begin);
  f_cutdistance_end_->setValue(f.cutdistance_end);
  f_cutangle_->setChecked(f.cutangle);
  f_cutangle_begin_->setValue(f.cutangle_begin);
  f_cutangle_end_->setValue(f.cutangle_end);
}

mbe::Filters EditorWidget::filtersFromDialog() const {
  mbe::Filters f;
  f.medianspike = f_median_->isChecked();
  f.medianspike_threshold = f_median_threshold_->value();
  f.medianspike_ltrack = f_median_ltrack_->value();
  f.medianspike_xtrack = f_median_xtrack_->value();
  f.wrongside = f_wrongside_->isChecked();
  f.wrongside_threshold = f_wrongside_threshold_->value();
  f.cutbeam = f_cutbeam_->isChecked();
  f.cutbeam_begin = f_cutbeam_begin_->value();
  f.cutbeam_end = f_cutbeam_end_->value();
  f.cutdistance = f_cutdistance_->isChecked();
  f.cutdistance_begin = f_cutdistance_begin_->value();
  f.cutdistance_end = f_cutdistance_end_->value();
  f.cutangle = f_cutangle_->isChecked();
  f.cutangle_begin = f_cutangle_begin_->value();
  f.cutangle_end = f_cutangle_end_->value();
  return f;
}

void EditorWidget::filtersApply() {
  clearPreview();
  session_.filters = filtersFromDialog();
  runJob("Applying bathymetry filters...", [this] { session_.filterForward(); }, [this] { canvas_->redraw(); });
}

/*--------------------------------------------------------------------*/
/* The thread reads the depths, which do not change, and a copy of the flags. */
void EditorWidget::stopBackground() {
  preview_timer_->stop();
  background_cancel_ = true;
  if (preview_thread_ != nullptr) {
    preview_thread_->wait();
    preview_thread_ = nullptr;
  }
  if (suspects_thread_ != nullptr) {
    suspects_thread_->wait();
    suspects_thread_ = nullptr;
  }
  background_cancel_ = false;
  preview_generation_++;
}

void EditorWidget::startFilterPreview() {
  if (!filters_dialog_->isVisible() || !session_.isOpen() || session_.nbuff() == 0 || busy())
    return;
  if (!f_preview_->isChecked()) {
    clearPreview();
    return;
  }
  /* a run still going is out of date */
  if (preview_thread_ != nullptr) {
    stopBackground();
    startSuspects();
  }
  const int generation = ++preview_generation_;
  auto result = std::make_shared<mbe::FlagPreview>();
  auto flags = std::make_shared<mbe::Session::FlagCopy>(session_.copyFlags());
  const mbe::Filters filters = filtersFromDialog();
  const int first = session_.current();
  QThread *thread = QThread::create([this, result, flags, filters, first] { *result = session_.previewFilters(filters, std::move(*flags), first, &background_cancel_); });
  preview_thread_ = thread;
  connect(thread, &QThread::finished, this, [this, thread, generation, result] {
    if (preview_thread_ == thread)
      preview_thread_ = nullptr;
    thread->deleteLater();
    if (generation == preview_generation_) {
      showPreview(result);
      f_preview_label_->setText(QString("Would flag %1 and unflag %2 soundings in %3 pings").arg(result->nflag).arg(result->nunflag).arg(result->npings));
    }
  });
  thread->start();
}

void EditorWidget::showPreview(std::shared_ptr<mbe::FlagPreview> preview) {
  preview_ = *preview;
  canvas_->setPreview(&preview_);
}

void EditorWidget::clearPreview() {
  preview_timer_->stop();
  preview_generation_++;
  preview_ = mbe::FlagPreview();
  canvas_->setPreview(nullptr);
  f_preview_label_->clear();
}

void EditorWidget::startSuspects() {
  suspects_.clear();
  suspect_index_ = -1;
  strip_->setSuspects(suspects_, 0);
  canvas_->setMarkedPing(-1);
  if (!session_.isOpen() || session_.nbuff() == 0 || suspects_thread_ != nullptr)
    return;
  auto result = std::make_shared<std::vector<mbe::Suspect>>();
  auto flags = std::make_shared<mbe::Session::FlagCopy>(session_.copyFlags());
  QThread *thread = QThread::create([this, result, flags] { *result = session_.findSuspects(*flags, &background_cancel_); });
  suspects_thread_ = thread;
  connect(thread, &QThread::finished, this, [this, thread, result] {
    const bool current = suspects_thread_ == thread;
    if (current)
      suspects_thread_ = nullptr;
    thread->deleteLater();
    if (current && session_.isOpen()) {
      suspects_ = *result;
      strip_->setSuspects(suspects_, session_.nbuff());
    }
  });
  thread->start();
}

void EditorWidget::showPing(int ping) {
  if (!session_.isOpen() || session_.nbuff() == 0)
    return;
  const int target = std::max(ping - session_.settings.plot_size / 2, 0);
  session_.step(target - session_.current());
  canvas_->redraw();
  afterMove();
}

void EditorWidget::nextSuspect(int direction) {
  if (suspects_.empty()) {
    emit statusMessage("No suspect pings", 3000);
    return;
  }
  const int index = suspect_index_ + direction;
  if (index < 0 || index >= (int)suspects_.size()) {
    QApplication::beep();
    return;
  }
  suspect_index_ = index;
  const mbe::Suspect &s = suspects_[index];
  showPing(s.ping);
  canvas_->setMarkedPing(s.ping);
  QStringList why;
  if (s.spikes > 0)
    why << QString("%1 soundings far from their neighbours").arg(s.spikes);
  if (s.deficit > 0.0)
    why << QString("%1% fewer good soundings").arg((int)(100.0 * s.deficit));
  if (s.attitude_jump)
    why << "attitude jump";
  if (s.time_gap)
    why << "time gap";
  emit statusMessage(QString("Suspect %1 of %2: ping %3: %4").arg(index + 1).arg((int)suspects_.size()).arg(session_.pings()[s.ping].record + 1).arg(why.join(", ")), 0);
}

/*--------------------------------------------------------------------*/
void EditorWidget::buildReasonsDialog() {
  reasons_dialog_ = new QDialog(this);
  reasons_dialog_->setObjectName("dialog_reasons");
  reasons_dialog_->setWindowTitle("Flags by Reason");
  auto *form = new QFormLayout(reasons_dialog_);
  show_manual_ = new QCheckBox("Show manual flags");
  show_filter_ = new QCheckBox("Show filter flags");
  show_sonar_ = new QCheckBox("Show sonar flags");
  show_manual_->setObjectName("show_manual");
  show_filter_->setObjectName("show_filter");
  show_sonar_->setObjectName("show_sonar");
  for (QCheckBox *box : {show_manual_, show_filter_, show_sonar_}) {
    box->setChecked(true);
    form->addRow(box);
    connect(box, &QCheckBox::toggled, this, [this] {
      session_.shown_reasons = (show_manual_->isChecked() ? mbe::REASON_MANUAL : 0) | (show_filter_->isChecked() ? mbe::REASON_FILTER : 0) | (show_sonar_->isChecked() ? mbe::REASON_SONAR : 0);
      canvas_->redraw();
    });
  }
  reason_ = new QComboBox;
  reason_->setObjectName("reason");
  reason_->addItem("Manual", mbe::REASON_MANUAL);
  reason_->addItem("Filter", mbe::REASON_FILTER);
  reason_->addItem("Sonar", mbe::REASON_SONAR);
  scope_ = new QComboBox;
  scope_->setObjectName("scope");
  scope_->addItem("Pings in view", mbe::SCOPE_VIEW);
  scope_->addItem("From the first ping in view", mbe::SCOPE_FORWARD);
  scope_->addItem("All pings in memory", mbe::SCOPE_BUFFER);
  form->addRow("Reason:", reason_);
  form->addRow("Pings:", scope_);
  reason_count_ = new QLabel;
  reason_count_->setObjectName("reason_count");
  form->addRow(reason_count_);
  connect(reason_, &QComboBox::currentIndexChanged, this, &EditorWidget::updateReasonCount);
  connect(scope_, &QComboBox::currentIndexChanged, this, &EditorWidget::updateReasonCount);
  auto *unflag = new QPushButton("Unflag");
  unflag->setObjectName("reason_unflag");
  auto *dismiss = new QPushButton("Dismiss");
  auto *row = new QHBoxLayout;
  row->addWidget(unflag);
  row->addStretch(1);
  row->addWidget(dismiss);
  form->addRow(row);
  connect(unflag, &QPushButton::clicked, this, [this] {
    session_.unflagReason(reason_->currentData().toInt(), scope_->currentData().toInt());
    canvas_->redraw();
    afterEdit();
  });
  connect(dismiss, &QPushButton::clicked, reasons_dialog_, &QDialog::hide);
}

void EditorWidget::updateReasonCount() {
  reason_count_->setText(QString("%1 soundings").arg(session_.countReason(reason_->currentData().toInt(), scope_->currentData().toInt())));
}

/*--------------------------------------------------------------------*/
void EditorWidget::buildMbcleanDialog() {
  mbclean_dialog_ = new QDialog(this);
  mbclean_dialog_->setObjectName("dialog_mbclean");
  mbclean_dialog_->setWindowTitle("Preview mbclean");
  mbclean_dialog_->setMinimumWidth(560);
  auto *layout = new QVBoxLayout(mbclean_dialog_);
  layout->addWidget(new QLabel("mbclean runs on a scratch folder with the current edits; nothing changes until you accept.\nOptions:"));
  mbclean_args_ = new QLineEdit(QSettings("MB-System", "mbedit-qt").value("mbclean_options").toString());
  mbclean_args_->setObjectName("mbclean_options");
  mbclean_args_->setPlaceholderText("for example --max-slope=45/0");
  layout->addWidget(mbclean_args_);
  mbclean_status_ = new QLabel;
  mbclean_status_->setObjectName("mbclean_status");
  mbclean_status_->setWordWrap(true);
  layout->addWidget(mbclean_status_);
  mbclean_run_ = new QPushButton("Run");
  mbclean_run_->setObjectName("mbclean_run");
  mbclean_accept_ = new QPushButton("Accept");
  mbclean_accept_->setObjectName("mbclean_accept");
  mbclean_reject_ = new QPushButton("Reject");
  mbclean_reject_->setObjectName("mbclean_reject");
  mbclean_accept_->setEnabled(false);
  mbclean_reject_->setEnabled(false);
  auto *dismiss = new QPushButton("Dismiss");
  auto *row = new QHBoxLayout;
  row->addWidget(mbclean_run_);
  row->addWidget(mbclean_accept_);
  row->addWidget(mbclean_reject_);
  row->addStretch(1);
  row->addWidget(dismiss);
  layout->addLayout(row);
  connect(mbclean_run_, &QPushButton::clicked, this, &EditorWidget::runMbclean);
  auto clear = [this] {
    clearPreview();
    mbclean_accept_->setEnabled(false);
    mbclean_reject_->setEnabled(false);
  };
  connect(mbclean_accept_, &QPushButton::clicked, this, [this, clear] {
    session_.applyPreview(preview_, "mbclean");
    clear();
    mbclean_status_->setText("Accepted. Undo takes it back.");
    canvas_->redraw();
    afterEdit();
  });
  connect(mbclean_reject_, &QPushButton::clicked, this, [this, clear] {
    clear();
    mbclean_status_->setText("Rejected. Nothing changed.");
  });
  connect(dismiss, &QPushButton::clicked, this, [this, clear] {
    clear();
    mbclean_dialog_->hide();
  });
}

/* beside this program first, for a build that is not installed */
QString EditorWidget::findProgram(const QString &name) {
  const QString here = QCoreApplication::applicationDirPath();
  const QString program = QStandardPaths::findExecutable(name, {here, here + "/../utilities", here + "/../" + name});
  return program.isEmpty() ? QStandardPaths::findExecutable(name) : program;
}

void EditorWidget::runMbclean() {
  if (!session_.isOpen() || session_.nbuff() == 0 || mbclean_process_ != nullptr)
    return;
  QString program = findProgram("mbclean");
  if (program.isEmpty()) {
    mbclean_status_->setText("mbclean was not found.");
    return;
  }
  QSettings("MB-System", "mbedit-qt").setValue("mbclean_options", mbclean_args_->text());

  /* a link to the swath file, so that its lock and edit save file are not touched */
  mbclean_dir_ = std::make_unique<QTemporaryDir>();
  const QString source = QFileInfo(QString::fromStdString(session_.file())).absoluteFilePath();
  const QString name = QFileInfo(source).fileName();
  const QString link = mbclean_dir_->filePath(name);
  std::error_code code;
  std::filesystem::create_symlink(source.toStdString(), link.toStdString(), code);
  if (code || !mbclean_dir_->isValid() || !session_.writeCurrentEdits((link + ".esf").toStdString())) {
    mbclean_status_->setText("Unable to prepare a scratch folder.");
    return;
  }
  mbclean_output_ = link + ".esf";
  mbclean_process_ = new QProcess(this);
  mbclean_process_->setWorkingDirectory(mbclean_dir_->path());
  mbclean_process_->setProcessChannelMode(QProcess::MergedChannels);
  QStringList arguments = {QString("-F%1").arg(session_.format()), "-I" + name};
  arguments += QProcess::splitCommand(mbclean_args_->text());
  mbclean_run_->setEnabled(false);
  mbclean_status_->setText("Running mbclean...");
  connect(mbclean_process_, &QProcess::finished, this, &EditorWidget::mbcleanFinished);
  connect(mbclean_process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart)
      mbcleanFinished();
  });
  mbclean_process_->start(program, arguments);
}

void EditorWidget::mbcleanFinished() {
  QProcess *process = mbclean_process_;
  if (process == nullptr)
    return;
  mbclean_process_ = nullptr;
  mbclean_run_->setEnabled(true);
  const QString output = QString::fromLocal8Bit(process->readAll());
  const bool ok = process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0 && process->error() != QProcess::FailedToStart;
  process->deleteLater();
  if (!ok) {
    mbclean_status_->setText("mbclean failed:\n" + output.right(400));
    return;
  }
  if (!session_.isOpen())
    return;
  auto preview = std::make_shared<mbe::FlagPreview>();
  std::string message;
  if (!session_.previewEditFile(mbclean_output_.toStdString(), *preview, message)) {
    mbclean_status_->setText(QString::fromStdString(message));
    return;
  }
  showPreview(preview);
  mbclean_accept_->setEnabled(true);
  mbclean_reject_->setEnabled(true);
  mbclean_status_->setText(QString("mbclean would flag %1 and unflag %2 soundings in %3 pings.%4")
                               .arg(preview->nflag)
                               .arg(preview->nunflag)
                               .arg(preview->npings)
                               .arg(preview->complete ? "" : " It also edited pings beyond those in memory; those are not taken."));
}

void EditorWidget::editSelectedFile() {
  const int row = filelist_->currentRow();
  if (row < 0 || row == current_file_)
    return;
  afterCurrentFileClosed([this, row] { loadFile(row, true, false); });
}

void EditorWidget::removeSelectedFile() {
  const int row = filelist_->currentRow();
  if (row < 0 || row == current_file_)
    return;
  files_.erase(files_.begin() + row);
  if (current_file_ > row)
    current_file_--;
  syncFileList();
  syncControls();
}

void EditorWidget::about() {
  QMessageBox::about(this, "About mbedit-qt",
                     QString("<b>mbedit-qt</b><br>Interactive Swath Bathymetry Editor<br>One component of the MB-System open source software package "
                             "for processing and display of swath sonar data.<br><br>MB-System Release %1, %2<br><br>Created by David W. Caress and Dale N. Chayes. "
                             "Qt6 port by Christian dos Santos Ferreira.")
                         .arg(MB_VERSION, MB_VERSION_DATE));
}

/*--------------------------------------------------------------------*/
void EditorWidget::loadReference(const QString &path) {
  if (busy())
    return;
  stopBackground();
  auto message = std::make_shared<std::string>();
  auto ok = std::make_shared<bool>(false);
  const std::string name = path.toStdString();
  runJob("Reading " + file_name(name) + "...", [this, name, message, ok] { *ok = session_.loadReference(name, *message); }, [this, path, message, ok] {
    if (*ok) {
      session_.settings.show_mode = mbe::SHOW_RESIDUAL;
      show_residual_->setEnabled(true);
      show_residual_->setChecked(true);
      applySettings();
      startSuspects();
      reportReference();
    }
    else {
      showError("Unable to use the reference surface", QString::fromStdString(*message), path);
    }
  });
}

void EditorWidget::clearReference() {
  stopBackground();
  session_.clearReference();
  if (session_.settings.show_mode == mbe::SHOW_RESIDUAL) {
    for (QAction *action : show_group_->actions()) {
      if (action->data().toInt() == mbe::SHOW_FLAG)
        action->setChecked(true);
    }
    session_.settings.show_mode = mbe::SHOW_FLAG;
  }
  applySettings();
  startSuspects();
}

/* the kind of grid is found from the data, so it is said, and also if the grid is elsewhere */
void EditorWidget::reportReference() {
  if (!session_.hasReference() || !session_.isOpen())
    return;
  const long compared = session_.updateReferenceSign();
  canvas_->redraw();
  if (session_.nbuff() > 0 && compared == 0)
    emit statusMessage("The reference surface does not cover these pings", 0);
  else if (session_.nbuff() > 0)
    emit statusMessage(QString("Reference surface read as %1").arg(session_.referenceIsTopography() ? "topography (seafloor negative)" : "depth (seafloor positive)"), 0);
}

/*--------------------------------------------------------------------*/
/* The file of this line that mbwcdview reads: the file itself, or one beside it */
QString EditorWidget::waterColumnFile() const {
  if (!session_.isOpen())
    return QString();
  const QFileInfo info(QString::fromStdString(session_.file()));
  const QStringList kinds = {"kmwcd", "kmall", "wcd", "all"};
  if (kinds.contains(info.suffix().toLower()))
    return info.absoluteFilePath();
  for (const QString &kind : kinds) {
    const QString other = info.absolutePath() + "/" + info.completeBaseName() + "." + kind;
    if (QFileInfo::exists(other))
      return other;
  }
  return QString();
}

void EditorWidget::openWaterColumn(int ping) {
  const QString program = findProgram("mbwcdview"), path = waterColumnFile();
  if (program.isEmpty() || path.isEmpty() || session_.nbuff() == 0)
    return;
  const long index = session_.pingsBefore() + std::min(std::max(ping, 0), session_.nbuff() - 1);
  if (QProcess::startDetached(program, {"--input=" + path, QString("--ping=%1").arg(index)}))
    emit statusMessage(QString("Opened %1 at ping %2: mbwcdview counts water column pings, so it may differ").arg(file_name(path.toStdString())).arg(index), 8000);
  else
    showError("Unable to start mbwcdview", path, "");
}

void EditorWidget::openMbeditviz() {
  const QString program = findProgram("mbeditviz");
  if (program.isEmpty()) {
    showError("mbeditviz was not found", "", "");
    return;
  }
  const QString path = QString::fromStdString(session_.file()), format = QString::number(session_.format());
  /* it takes the lock, so this program must have closed the file first */
  afterCurrentFileClosed([this, program, path, format] {
    syncControls();
    if (!QProcess::startDetached(program, {"-F", format, "-I", path}))
      showError("Unable to start mbeditviz", path, "");
  });
}

/*--------------------------------------------------------------------*/
void EditorWidget::buildHistoryDialog() {
  history_dialog_ = new QDialog(this);
  history_dialog_->setObjectName("dialog_history");
  history_dialog_->setWindowTitle("Edit History");
  history_dialog_->resize(720, 340);
  auto *layout = new QVBoxLayout(history_dialog_);
  history_table_ = new QTableWidget(0, 6);
  history_table_->setObjectName("history_table");
  history_table_->setHorizontalHeaderLabels({"Version", "Saved (UTC)", "Program", "User", "Edits", "Note"});
  history_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  history_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  history_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  history_table_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(history_table_);
  history_status_ = new QLabel;
  history_status_->setObjectName("history_status");
  layout->addWidget(history_status_);
  auto *buttons = new QHBoxLayout;
  auto *preview = new QPushButton("Preview");
  auto *clear = new QPushButton("Clear Preview");
  auto *restore = new QPushButton("Restore...");
  auto *close = new QPushButton("Close");
  preview->setObjectName("history_preview");
  clear->setObjectName("history_clear");
  restore->setObjectName("history_restore");
  buttons->addWidget(preview);
  buttons->addWidget(clear);
  buttons->addStretch(1);
  buttons->addWidget(restore);
  buttons->addWidget(close);
  layout->addLayout(buttons);
  connect(preview, &QPushButton::clicked, this, &EditorWidget::historyPreview);
  connect(clear, &QPushButton::clicked, this, [this] {
    clearPreview();
    history_status_->clear();
  });
  connect(restore, &QPushButton::clicked, this, &EditorWidget::historyRestore);
  connect(close, &QPushButton::clicked, history_dialog_, &QDialog::hide);
  connect(history_dialog_, &QDialog::finished, this, [this] { clearPreview(); });
}

void EditorWidget::showHistory() {
  history_ = mbe::Session::historyVersions(session_.file());
  history_table_->setRowCount((int)history_.size());
  for (int i = 0; i < (int)history_.size(); i++) {
    const mbe::Session::HistoryVersion &v = history_[i];
    const QStringList cells = {QString::number(v.version), QString::fromStdString(v.utc), QString::fromStdString(v.program),
                               QString::fromStdString(v.user), QString::number(v.nedits), QString::fromStdString(v.note)};
    for (int k = 0; k < cells.size(); k++)
      history_table_->setItem(i, k, new QTableWidgetItem(cells[k]));
  }
  history_table_->resizeColumnsToContents();
  if (!history_.empty())
    history_table_->selectRow((int)history_.size() - 1);
  history_status_->setText(history_.empty() ? "No earlier versions of the edits of this file." : "The edits now in the file are newer than all of these.");
  history_dialog_->show();
  history_dialog_->raise();
}

void EditorWidget::historyPreview() {
  const int row = history_table_->currentRow();
  if (row < 0 || row >= (int)history_.size())
    return;
  mbe::FlagPreview preview;
  std::string message;
  if (!session_.previewEditFile(history_[row].path, preview, message)) {
    history_status_->setText(QString::fromStdString(message));
    return;
  }
  showPreview(std::make_shared<mbe::FlagPreview>(preview));
  history_status_->setText(QString("Compared with now, version %1 would flag %2 and unflag %3 soundings in %4 pings%5.")
                               .arg(history_[row].version)
                               .arg(preview.nflag)
                               .arg(preview.nunflag)
                               .arg(preview.npings)
                               .arg(preview.complete ? "" : " in memory; it reaches further"));
}

void EditorWidget::historyRestore() {
  const int row = history_table_->currentRow();
  if (row < 0 || row >= (int)history_.size())
    return;
  const QString program = findProgram("mbhistory");
  if (program.isEmpty()) {
    history_status_->setText("mbhistory was not found.");
    return;
  }
  const int version = history_[row].version, index = current_file_;
  const QString path = QString::fromStdString(session_.file());
  if (QMessageBox::question(this, display_name_, QString("Restore version %1 of the edits and reopen the file?\nThe edits now in the file are kept as a new version.").arg(version)) != QMessageBox::Yes)
    return;
  history_dialog_->hide();
  /* mbhistory takes the same lock, so the file must be closed first */
  afterCurrentFileClosed([this, program, path, version, index] {
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, {"--input=" + path, QString("--restore=%1").arg(version), "--yes"});
    const bool ok = process.waitForFinished(60000) && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    if (!ok)
      showError("Unable to restore the edits", QString::fromUtf8(process.readAll()).left(300), path);
    syncControls();
    if (index >= 0 && index < (int)files_.size())
      loadFile(index, false, true);
  });
}

/*--------------------------------------------------------------------*/
void EditorWidget::flagWholePing(int ping, bool flag) {
  mbe::FlagPreview preview;
  const mbe::Ping &p = session_.pings()[ping];
  const mbe::PlotBox &box = session_.box();
  for (int j = 0; j < p.beams_bath; j++) {
    /* zoomed, what is hidden is not edited */
    if (session_.zoomed() && (p.bath_x[j] < box.xmin || p.bath_x[j] > box.xmax || p.bath_y[j] < box.ymin || p.bath_y[j] > box.ymax))
      continue;
    if (flag && mb_beam_ok(p.beamflag[j]))
      preview.items.push_back({ping, j, (char)(MB_FLAG_FLAG + MB_FLAG_MANUAL)});
    else if (!flag && !mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j]))
      preview.items.push_back({ping, j, (char)MB_FLAG_NONE});
  }
  session_.applyPreview(preview, flag ? "Flag Ping" : "Unflag Ping");
  canvas_->redraw();
  afterEdit();
}

void EditorWidget::pingContextMenu(QPoint global, int ping, int beam) {
  if (!session_.isOpen() || session_.nbuff() == 0)
    return;
  QMenu menu(this);
  menu.setObjectName("context_menu");
  if (ping >= 0) {
    const mbe::Ping &p = session_.pings()[ping];
    menu.addSection(QString("Ping %1, beam %2, depth %3 m").arg(session_.pingsBefore() + ping).arg(beam).arg(p.bath[beam], 0, 'f', 2));
    menu.addAction("Flag This Ping", this, [this, ping] { flagWholePing(ping, true); });
    menu.addAction("Unflag This Ping", this, [this, ping] { flagWholePing(ping, false); });
    menu.addAction("Center on This Ping", this, [this, ping] { showPing(ping); });
    if (water_column_->isVisible() && water_column_->isEnabled())
      menu.addAction("Water Column at This Ping", this, [this, ping] { openWaterColumn(ping); });
    menu.addSeparator();
  }
  if (undo_action_->isEnabled())
    menu.addAction(undo_action_);
  if (redo_action_->isEnabled())
    menu.addAction(redo_action_);
  menu.exec(global);
}

}  // namespace mbe
