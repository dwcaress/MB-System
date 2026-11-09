/*--------------------------------------------------------------------
 *    The MB-system:  MainWindow.cc  1/11/2026
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

#include "MainWindow.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextStream>
#include <QTimeZone>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "mb_define.h"
#include "WcdView.h"

namespace wcd {

namespace {

QString utc_text(double time_d) {
  return QDateTime::fromMSecsSinceEpoch((qint64)std::llround(time_d * 1000.0), QTimeZone::UTC)
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
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("mbwcdview - water column through MB-System MBIO");
  setFocusPolicy(Qt::StrongFocus);
  auto *central = new QWidget;
  setCentralWidget(central);
  auto *layout = new QVBoxLayout(central);

  /* header */
  auto *header = new QHBoxLayout;
  auto *open = new QPushButton("Open .kmwcd/.kmall/.wcd/.all");
  connect(open, &QPushButton::clicked, this, &MainWindow::openDialog);
  header->addWidget(open);
  file_name_ = new QLabel("No file loaded");
  file_name_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  header->addWidget(file_name_);
  auto *save = new QPushButton("Save Image");
  connect(save, &QPushButton::clicked, this, &MainWindow::saveImageDialog);
  header->addWidget(save);
  bottom_ = new QCheckBox("Bottom");
  wc_bottom_ = new QCheckBox("WC bottom");
  raytrace_ = new QCheckBox("Ray trace (SVP)");
  raytrace_->setToolTip("Trace the water column samples through the sound velocity profile instead of "
                        "straight rays with the sound speed at the transducer");
  for (QCheckBox *box : {bottom_, wc_bottom_, raytrace_}) {
    connect(box, &QCheckBox::toggled, this, &MainWindow::redraw);
    header->addWidget(box);
  }
  pick_ = new QCheckBox("Pick");
  connect(pick_, &QCheckBox::clicked, this, &MainWindow::showPickDialog);
  connect(pick_, &QCheckBox::toggled, this, &MainWindow::togglePick);
  header->addWidget(pick_);
  meta_ = new QLabel;
  meta_->setWordWrap(true);
  header->addWidget(meta_);
  layout->addLayout(header);

  /* depth, amplitude, across-track and replay controls */
  dmin_ = spin_box(0, 1.0e5, 1.0, 2);
  dmax_ = spin_box(0, 1.0e5, 1.0, 2);
  amin_ = spin_box(-200, 200, 0.5, 2);
  amax_ = spin_box(-200, 200, 0.5, 2);
  xmax_ = spin_box(0, 1.0e6, 10.0, 2);
  replay_rate_ = spin_box(0.1, 10.0, 0.1, 1);
  replay_rate_->setValue(2.0);
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
  replay_ = new QPushButton("Replay");
  replay_->setCheckable(true);
  x_row->addWidget(replay_);
  layout->addLayout(x_row);
  for (QDoubleSpinBox *box : {dmin_, dmax_, amin_, amax_, xmax_})
    connect(box, &QDoubleSpinBox::valueChanged, this, &MainWindow::updatePlotSettings);
  connect(replay_, &QPushButton::toggled, this, &MainWindow::toggleReplay);
  connect(replay_rate_, &QDoubleSpinBox::valueChanged, this, &MainWindow::updateReplayInterval);
  replay_timer_ = new QTimer(this);
  connect(replay_timer_, &QTimer::timeout, this, &MainWindow::replayTick);

  /* plot */
  view_ = new WcdView;
  connect(view_, &WcdView::clicked, this, &MainWindow::onClicked);
  layout->addWidget(view_, 1);

  /* ping slider */
  auto *slider_row = new QHBoxLayout;
  slider_row->addWidget(new QLabel("Ping:"));
  slider_ = new OneStepWheelSlider(Qt::Horizontal);
  connect(slider_, &QSlider::valueChanged, this, &MainWindow::onSlider);
  slider_row->addWidget(slider_);
  step_label_ = new QLabel("0");
  slider_row->addWidget(step_label_);
  layout->addLayout(slider_row);

  statusBar()->showMessage("Ready");
  resize(1400, 900);
}

/*--------------------------------------------------------------------*/
void MainWindow::openDialog() {
  const QString path = QFileDialog::getOpenFileName(this, "Open .kmwcd/.kmall/.wcd/.all", "", "*.kmwcd *.kmall *.wcd *.all");
  if (!path.isEmpty())
    openFile(path);
}

bool MainWindow::openFile(const QString &path) {
  replay_->setChecked(false);
  file_label_ = QFileInfo(path).fileName();
  file_name_->setText(file_label_);
  statusBar()->showMessage("Reading survey file (navigation, bathymetry)...");
  QApplication::processEvents();
  int last = -1;
  auto progress = [&](int npings, double fraction) {
    const int pct = (int)(100.0 * fraction);
    if (pct != last) {
      last = pct;
      statusBar()->showMessage(QString("Loading pings... %1% (%2 pings)").arg(pct).arg(npings));
      QApplication::processEvents();
    }
  };
  std::string message;
  line_ = wcd::Line();
  steps_.clear();
  if (!line_.load(path.toStdString(), progress, &message) || !line_.has_wc) {
    statusBar()->showMessage(line_.has_survey ? "No water column data found."
                                              : QString("Cannot read %1: %2").arg(file_label_,
                                                                                    QString::fromStdString(message)));
    view_->plot().setData(wcd::PlotData());
    view_->update();
    return false;
  }
  fprintf(stderr, "[diag] survey=%s water column=%s\n", line_.kmall.c_str(), line_.kmwcd.c_str());
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
  redraw();

  const QString summary =
      QString("Nav summary: survey pings=%1, position records=%2%3, WC pings=%4, matched=%5, fill value=%6")
          .arg(line_.has_survey ? (int)line_.survey.size() : 0)
          .arg(line_.has_survey ? (int)line_.survey.nav.size() : (int)wc.nav.size())
          .arg(line_.has_survey ? "" : " (from water column file)")
          .arg((int)wc.size())
          .arg(line_.matched())
          .arg(wc.has_fill ? QString::number(wc.fill_value, 'f', 1) : QString("none"));
  fprintf(stderr, "[diag] %s\n", summary.toStdString().c_str());
  QTimer::singleShot(900, this, [this, summary]() { statusBar()->showMessage(summary, 6000); });
  return true;
}

/*--------------------------------------------------------------------*/
void MainWindow::updatePlotSettings() {
  wcd::WcdPlot &plot = view_->plot();
  plot.setColorRange(amin_->value(), amax_->value());
  plot.setDepthWindow(dmin_->value(), dmax_->value());
  plot.setAcrossMax(xmax_->value());
  view_->update();
}

void MainWindow::redraw() {
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
  step_label_->setText(QString("%1 / %2").arg(current_).arg((int)steps_.size() - 1));
  if (!info.status.isEmpty())
    statusBar()->showMessage(info.status);
}

void MainWindow::setStep(int step) { slider_->setValue(step); }

void MainWindow::showPing(int ping) {
  for (size_t k = 0; k < steps_.size(); k++) {
    if (steps_[k].first >= ping) {
      slider_->setValue((int)k);
      return;
    }
  }
}

void MainWindow::onSlider(int value) {
  current_ = value;
  redraw();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Left && current_ > 0)
    slider_->setValue(current_ - 1);
  else if (event->key() == Qt::Key_Right && current_ < slider_->maximum())
    slider_->setValue(current_ + 1);
  else
    QMainWindow::keyPressEvent(event);
}

/*--------------------------------------------------------------------*/
void MainWindow::toggleReplay(bool on) {
  if (on && !steps_.empty()) {
    updateReplayInterval();
    replay_timer_->start();
  }
  else {
    replay_timer_->stop();
    replay_->setText("Replay");
  }
}

void MainWindow::updateReplayInterval() {
  const double pps = std::max(replay_rate_->value(), 0.1);
  replay_timer_->setInterval(std::max(1, (int)(1000.0 / pps)));
  if (replay_timer_->isActive() || replay_->isChecked())
    replay_->setText(QString("Stop (%1 p/s)").arg(pps, 0, 'f', 1));
}

void MainWindow::replayTick() {
  if (steps_.empty()) {
    replay_->setChecked(false);
    return;
  }
  slider_->setValue((current_ + 1) % (int)steps_.size());
}

/*--------------------------------------------------------------------*/
bool MainWindow::saveImage(const QString &path) {
  if (steps_.empty())
    return false;
  const QImage image = view_->plot().render(view_->size(), 2.0, file_label_ + " | " + meta_->text());
  return image.save(path);
}

void MainWindow::saveImageDialog() {
  const QString path = QFileDialog::getSaveFileName(this, "Save Image", file_label_, "PNG (*.png);;JPEG (*.jpg)");
  if (!path.isEmpty() && !saveImage(path))
    statusBar()->showMessage("Cannot write " + path, 5000);
}

/*--------------------------------------------------------------------*/
void MainWindow::togglePick(bool on) { view_->setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor); }

void MainWindow::showPickDialog() {
  if (pick_dialog_ == nullptr) {
    pick_dialog_ = new PickDialog(this);
    connect(pick_dialog_, &QDialog::finished, this, [this]() { pick_->setChecked(false); });
    pick_dialog_->resize(std::max(600, width()), std::max(240, height() / 3));
    pick_dialog_->setMinimumSize(600, 240);
  }
  pick_dialog_->show();
  if (!pick_shown_once_) {
    pick_dialog_->raise();
    pick_dialog_->activateWindow();
    pick_shown_once_ = true;
  }
}

void MainWindow::onClicked(double across, double depth) {
  if (pick_->isChecked())
    pick(across, depth);
}

QStringList MainWindow::pick(double across, double depth) {
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
    statusBar()->showMessage("Nothing to pick (no grid).", 3000);
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
  return row;
}

}  // namespace wcd
