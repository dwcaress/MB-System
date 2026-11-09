/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_widget.cc  2/11/2026
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

#include "mbvt_widget.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <initializer_list>
#include <memory>
#include <utility>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHideEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QRadioButton>
#include <QScreen>
#include <QShowEvent>
#include <QSlider>
#include <QSplitter>
#include <QThread>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"

namespace mbvt {

namespace {

constexpr double DEFAULT_SIMPLIFY_TOLERANCE = 0.25;  // m/s; on real 500-900 node profiles: 23-89 nodes, residuals within about 0.1 m

QString file_name(const QString &path) { return QFileInfo(path).fileName(); }
QString file_name(const std::string &path) { return file_name(QString::fromStdString(path)); }

constexpr int ASK_ABOVE_NODES = 50;  // a longer profile offers to be simplified when it is activated

}  // namespace

/*--------------------------------------------------------------------*/
ScalingControls::ScalingControls(QWidget *parent) : QWidget(parent) {
  auto *form = new QFormLayout(this);
  form->setContentsMargins(0, 0, 0, 0);
  maxdepth_ = addRow(form, "Maximum Depth (m):", 1.0, 17000.0, 0);
  velrange_ = addRow(form, "Velocity Range (m/s):", 25.0, 1000.0, 0);
  velcenter_ = addRow(form, "Velocity Center (m/s):", 1300.0, 1700.0, 0);
  resrange_ = addRow(form, "Residual Range (m):", 0.1, 200.0, 1);
}

ScalingControls::Row ScalingControls::addRow(QFormLayout *form, const QString &label, double low, double high, int decimals) {
  Row row;
  row.factor = decimals > 0 ? 10.0 : 1.0;
  row.slider = new QSlider(Qt::Horizontal);
  row.slider->setRange((int)(low * row.factor), (int)(high * row.factor));
  row.spin = new QDoubleSpinBox;
  row.spin->setRange(low, high);
  row.spin->setDecimals(decimals);
  auto *line = new QHBoxLayout;
  line->addWidget(row.slider, 1);
  line->addWidget(row.spin);
  form->addRow(label, line);
  const Row copy = row;
  connect(row.slider, &QSlider::valueChanged, this, [this, copy](int value) {
    const QSignalBlocker block(copy.spin);
    copy.spin->setValue(value / copy.factor);
    emit changed();
  });
  connect(row.spin, &QDoubleSpinBox::valueChanged, this, [this, copy](double value) {
    const QSignalBlocker block(copy.slider);
    copy.slider->setValue((int)(value * copy.factor + 0.5));
    emit changed();
  });
  return row;
}

void ScalingControls::setRow(const Row &row, double value) {
  const QSignalBlocker block_slider(row.slider), block_spin(row.spin);
  row.spin->setValue(value);
  row.slider->setValue((int)(row.spin->value() * row.factor + 0.5));
}

void ScalingControls::setScale(const PlotScale &scale) {
  setRow(maxdepth_, scale.maxdepth);
  setRow(velrange_, scale.velrange);
  setRow(velcenter_, scale.velcenter);
  setRow(resrange_, scale.resrange);
}

PlotScale ScalingControls::scale() const {
  PlotScale scale;
  scale.maxdepth = maxdepth_.spin->value();
  scale.velrange = velrange_.spin->value();
  scale.velcenter = velcenter_.spin->value();
  scale.resrange = resrange_.spin->value();
  return scale;
}

ScalingDialog::ScalingDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Plot Scaling");
  controls_ = new ScalingControls;
  auto *layout = new QVBoxLayout(this);
  layout->addWidget(controls_);
  setMinimumWidth(460);
  connect(controls_, &ScalingControls::changed, this, &ScalingDialog::changed);
}

/*--------------------------------------------------------------------*/
ActivateDialog::ActivateDialog(const mbvt::Profile &profile, QWidget *parent) : QDialog(parent) {
  setWindowTitle("Activate as Editing Profile");
  auto *layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel(QString::fromStdString(profile.name)));
  auto *all = new QRadioButton(QString("All %1 nodes: the same profile as the original").arg(profile.size()));
  simplified_ = new QRadioButton;
  tolerance_ = new QDoubleSpinBox;
  tolerance_->setRange(0.01, 10.0);
  tolerance_->setSingleStep(0.05);
  tolerance_->setDecimals(2);
  tolerance_->setValue(DEFAULT_SIMPLIFY_TOLERANCE);
  tolerance_->setSuffix(" m/s");
  all->setChecked(true);
  layout->addWidget(all);
  layout->addWidget(simplified_);
  auto *tolerance_row = new QFormLayout;
  tolerance_row->addRow("Velocity kept within:", tolerance_);
  layout->addLayout(tolerance_row);
  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);

  const auto count = [this, profile] {
    simplified_->setText(QString("Simplified: %1 nodes, easier to edit by hand").arg(mbvt::simplify_profile(profile, tolerance_->value()).size()));
  };
  count();
  connect(tolerance_, &QDoubleSpinBox::valueChanged, this, count);
  connect(tolerance_, &QDoubleSpinBox::valueChanged, this, [this] { simplified_->setChecked(true); });
}

double ActivateDialog::tolerance() const { return simplified_->isChecked() ? tolerance_->value() : 0.0; }

/*--------------------------------------------------------------------*/
VelocityWidget::VelocityWidget(int verbose, int nthreads, int anglemode, const int btime_i[7], const int etime_i[7], bool quiet,
                               Presentation presentation)
    : verbose_(verbose), nthreads_(nthreads), anglemode_(anglemode), quiet_(quiet), presentation_(presentation) {
  std::copy(btime_i, btime_i + 7, btime_i_);
  std::copy(etime_i, etime_i + 7, etime_i_);
  title_ = "mbvelocitytool-qt[*]";

  velocity_plot_ = new mbvt::VelocityPlot;
  residual_plot_ = new mbvt::ResidualPlot;
  raypath_plot_ = new mbvt::RaypathPlot;
  velocity_plot_->setProfiles(nullptr, &display_);
  connect(velocity_plot_, &mbvt::VelocityPlot::edited, this, &VelocityWidget::editChanged);
  connect(velocity_plot_, &mbvt::VelocityPlot::profileAction, this, &VelocityWidget::onProfileAction);
  connect(velocity_plot_, &mbvt::VelocityPlot::showHiddenProfiles, this, &VelocityWidget::showHiddenProfiles);
  connect(velocity_plot_, &mbvt::VelocityPlot::pointer, this, [this](bool inside, double velocity, double depth) {
    pointer_label_->setText(inside ? QString("Velocity %1 m/s, depth %2 m").arg(velocity, 0, 'f', 2).arg(depth, 0, 'f', 1) : QString());
  });

  auto *top = new QSplitter(Qt::Horizontal);
  top->addWidget(velocity_plot_);
  top->addWidget(residual_plot_);
  top->setChildrenCollapsible(false);
  auto *plots = new QSplitter(Qt::Vertical);
  plots->addWidget(top);
  plots->addWidget(raypath_plot_);
  plots->setChildrenCollapsible(false);

  swath_label_ = new QLabel;
  edit_label_ = new QLabel;
  display_label_ = new QLabel;
  auto *labels = new QHBoxLayout;
  for (QLabel *label : {swath_label_, edit_label_, display_label_})
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  labels->addWidget(swath_label_, 3);
  labels->addWidget(edit_label_, 3);
  labels->addWidget(display_label_, 1);
  auto *layout = new QVBoxLayout(this);
  if (presentation_ == Presentation::Panel) {
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
  }
  layout->addLayout(labels);
  layout->addWidget(plots, 1);

  /* a line under the plots, unless the window around the tool moves them to its status bar */
  pointer_label_ = new QLabel(this);
  result_label_ = new QLabel(this);
  result_label_->setContentsMargins(0, 0, 10, 0);
  progress_ = new QProgressBar(this);
  progress_->setRange(0, 0);
  progress_->setMaximumWidth(160);
  progress_->hide();
  auto *status = new QHBoxLayout;
  status->addWidget(pointer_label_, 1);
  status->addWidget(result_label_);
  status->addWidget(progress_);
  layout->addLayout(status);

  scaling_dialog_ = new ScalingDialog(this);
  scaling_dialog_->setScale(scale_);
  connect(scaling_dialog_, &ScalingDialog::changed, this, [this] {
    scale_ = scaling_dialog_->scale();
    if (panel_scales_ != nullptr)
      panel_scales_->setScale(scale_);
    applyScale();
  });

  createMenus();
  if (presentation_ == Presentation::Panel) {
    buildPanelBar(layout);
    setShortcutContext(Qt::WidgetWithChildrenShortcut);
    setFocusProxy(velocity_plot_);
    for (QWidget *plot : std::initializer_list<QWidget *>{velocity_plot_, residual_plot_, raypath_plot_})
      plot->setMinimumSize(160, 100);
  }
  applyScale();
  updateState();

  const QSize size = initialSize();
  plots->setSizes({size.height() / 2, size.height() / 2});
  top->setSizes({size.width() / 2, size.width() / 2});
}

VelocityWidget::~VelocityWidget() {
  /* the thread reads swath_ */
  if (worker_ != nullptr)
    worker_->wait();
}

QSize VelocityWidget::initialSize() {
  QSize size(1016, 668);
  if (const QScreen *screen = QApplication::primaryScreen())
    size = size.boundedTo(screen->availableGeometry().size());
  return size;
}

void VelocityWidget::createMenus() {
  const auto add_menu = [this](const char *title) {
    auto *menu = new QMenu(title, this);
    menus_.append(menu);
    return menu;
  };
  QMenu *file = add_menu("&File");
  open_display_action_ = file->addAction("Open &Display Profile...", this, &VelocityWidget::openDisplayDialog);
  file->addAction("Open &Editable Profile...", this, &VelocityWidget::openEditDialog);
  file->addAction("&New Editable Profile", this, [this] {
    if (offerToSaveEdit())
      newEditProfile();
  });
  save_edit_action_ = file->addAction("&Save Editable Profile...", this, &VelocityWidget::saveEditDialog);
  save_edit_action_->setShortcut(QKeySequence::Save);
  file->addSeparator();
  open_swath_action_ = file->addAction("&Open Swath Sonar Data...", this, [this] { openSwathDialog(false); });
  open_swath_action_->setShortcut(QKeySequence::Open);
  open_swath_format_action_ = file->addAction("Open Swath Sonar Data With &Format...", this, [this] { openSwathDialog(true); });
  save_swath_action_ = file->addAction("Save Swath S&VP File", this, &VelocityWidget::saveSwathSvp);
  save_residuals_action_ = file->addAction("Save &Residuals As Offsets", this, &VelocityWidget::saveResidualsAsOffsets);

  QMenu *view = add_menu("&View");
  scaling_action_ = view->addAction("Plot &Scaling...", this, [this] {
    scaling_dialog_->show();
    scaling_dialog_->raise();
    scaling_dialog_->activateWindow();
  });

  show_hidden_action_ = view->addAction("Show &Hidden Profiles", this, &VelocityWidget::showHiddenProfiles);

  QMenu *processing = add_menu("&Processing");
  reprocess_action_ = processing->addAction("&Reprocess", this, &VelocityWidget::reprocess);
  reprocess_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
  QMenu *angle = processing->addMenu("&Angle Mode");
  angle_group_ = new QActionGroup(this);
  const std::pair<const char *, int> modes[] = {{"Do Not Change Beam Angles", MBP_ANGLES_OK},
                                                {"Adjust Angles Using Snell's Law", MBP_ANGLES_SNELL},
                                                {"Adjust Angles Using Sonar Geometry", MBP_ANGLES_SNELLNULL}};
  for (const auto &mode : modes) {
    QAction *action = angle->addAction(mode.first);
    action->setCheckable(true);
    action->setData(mode.second);
    action->setChecked(mode.second == anglemode_);
    angle_group_->addAction(action);
    const int value = mode.second;
    connect(action, &QAction::triggered, this, [this, value] { setAngleMode(value); });
  }

  QMenu *help = add_menu("&Help");
  QAction *about = help->addAction("&About mbvelocitytool-qt", this, &VelocityWidget::showAbout);
  about->setMenuRole(QAction::AboutRole);
}

void VelocityWidget::buildPanelBar(QBoxLayout *layout) {
  auto *bar = new QToolBar;
  bar->setObjectName("panel_bar");
  bar->setToolButtonStyle(Qt::ToolButtonTextOnly);
  const auto popup_button = [bar](const char *text, const char *name, QMenu *menu) {
    auto *button = new QToolButton;
    button->setText(text);
    button->setObjectName(name);
    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setFocusPolicy(Qt::NoFocus);
    bar->addWidget(button);
  };

  auto *panel_menu = new QMenu(this);
  for (QMenu *menu : menus_)
    panel_menu->addMenu(menu);
  popup_button("Menu", "panel_menu", panel_menu);

  panel_scales_ = new ScalingControls;
  panel_scales_->setObjectName("panel_scales_controls");
  panel_scales_->setContentsMargins(8, 8, 8, 8);
  panel_scales_->setScale(scale_);
  connect(panel_scales_, &ScalingControls::changed, this, [this] {
    scale_ = panel_scales_->scale();
    scaling_dialog_->setScale(scale_);
    applyScale();
  });
  auto *scales_menu = new QMenu(this);
  auto *scales_action = new QWidgetAction(scales_menu);
  scales_action->setDefaultWidget(panel_scales_);
  scales_menu->addAction(scales_action);
  popup_button("Scales", "panel_scales", scales_menu);

  bar->addAction(reprocess_action_);
  layout->insertWidget(0, bar);
}

/* the dialogs are windows of their own: they go and come with the panel */
void VelocityWidget::hideEvent(QHideEvent *event) {
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

void VelocityWidget::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!event->spontaneous()) {
    for (QDialog *dialog : hidden_dialogs_)
      dialog->show();
    hidden_dialogs_.clear();
  }
}

void VelocityWidget::addToolActions(QToolBar *bar) {
  bar->addAction(open_swath_action_);
  bar->addAction(open_display_action_);
  bar->addSeparator();
  bar->addAction(reprocess_action_);
  bar->addAction(scaling_action_);
}

void VelocityWidget::setShortcutContext(Qt::ShortcutContext context) {
  for (QAction *action : {save_edit_action_, open_swath_action_, reprocess_action_}) {
    action->setShortcutContext(context);
    /* with a smaller scope than the window the key works only if the action is on this widget */
    if (context != Qt::WindowShortcut)
      addAction(action);
  }
}

void VelocityWidget::setTitle(const QString &title) {
  title_ = title;
  emit titleChanged(title_);
}

void VelocityWidget::setEditModified(bool modified) {
  const bool changed = modified != edit_modified_;
  edit_modified_ = modified;
  if (changed)
    emit unsavedChanged(modified);
}

/*--------------------------------------------------------------------*/
void VelocityWidget::report(const QString &title, const QString &text) {
  if (quiet_)
    fprintf(stderr, "%s: %s\n", title.toLocal8Bit().constData(), text.toLocal8Bit().constData());
  else
    QMessageBox::warning(this, title, text);
}

void VelocityWidget::runJob(const QString &message, std::function<void()> work, std::function<void()> done) {
  worker_ = QThread::create(std::move(work));
  worker_->setParent(this);
  connect(worker_, &QThread::finished, this, [this, done = std::move(done)] {
    worker_->deleteLater();
    worker_ = nullptr;
    progress_->hide();
    done();
    updateState();
    reportBusy();
  });
  result_label_->setText(message);
  progress_->show();
  worker_->start();
  updateState();
  reportBusy();
}

void VelocityWidget::reportBusy() {
  if (busy() != reported_busy_) {
    reported_busy_ = busy();
    emit busyChanged(reported_busy_);
  }
}

void VelocityWidget::waitUntilIdle() {
  while (worker_ != nullptr)
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
}

/*--------------------------------------------------------------------*/
bool VelocityWidget::openDisplayProfile(const QString &path) {
  mbvt::Profile profile;
  std::string error;
  if (!mbvt::read_profile(path.toStdString(), mbvt::ProfileSource::File, profile, error)) {
    failed_ = true;
    report("Unable to read the display profile", QString::fromStdString(error));
    return false;
  }
  display_.push_back(std::move(profile));
  velocity_plot_->update();
  updateState();
  return true;
}

bool VelocityWidget::openEditProfile(const QString &path) {
  mbvt::Profile profile;
  std::string error;
  if (!mbvt::read_profile(path.toStdString(), mbvt::ProfileSource::File, profile, error)) {
    failed_ = true;
    report("Unable to read the editable profile", QString::fromStdString(error));
    return false;
  }
  edit_ = std::move(profile);
  have_edit_ = true;
  velocity_plot_->setProfiles(&edit_, &display_);
  edit_version_++;
  setEditModified(false);
  updateState();
  emit editProfileChanged();
  return true;
}

void VelocityWidget::newEditProfile() {
  const bool traced = have_residuals_ && residuals_.bath_max > residuals_.bath_min;
  edit_ = mbvt::default_profile(traced ? residuals_.bath_min : swath_.bath_min, traced ? residuals_.bath_max : swath_.bath_max);
  have_edit_ = true;
  velocity_plot_->setProfiles(&edit_, &display_);
  edit_version_++;
  setEditModified(false);
  updateState();
  emit editProfileChanged();
}

void VelocityWidget::editChanged() {
  edit_version_++;
  setEditModified(true);
  updateState();
  emit editProfileChanged();
}

void VelocityWidget::setAngleMode(int anglemode) {
  if (anglemode != anglemode_)
    edit_version_++;
  anglemode_ = anglemode;
  for (QAction *action : angle_group_->actions())
    action->setChecked(action->data().toInt() == anglemode);
  updateState();
}

void VelocityWidget::applyScale() {
  velocity_plot_->setScale(scale_.maxdepth, scale_.velrange, scale_.velcenter);
  residual_plot_->setRange(scale_.resrange);
  raypath_plot_->setMaxDepth(scale_.maxdepth);
}

/*--------------------------------------------------------------------*/
void VelocityWidget::onProfileAction(int index, const QString &name, mbvt::ProfileAction action) {
  /* a new swath file may have replaced the profiles while the menu was open */
  const auto current = [this, index, name] {
    return index >= 0 && index < (int)display_.size() && QString::fromStdString(display_[index].name) == name;
  };
  if (!current())
    return;
  switch (action) {
  case mbvt::ProfileAction::Activate: {
    double tolerance = 0.0;
    if (display_[index].size() > ASK_ABOVE_NODES) {
      ActivateDialog dialog(display_[index], this);
      if (dialog.exec() != QDialog::Accepted || !current())
        return;
      tolerance = dialog.tolerance();
    }
    activateDisplayProfile(index, tolerance);
    break;
  }
  case mbvt::ProfileAction::Hide:
    hideDisplayProfile(index);
    break;
  case mbvt::ProfileAction::Remove:
    if (display_[index].unsaved && !quiet_ &&
        QMessageBox::question(this, "Remove Profile", QString::fromStdString(display_[index].name) + " has edits that are not saved. Remove it?") !=
            QMessageBox::Yes)
      return;
    removeDisplayProfile(index);
    break;
  case mbvt::ProfileAction::Save:
    saveDisplayDialog(index);
    break;
  }
}

bool VelocityWidget::activateDisplayProfile(int index, double tolerance) {
  if (index < 0 || index >= (int)display_.size())
    return false;

  /* the new edit profile is a copy, so the original stays on the plot as the reference while editing */
  mbvt::Profile profile = tolerance > 0.0 ? mbvt::simplify_profile(display_[index], tolerance) : display_[index];
  const bool unsaved = tolerance <= 0.0 && display_[index].unsaved;
  if (tolerance > 0.0)
    profile.name += " simplified";
  profile.hidden = false;
  profile.unsaved = false;
  display_[index].unsaved = false;

  /* the profile it replaces joins the display profiles, unless it is the plain default or already there */
  if (have_edit_) {
    auto same = std::find_if(display_.begin(), display_.end(), [this](const mbvt::Profile &p) { return mbvt::same_nodes(p, edit_, 0.0); });
    if (same != display_.end()) {
      same->unsaved = same->unsaved || edit_modified_;
    }
    else if (edit_.source != mbvt::ProfileSource::Default || edit_modified_) {
      mbvt::Profile previous = std::move(edit_);
      if (edit_modified_ && previous.name.find(" (edited)") == std::string::npos)
        previous.name += " (edited)";
      previous.source = mbvt::ProfileSource::Edited;
      previous.unsaved = edit_modified_;
      display_.push_back(std::move(previous));
    }
  }

  edit_ = std::move(profile);
  have_edit_ = true;
  velocity_plot_->setProfiles(&edit_, &display_);
  edit_version_++;
  setEditModified(unsaved || tolerance > 0.0);
  updateState();
  emit editProfileChanged();
  return true;
}

void VelocityWidget::hideDisplayProfile(int index) {
  if (index < 0 || index >= (int)display_.size())
    return;
  display_[index].hidden = true;
  velocity_plot_->update();
  updateState();
}

void VelocityWidget::removeDisplayProfile(int index) {
  if (index < 0 || index >= (int)display_.size())
    return;
  display_.erase(display_.begin() + index);
  velocity_plot_->update();
  updateState();
}

void VelocityWidget::showHiddenProfiles() {
  for (mbvt::Profile &p : display_)
    p.hidden = false;
  velocity_plot_->update();
  updateState();
}

bool VelocityWidget::saveDisplayDialog(int index) {
  const QString path = QFileDialog::getSaveFileName(this, "Save Profile", QString(), "Sound Velocity Profiles (*.svp);;All Files (*)");
  if (path.isEmpty() || index >= (int)display_.size())
    return false;
  std::string error;
  if (!mbvt::write_profile(path.toStdString(), display_[index], "", error)) {
    report("Unable to save the profile", QString::fromStdString(error));
    return false;
  }
  display_[index].unsaved = false;
  emit statusMessage("Saved the profile to " + path, 5000);
  emit fileSaved(path);
  return true;
}

/*--------------------------------------------------------------------*/
bool VelocityWidget::openSwath(const QString &path, int format) {
  if (worker_ != nullptr)
    return false;
  if (format == 0 && !mbvt::identify_format(path.toStdString(), format)) {
    bool ok = false;
    if (!quiet_)
      format = QInputDialog::getInt(this, "MBIO Format ID", "The format of " + file_name(path) + " is not known from its name.\nMBIO format id:",
                                    0, 0, 999, 1, &ok);
    if (!ok || format == 0) {
      failed_ = true;
      report("Swath data not loaded", "The format of " + path + " is not known.");
      emit swathOpenFailed(path);
      return false;
    }
  }

  struct Load {
    mbvt::SwathFile swath;
    std::string warning, error;
    bool ok = false;
    std::vector<mbvt::Profile> profiles;
    std::string levitus_error;
  };
  auto load = std::make_shared<Load>();
  const std::string file = path.toStdString();
  runJob("Loading " + file_name(path) + "...",
         [this, load, file, format] {
           load->ok = mbvt::load_swath(file, format, btime_i_, etime_i_, verbose_, load->swath, load->warning, load->error);
           if (!load->ok)
             return;
           load->profiles = mbvt::swath_display_profiles(load->swath, load->levitus_error);
         },
         [this, load, path, format] {
           if (!load->ok) {
             failed_ = true;
             updateState();
             report("Data loading aborted", QString::fromStdString(load->error));
             emit swathOpenFailed(path);
             return;
           }

           /* the profiles of the previous swath file go with it; those the user opened stay */
           display_.erase(std::remove_if(display_.begin(), display_.end(),
                                         [](const mbvt::Profile &p) {
                                           return p.source == mbvt::ProfileSource::Levitus || p.source == mbvt::ProfileSource::SwathRecord ||
                                                  p.source == mbvt::ProfileSource::SwathListing;
                                         }),
                          display_.end());
           display_.insert(display_.begin(), std::make_move_iterator(load->profiles.begin()),
                           std::make_move_iterator(load->profiles.end()));

           residual_plot_->setData(nullptr, load->swath.beams_bath);
           raypath_plot_->setData(nullptr);
           have_residuals_ = false;
           swath_ = std::move(load->swath);
           if (!have_edit_)
             newEditProfile();
           velocity_plot_->update();

           /* the plots follow the depth of the new data */
           if (swath_.bath_max > 0.0 && swath_.bath_max < 13000.0) {
             scale_.maxdepth = 1.25 * swath_.bath_max;
             scale_.resrange = std::min(std::max(0.02 * swath_.bath_max, 0.1), 200.0);
             scaling_dialog_->setScale(scale_);
             if (panel_scales_ != nullptr)
               panel_scales_->setScale(scale_);
             applyScale();
           }
           setTitle("mbvelocitytool-qt - " + file_name(swath_.path) + "[*]");
           emit swathOpened(path, format);

           reprocess();

           if (!load->levitus_error.empty())
             fprintf(stderr, "%s\n", load->levitus_error.c_str());
           if (!load->warning.empty())
             report("Data doesn't include travel times", QString::fromStdString(load->warning));
         });
  return true;
}

void VelocityWidget::reprocess() {
  if (worker_ != nullptr || !have_edit_ || swath_.pings.empty())
    return;
  struct Process {
    mbvt::Profile profile;
    mbvt::Residuals residuals;
    std::string error;
    bool ok = false;
    double seconds = 0.0;
    long version = 0;
  };
  auto job = std::make_shared<Process>();
  job->profile = edit_;
  job->version = edit_version_;
  const int anglemode = anglemode_;
  const int nthreads = nthreads_;
  runJob("Reprocessing...",
         [this, job, anglemode, nthreads] {
           const auto start = std::chrono::steady_clock::now();
           job->ok = mbvt::process(swath_, job->profile, anglemode, nthreads, job->residuals, job->error);
           job->seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
         },
         [this, job] {
           if (!job->ok) {
             failed_ = true;
             updateState();
             report("Reprocess failed", QString::fromStdString(job->error));
             return;
           }
           residuals_ = std::move(job->residuals);
           have_residuals_ = true;
           process_seconds_ = job->seconds;
           processed_version_ = job->version;
           residual_plot_->setData(&residuals_, swath_.beams_bath);
           raypath_plot_->setData(&residuals_);
           emit residualsChanged();
         });
}

/*--------------------------------------------------------------------*/
bool VelocityWidget::offerToSaveEdit() {
  if (quiet_ || !have_edit_ || !edit_modified_)
    return true;
  const QMessageBox::StandardButton answer =
      QMessageBox::question(this, "mbvelocitytool-qt", "The editable profile has changes that are not saved. Save them?",
                            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
  if (answer == QMessageBox::Save)
    return saveEditDialog();
  return answer == QMessageBox::Discard;
}

bool VelocityWidget::saveEditDialog() {
  const QString path = QFileDialog::getSaveFileName(this, "Save Editable Profile", QString(), "Sound Velocity Profiles (*.svp);;All Files (*)");
  if (path.isEmpty())
    return false;
  return saveEditProfile(path);
}

bool VelocityWidget::saveEditProfile(const QString &path) {
  std::string error;
  if (!mbvt::write_profile(path.toStdString(), edit_, "", error)) {
    report("Unable to save the editable profile", QString::fromStdString(error));
    return false;
  }
  setEditModified(false);
  emit statusMessage("Saved the editable profile to " + path, 5000);
  emit fileSaved(path);
  return true;
}

void VelocityWidget::openDisplayDialog() {
  const QStringList paths = QFileDialog::getOpenFileNames(this, "Open Display Profile", QString(), "Sound Velocity Profiles (*.svp);;All Files (*)");
  for (const QString &path : paths)
    openDisplayProfile(path);
}

void VelocityWidget::openEditDialog() {
  if (!offerToSaveEdit())
    return;
  const QString path = QFileDialog::getOpenFileName(this, "Open Editable Profile", QString(), "Sound Velocity Profiles (*.svp);;All Files (*)");
  if (!path.isEmpty())
    openEditProfile(path);
}

void VelocityWidget::openSwathDialog(bool ask_format) {
  const QString path = QFileDialog::getOpenFileName(this, "Open Swath Sonar Data");
  if (path.isEmpty())
    return;
  int format = 0;
  if (ask_format) {
    bool ok = false;
    mbvt::identify_format(path.toStdString(), format);
    format = QInputDialog::getInt(this, "MBIO Format ID", "MBIO format id of " + file_name(path) + ":", format, 0, 999, 1, &ok);
    if (!ok)
      return;
  }
  openSwath(path, format);
}

bool VelocityWidget::saveSwathSvp() {
  if (worker_ != nullptr)
    return false;
  std::string saved, error;
  if (!mbvt::save_swath_profile(swath_, edit_, anglemode_, saved, error)) {
    report("Unable to save the swath SVP file", QString::fromStdString(error));
    return false;
  }
  setEditModified(false);
  emit statusMessage("Saved " + QString::fromStdString(saved) + " and set it in the mbprocess parameter file", 5000);
  emit fileSaved(QString::fromStdString(saved));
  return true;
}

bool VelocityWidget::saveResidualsAsOffsets() {
  if (worker_ != nullptr)
    return false;
  if (residualsOutOfDate() && !quiet_ &&
      QMessageBox::question(this, "Save Residuals As Offsets",
                            "The residuals are from before the latest change. Save them anyway?") != QMessageBox::Yes)
    return false;
  std::string saved, error;
  if (!mbvt::save_residuals(swath_, edit_, residuals_, saved, error)) {
    report("Unable to save the residuals", QString::fromStdString(error));
    return false;
  }
  emit statusMessage("Saved " + QString::fromStdString(saved) + " and set it in the mbprocess parameter file", 5000);
  emit fileSaved(QString::fromStdString(swath_.path + ".sbo"));
  emit fileSaved(QString::fromStdString(swath_.path + ".sbao"));
  return true;
}

void VelocityWidget::showAbout() {
  QMessageBox::about(this, "About mbvelocitytool-qt",
                     QString("<b>mbvelocitytool-qt</b><br>Interactive water sound velocity profile editor.<br><br>"
                             "MB-System Release %1<br>%2<br><br>"
                             "A component of the MB-System software package, created by David W. Caress and "
                             "Dale N. Chayes.")
                         .arg(MB_VERSION, MB_VERSION_DATE));
}

void VelocityWidget::updateState() {
  const bool busy = worker_ != nullptr;
  const bool loaded = !swath_.pings.empty();
  show_hidden_action_->setEnabled(std::any_of(display_.begin(), display_.end(), [](const mbvt::Profile &p) { return p.hidden; }));
  open_swath_action_->setEnabled(!busy);
  open_swath_format_action_->setEnabled(!busy);
  save_edit_action_->setEnabled(have_edit_);
  save_swath_action_->setEnabled(have_edit_ && loaded && !busy);
  save_residuals_action_->setEnabled(have_edit_ && have_residuals_ && !busy);
  reprocess_action_->setEnabled(have_edit_ && loaded && !busy);

  if (loaded) {
    swath_label_->setText(QString("Read %1 pings from %2%3")
                              .arg(swath_.pings.size())
                              .arg(file_name(swath_.path))
                              .arg(swath_.truncated ? " (limit reached)" : ""));
    swath_label_->setToolTip(QString::fromStdString(swath_.path));
  }
  else {
    swath_label_->setText("No swath sonar data loaded...");
  }
  if (have_edit_) {
    edit_label_->setText(QString("Editable SVP: %1, %2 nodes").arg(file_name(edit_.name)).arg(edit_.size()));
    edit_label_->setToolTip(QString::fromStdString(edit_.name));
  }
  else {
    edit_label_->setText("No editable SVP loaded...");
  }
  if (display_.empty()) {
    display_label_->setText("No display SVPs loaded...");
  }
  else {
    QStringList names;
    int hidden = 0;
    for (const mbvt::Profile &p : display_) {
      names << QString::fromStdString(p.name) + (p.hidden ? " (hidden)" : "");
      hidden += p.hidden ? 1 : 0;
    }
    display_label_->setText(QString("%1 display SVP%2%3")
                                .arg(display_.size())
                                .arg(display_.size() == 1 ? "" : "s")
                                .arg(hidden > 0 ? QString(", %1 hidden").arg(hidden) : QString()));
    display_label_->setToolTip(names.join("\n"));
  }

  if (busy)
    return;
  QString result;
  if (have_residuals_) {
    if (residualsOutOfDate()) {
      result = "Residuals out of date: press Reprocess";
    }
    else {
      result = QString("Reprocess: %1 s").arg(process_seconds_, 0, 'f', 2);
      if (residuals_.rays_failed > 0)
        result += QString(", %1 rays failed").arg(residuals_.rays_failed);
      if (residuals_.rays_at_bottom > 0)
        result += QString(", %1 rays stopped at the bottom of the profile").arg(residuals_.rays_at_bottom);
    }
  }
  result_label_->setText(result);
}

bool VelocityWidget::prepareToClose() {
  if (!offerToSaveEdit())
    return false;
  const long unsaved = std::count_if(display_.begin(), display_.end(), [](const mbvt::Profile &p) { return p.unsaved; });
  if (!quiet_ && unsaved > 0 &&
      QMessageBox::question(this, "mbvelocitytool-qt",
                            QString("%1 edited display profile%2 not saved: right-click to save. Quit anyway?").arg(unsaved).arg(unsaved == 1 ? " is" : "s are")) !=
          QMessageBox::Yes)
    return false;
  return true;
}

}  // namespace mbvt
