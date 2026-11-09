/*--------------------------------------------------------------------
 *    The MB-system:  wcd_widget.cc  1/11/2026
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

#include "wcd_widget.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QShowEvent>
#include <QTableWidget>
#include <QTextStream>
#include <QThread>
#include <QTimeZone>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidgetAction>

#include "mb_define.h"
#include "WcdView.h"

namespace wcd {

namespace {

QString utc_text(double time_d) {
  return QDateTime::fromMSecsSinceEpoch((qint64)std::llround(time_d * 1000.0), QTimeZone::utc())
             .toString("yyyy-MM-dd HH:mm:ss.zzz") + " UTC";
}

QString number_or_empty(double v, int decimals) { return std::isfinite(v) ? QString::number(v, 'f', decimals) : ""; }

QDoubleSpinBox *spin_box(double lo, double hi, double step, int decimals) {
  auto *box = new QDoubleSpinBox;
  box->setRange(lo, hi);
  box->setSingleStep(step);
  box->setDecimals(decimals);
  return box;
}

}  // namespace

/*--------------------------------------------------------------------*/
void OneStepWheelSlider::wheelEvent(QWheelEvent *event) {
  const int delta = event->angleDelta().y();
  if (delta > 0 && value() < maximum())
    setValue(value() + 1);
  else if (delta < 0 && value() > minimum())
    setValue(value() - 1);
  event->accept();
}

/*--------------------------------------------------------------------*/
const QStringList PickDialog::columns = {"Ping", "UTC", "AcrossTrack_m", "Depth_m", "ShipLat",
                                         "ShipLon", "PointLat", "PointLon", "Azimuth"};

PickDialog::PickDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Picked Positions");
  table_ = new QTableWidget(0, columns.size(), this);
  table_->setHorizontalHeaderLabels(columns);
  table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto *save = new QPushButton("Save as TXT");
  auto *remove = new QPushButton("Remove selected");
  auto *close = new QPushButton("Close");
  connect(save, &QPushButton::clicked, this, &PickDialog::saveDialog);
  connect(remove, &QPushButton::clicked, this, &PickDialog::removeSelected);
  connect(close, &QPushButton::clicked, this, &PickDialog::clearAndClose);
  auto *buttons = new QHBoxLayout;
  buttons->addWidget(save);
  buttons->addWidget(remove);
  buttons->addStretch(1);
  buttons->addWidget(close);
  auto *layout = new QVBoxLayout(this);
  layout->addWidget(table_);
  layout->addLayout(buttons);
}

void PickDialog::addRow(const QStringList &values) {
  const int r = table_->rowCount();
  table_->insertRow(r);
  for (int c = 0; c < values.size() && c < columns.size(); c++)
    table_->setItem(r, c, new QTableWidgetItem(values[c]));
}

bool PickDialog::saveText(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;
  QTextStream out(&file);
  out << columns.join("\t") << "\n";
  for (int r = 0; r < table_->rowCount(); r++) {
    QStringList values;
    for (int c = 0; c < table_->columnCount(); c++)
      values << (table_->item(r, c) != nullptr ? table_->item(r, c)->text() : QString());
    out << values.join("\t") << "\n";
  }
  return true;
}

void PickDialog::saveDialog() {
  const QString path =
      QFileDialog::getSaveFileName(this, "Save Picks", "picks.txt", "Text Files (*.txt);;All Files (*)");
  if (!path.isEmpty())
    saveText(path);
}

void PickDialog::removeSelected() {
  QList<int> rows;
  for (const QModelIndex &index : table_->selectionModel()->selectedRows())
    rows << index.row();
  std::sort(rows.begin(), rows.end(), std::greater<int>());
  for (const int r : rows)
    table_->removeRow(r);
}

void PickDialog::clearAndClose() {
  table_->setRowCount(0);
  close();
}

/*--------------------------------------------------------------------*/
WaterColumnWidget::WaterColumnWidget(Presentation presentation, QWidget *parent) : QWidget(parent), presentation_(presentation) {
  setFocusPolicy(Qt::StrongFocus);

  /* the controls are disabled while a file loads, but not the pick table, which is a child of this widget */
  central_ = new QWidget;
  auto *outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->addWidget(central_);
  auto *layout = new QVBoxLayout(central_);

  /* header: a row, or the tool bar of the panel */
  const bool panel = presentation_ == Presentation::Panel;
  QToolBar *bar = panel ? new QToolBar : nullptr;
  QHBoxLayout *header = panel ? nullptr : new QHBoxLayout;
  const auto to_header = [bar, header](QWidget *widget) {
    if (bar != nullptr) {
      widget->setFocusPolicy(Qt::NoFocus);
      bar->addWidget(widget);
    }
    else {
      header->addWidget(widget);
    }
  };
  if (bar != nullptr) {
    bar->setObjectName("panel_bar");
    bar->setToolButtonStyle(Qt::ToolButtonTextOnly);
  }
  auto *open = new QPushButton(panel ? "Open" : "Open .kmwcd/.kmall/.wcd/.all");
  connect(open, &QPushButton::clicked, this, &WaterColumnWidget::openDialog);
  to_header(open);
  file_name_ = new QLabel("No file loaded");
  file_name_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  if (!panel)
    header->addWidget(file_name_);
  auto *save = new QPushButton("Save Image");
  connect(save, &QPushButton::clicked, this, &WaterColumnWidget::saveImageDialog);
  to_header(save);
  QAction *scales_place = panel ? bar->addSeparator() : nullptr;
  bottom_ = new QCheckBox("Bottom");
  wc_bottom_ = new QCheckBox("WC bottom");
  raytrace_ = new QCheckBox(panel ? "Ray trace" : "Ray trace (SVP)");
  raytrace_->setToolTip("Trace the water column samples through the sound velocity profile instead of "
                        "straight rays with the sound speed at the transducer");
  for (QCheckBox *box : {bottom_, wc_bottom_, raytrace_}) {
    connect(box, &QCheckBox::toggled, this, &WaterColumnWidget::redraw);
    to_header(box);
  }
  pick_ = new QCheckBox("Pick");
  connect(pick_, &QCheckBox::clicked, this, &WaterColumnWidget::showPickDialog);
  connect(pick_, &QCheckBox::toggled, this, &WaterColumnWidget::togglePick);
  to_header(pick_);
  meta_ = new QLabel;
  if (panel) {
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
    layout->addWidget(bar);
  }
  else {
    meta_->setWordWrap(true);
    header->addWidget(meta_);
    layout->addLayout(header);
  }

  /* depth, amplitude, across-track and replay controls */
  dmin_ = spin_box(0, 1.0e5, 1.0, 2);
  dmax_ = spin_box(0, 1.0e5, 1.0, 2);
  amin_ = spin_box(-200, 200, 0.5, 2);
  amax_ = spin_box(-200, 200, 0.5, 2);
  xmax_ = spin_box(0, 1.0e6, 10.0, 2);
  replay_rate_ = spin_box(0.1, 10.0, 0.1, 1);
  replay_rate_->setValue(2.0);
  replay_ = new QPushButton("Replay");
  replay_->setCheckable(true);
  if (panel) {
    /* the numbers are in a popup, so that the plot keeps the space */
    auto *scales = new QWidget;
    scales->setObjectName("panel_scales_controls");
    auto *form = new QFormLayout(scales);
    form->addRow("Depth Min (m):", dmin_);
    form->addRow("Depth Max (m):", dmax_);
    form->addRow("Amp Min (dB):", amin_);
    form->addRow("Amp Max (dB):", amax_);
    form->addRow("Across-track Max (m):", xmax_);
    form->addRow("Replay (pings/s):", replay_rate_);
    auto *scales_menu = new QMenu(this);
    auto *scales_action = new QWidgetAction(scales_menu);
    scales_action->setDefaultWidget(scales);
    scales_menu->addAction(scales_action);
    auto *scales_button = new QToolButton;
    scales_button->setText("Scales");
    scales_button->setObjectName("panel_scales");
    scales_button->setMenu(scales_menu);
    scales_button->setPopupMode(QToolButton::InstantPopup);
    scales_button->setFocusPolicy(Qt::NoFocus);
    bar->insertWidget(scales_place, scales_button);
    to_header(replay_);
  }
  else {
    auto *depth_row = new QHBoxLayout;
    depth_row->addWidget(new QLabel("Depth Min (m):"));
    depth_row->addWidget(dmin_);
    depth_row->addWidget(new QLabel("Depth Max (m):"));
    depth_row->addWidget(dmax_);
    layout->addLayout(depth_row);
    auto *amp_row = new QHBoxLayout;
    amp_row->addWidget(new QLabel("Amp Min (dB):"));
    amp_row->addWidget(amin_);
    amp_row->addWidget(new QLabel("Amp Max (dB):"));
    amp_row->addWidget(amax_);
    layout->addLayout(amp_row);
    auto *x_row = new QHBoxLayout;
    x_row->addWidget(new QLabel("Across-track Max (m):"));
    x_row->addWidget(xmax_);
    x_row->addWidget(new QLabel("Replay (pings/s):"));
    x_row->addWidget(replay_rate_);
    x_row->addWidget(replay_);
    layout->addLayout(x_row);
  }
  for (QDoubleSpinBox *box : {dmin_, dmax_, amin_, amax_, xmax_})
    connect(box, &QDoubleSpinBox::valueChanged, this, &WaterColumnWidget::updatePlotSettings);
  connect(replay_, &QPushButton::toggled, this, &WaterColumnWidget::toggleReplay);
  connect(replay_rate_, &QDoubleSpinBox::valueChanged, this, &WaterColumnWidget::updateReplayInterval);
  replay_timer_ = new QTimer(this);
  connect(replay_timer_, &QTimer::timeout, this, &WaterColumnWidget::replayTick);

  /* plot */
  view_ = new WcdView;
  connect(view_, &WcdView::clicked, this, &WaterColumnWidget::onClicked);
  layout->addWidget(view_, 1);

  /* ping slider */
  auto *slider_row = new QHBoxLayout;
  slider_row->addWidget(new QLabel("Ping:"));
  slider_ = new OneStepWheelSlider(Qt::Horizontal);
  connect(slider_, &QSlider::valueChanged, this, &WaterColumnWidget::onSlider);
  slider_row->addWidget(slider_);
  step_label_ = new QLabel("0");
  slider_row->addWidget(step_label_);
  layout->addLayout(slider_row);

  if (panel) {
    /* one line for the ping and for the messages of the tool; the host can show the messages too */
    file_name_->setParent(central_);
    file_name_->hide();
    auto *status = new QLabel;
    status->setObjectName("panel_status");
    for (QLabel *label : {meta_, status})
      label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    status->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto *info = new QHBoxLayout;
    info->addWidget(meta_, 3);
    info->addWidget(status, 2);
    layout->addLayout(info);
    connect(this, &WaterColumnWidget::statusMessage, status, [status](const QString &message, int timeout_ms) {
      status->setText(message);
      status->setToolTip(message);
      if (timeout_ms > 0)
        QTimer::singleShot(timeout_ms, status, [status, message] {
          if (status->text() == message)
            status->clear();
        });
    });
    view_->setMinimumSize(200, 150);
    view_->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(view_);
  }
}

WaterColumnWidget::~WaterColumnWidget() {
  /* the thread writes to data held by its lambda and posts to this object */
  if (worker_ != nullptr)
    worker_->wait();
}

void WaterColumnWidget::waitUntilIdle() {
  while (worker_ != nullptr)
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
}

QString WaterColumnWidget::title() const { return "mbwcdview - water column through MB-System MBIO"; }

void WaterColumnWidget::showStatus(const QString &message, int timeout_ms) { emit statusMessage(message, timeout_ms); }

/*--------------------------------------------------------------------*/
void WaterColumnWidget::openDialog() {
  const QString path = QFileDialog::getOpenFileName(this, "Open .kmwcd/.kmall/.wcd/.all", "", "*.kmwcd *.kmall *.wcd *.all");
  if (!path.isEmpty())
    openFile(path);
}

bool WaterColumnWidget::openFile(const QString &path) {
  if (worker_ != nullptr)
    return false;
  replay_->setChecked(false);
  file_label_ = QFileInfo(path).fileName();
  file_name_->setText(file_label_);
  showStatus("Reading survey file (navigation, bathymetry)...");
  line_ = wcd::Line();
  steps_.clear();
  reported_step_ = -1;

  struct Load {
    wcd::Line line;
    std::string message;
    bool ok = false;
  };
  auto load = std::make_shared<Load>();
  const std::string file = path.toStdString();
  worker_ = QThread::create([this, load, file] {
    int last = -1;
    const wcd::Progress progress = [this, &last](int npings, double fraction) {
      const int pct = (int)(100.0 * fraction);
      if (pct != last) {
        last = pct;
        QMetaObject::invokeMethod(
            this, [this, pct, npings] { showStatus(QString("Loading pings... %1% (%2 pings)").arg(pct).arg(npings)); }, Qt::QueuedConnection);
      }
    };
    load->ok = load->line.load(file, progress, &load->message);
  });
  worker_->setParent(this);
  connect(worker_, &QThread::finished, this, [this, load, path] {
    worker_->deleteLater();
    worker_ = nullptr;
    central_->setEnabled(true);
    /* disabling the controls took the focus away, and the keys need it */
    if (job_had_focus_ || QApplication::focusWidget() == nullptr)
      setFocus();
    finishOpen(path, std::move(load->line), load->ok, load->message);
    emit busyChanged(false);
  });
  const QWidget *focus = QApplication::focusWidget();
  job_had_focus_ = focus != nullptr && (focus == this || isAncestorOf(focus));
  central_->setEnabled(false);
  worker_->start();
  emit busyChanged(true);
  return true;
}

void WaterColumnWidget::finishOpen(const QString &path, Line line, bool ok, const std::string &message) {
  line_ = std::move(line);
  const int ping = pending_ping_;
  pending_ping_ = -1;
  if (!ok || !line_.has_wc) {
    showStatus(line_.has_survey ? "No water column data found."
                                : QString("Cannot read %1: %2").arg(file_label_, QString::fromStdString(message)));
    view_->plot().setData(wcd::PlotData());
    view_->update();
    emit fileOpenFailed(path);
    return;
  }
  emit diagnostic(QString("survey=%1 water column=%2").arg(QString::fromStdString(line_.kmall), QString::fromStdString(line_.kmwcd)));
  for (size_t i = 0; i < line_.wc.size(); i++)
    for (const int s : line_.wc.swaths(i))
      steps_.emplace_back((int)i, s);

  /* file wide ranges */
  const wcd::WaterColumnFile &wc = line_.wc;
  for (QDoubleSpinBox *box : {amin_, amax_, dmin_, dmax_, xmax_})
    box->blockSignals(true);
  amin_->setRange(wc.amp_min, wc.amp_max);
  amax_->setRange(wc.amp_min, wc.amp_max);
  amin_->setValue(wc.amp_min);
  amax_->setValue(wc.amp_max);
  double draft_max = 0.0;
  for (const double d : line_.survey.sensordepth)
    draft_max = std::max(draft_max, d);
  const double depth_max = wc.range_max_m + draft_max;
  dmin_->setRange(0, depth_max);
  dmax_->setRange(0, depth_max);
  dmin_->setValue(0);
  dmax_->setValue(depth_max);
  xmax_->setRange(0, wc.range_max_m > 0 ? 2.0 * wc.range_max_m : 1.0e5);
  xmax_->setValue(0);
  for (QDoubleSpinBox *box : {amin_, amax_, dmin_, dmax_, xmax_})
    box->blockSignals(false);
  const bool has_svp = line_.svp(0.0) != nullptr;
  raytrace_->blockSignals(true);
  raytrace_->setEnabled(has_svp);
  raytrace_->setChecked(has_svp);
  raytrace_->blockSignals(false);

  slider_->blockSignals(true);
  slider_->setRange(0, (int)steps_.size() - 1);
  slider_->setValue(0);
  slider_->blockSignals(false);
  current_ = 0;
  updatePlotSettings();
  emit fileOpened(path);
  redraw();

  const QString summary =
      QString("Nav summary: survey pings=%1, position records=%2%3, WC pings=%4, matched=%5, fill value=%6")
          .arg(line_.has_survey ? (int)line_.survey.size() : 0)
          .arg(line_.has_survey ? (int)line_.survey.nav.size() : (int)wc.nav.size())
          .arg(line_.has_survey ? "" : " (from water column file)")
          .arg((int)wc.size())
          .arg(line_.matched())
          .arg(wc.has_fill ? QString::number(wc.fill_value, 'f', 1) : QString("none"));
  emit diagnostic(summary);
  QTimer::singleShot(900, this, [this, summary]() { showStatus(summary, 6000); });
  if (ping >= 0)
    showPing(ping);
}

/*--------------------------------------------------------------------*/
void WaterColumnWidget::updatePlotSettings() {
  wcd::WcdPlot &plot = view_->plot();
  plot.setColorRange(amin_->value(), amax_->value());
  plot.setDepthWindow(dmin_->value(), dmax_->value());
  plot.setAcrossMax(xmax_->value());
  view_->update();
}

void WaterColumnWidget::redraw() {
  if (steps_.empty())
    return;
  current_ = std::min(std::max(current_, 0), (int)steps_.size() - 1);
  const auto [ping, swath] = steps_[current_];
  wcd::SceneOptions options;
  options.raytrace = raytrace_->isChecked();
  options.bottom = bottom_->isChecked();
  options.wc_bottom = wc_bottom_->isChecked();
  wcd::PlotData data;
  wcd::SceneInfo info;
  wcd::build_scene(line_, ping, swath, options, data, info);
  view_->plot().setData(std::move(data));
  view_->update();
  meta_->setText(wcd::metadata_text(line_, ping, swath));
  if (presentation_ == Presentation::Panel)
    meta_->setToolTip(file_label_ + "\n" + meta_->text());
  step_label_->setText(QString("%1 / %2").arg(current_).arg((int)steps_.size() - 1));
  if (!info.status.isEmpty())
    showStatus(info.status);
  if (current_ != reported_step_) {
    reported_step_ = current_;
    emit pingChanged(current_, ping, line_.wc.pings[ping].ping_number, swath, line_.wc.pings[ping].time_d);
  }
}

void WaterColumnWidget::setStep(int step) { slider_->setValue(step); }

void WaterColumnWidget::showPing(int ping) {
  if (worker_ != nullptr) {
    pending_ping_ = ping;
    return;
  }
  for (size_t k = 0; k < steps_.size(); k++) {
    if (steps_[k].first >= ping) {
      slider_->setValue((int)k);
      return;
    }
  }
}

bool WaterColumnWidget::showTime(double time_d) {
  if (steps_.empty())
    return false;
  const auto time_of = [this](size_t k) { return line_.wc.pings[steps_[k].first].time_d; };
  if (time_d < time_of(0) || time_d > time_of(steps_.size() - 1))
    return false;
  size_t best = 0;
  for (size_t k = 0; k < steps_.size(); k++)
    if (std::abs(time_of(k) - time_d) < std::abs(time_of(best) - time_d))
      best = k;
  slider_->setValue((int)best);
  return true;
}

void WaterColumnWidget::onSlider(int value) {
  current_ = value;
  redraw();
}

void WaterColumnWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Left && current_ > 0)
    slider_->setValue(current_ - 1);
  else if (event->key() == Qt::Key_Right && current_ < slider_->maximum())
    slider_->setValue(current_ + 1);
  else
    QWidget::keyPressEvent(event);
}

/* the pick table is a window of its own: it goes and comes with the panel, and a hidden panel does not replay */
void WaterColumnWidget::hideEvent(QHideEvent *event) {
  if (presentation_ == Presentation::Panel && !event->spontaneous()) {
    replay_->setChecked(false);
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

void WaterColumnWidget::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  if (!event->spontaneous()) {
    for (QDialog *dialog : hidden_dialogs_)
      dialog->show();
    hidden_dialogs_.clear();
  }
}

/*--------------------------------------------------------------------*/
void WaterColumnWidget::toggleReplay(bool on) {
  if (on && !steps_.empty()) {
    updateReplayInterval();
    replay_timer_->start();
  }
  else {
    replay_timer_->stop();
    replay_->setText("Replay");
  }
}

void WaterColumnWidget::updateReplayInterval() {
  const double pps = std::max(replay_rate_->value(), 0.1);
  replay_timer_->setInterval(std::max(1, (int)(1000.0 / pps)));
  if (replay_timer_->isActive() || replay_->isChecked())
    replay_->setText(QString("Stop (%1 p/s)").arg(pps, 0, 'f', 1));
}

void WaterColumnWidget::replayTick() {
  if (steps_.empty()) {
    replay_->setChecked(false);
    return;
  }
  slider_->setValue((current_ + 1) % (int)steps_.size());
}

/*--------------------------------------------------------------------*/
bool WaterColumnWidget::saveImage(const QString &path) {
  if (steps_.empty())
    return false;
  const QImage image = view_->plot().render(view_->size(), 2.0, file_label_ + " | " + meta_->text());
  return image.save(path);
}

void WaterColumnWidget::saveImageDialog() {
  const QString path = QFileDialog::getSaveFileName(this, "Save Image", file_label_, "PNG (*.png);;JPEG (*.jpg)");
  if (!path.isEmpty() && !saveImage(path))
    showStatus("Cannot write " + path, 5000);
}

/*--------------------------------------------------------------------*/
void WaterColumnWidget::togglePick(bool on) { view_->setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor); }

void WaterColumnWidget::showPickDialog() {
  if (pick_dialog_ == nullptr) {
    pick_dialog_ = new PickDialog(this);
    connect(pick_dialog_, &QDialog::finished, this, [this]() { pick_->setChecked(false); });
    const QWidget *beside = presentation_ == Presentation::Panel ? static_cast<const QWidget *>(this) : window();
    pick_dialog_->resize(std::max(600, beside->width()), std::max(240, beside->height() / 3));
    pick_dialog_->setMinimumSize(600, 240);
  }
  pick_dialog_->show();
  if (!pick_shown_once_) {
    pick_dialog_->raise();
    pick_dialog_->activateWindow();
    pick_shown_once_ = true;
  }
}

void WaterColumnWidget::onClicked(double across, double depth) {
  if (pick_->isChecked())
    pick(across, depth);
}

QStringList WaterColumnWidget::pick(double across, double depth) {
  const wcd::PlotData &d = view_->plot().data();
  size_t best = d.across.size();
  double best_d2 = std::numeric_limits<double>::max();
  for (size_t i = 0; i < d.across.size(); i++) {
    if (!std::isfinite(d.amplitude[i]) || !std::isfinite(d.across[i]) || !std::isfinite(d.depth[i]) ||
        d.depth[i] < dmin_->value() || d.depth[i] > dmax_->value())
      continue;
    const double d2 = (d.across[i] - across) * (d.across[i] - across) + (d.depth[i] - depth) * (d.depth[i] - depth);
    if (d2 < best_d2) {
      best_d2 = d2;
      best = i;
    }
  }
  if (best == d.across.size() || steps_.empty()) {
    showStatus("Nothing to pick (no grid).", 3000);
    return QStringList();
  }
  const auto [ping, swath] = steps_[current_];
  const double time_d = line_.wc.pings[ping].time_d;
  const double x = d.across[best], z = d.depth[best];
  double lon, lat, sog, cog;
  line_.nav_at(time_d, lon, lat, sog, cog);
  const double heading = line_.heading_at(time_d);
  double azimuth = std::numeric_limits<double>::quiet_NaN();
  QString source = "None";
  if (std::isfinite(heading)) {
    azimuth = heading;
    source = "Heading";
  }
  else if (std::isfinite(cog)) {
    azimuth = cog;
    source = "COG";
  }
  double pt_lat = std::numeric_limits<double>::quiet_NaN(), pt_lon = pt_lat;
  if (std::isfinite(lat) && std::isfinite(lon)) {
    /* the across-track offset is to starboard of the azimuth */
    const double east = std::isfinite(azimuth) ? x * std::cos(DTR * azimuth) : x;
    const double north = std::isfinite(azimuth) ? -x * std::sin(DTR * azimuth) : 0.0;
    pt_lat = lat + north / 111320.0;
    pt_lon = lon + east / (111320.0 * std::max(1.0e-6, std::cos(DTR * lat)));
  }
  const QStringList row = {QString("%1 (%2/%3)").arg(current_).arg(line_.wc.pings[ping].ping_number).arg(swath),
                           utc_text(time_d),
                           QString::number(x, 'f', 2),
                           QString::number(z, 'f', 2),
                           number_or_empty(lat, 6),
                           number_or_empty(lon, 6),
                           number_or_empty(pt_lat, 6),
                           number_or_empty(pt_lon, 6),
                           source};
  showPickDialog();
  pick_dialog_->addRow(row);
  emit pickAdded(row);
  return row;
}

}  // namespace wcd
