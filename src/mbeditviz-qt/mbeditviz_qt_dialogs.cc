/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_dialogs.cc  11/10/2026
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

#include "mbeditviz_qt_dialogs.h"

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <algorithm>

#include "mbeditviz_qt_bridge.h"
#include "mbeditviz_qt_controller.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
}

namespace {

/* a Motif "BX" label string as plain text */
QString text_of(const char *bxstring) { return mbvq::label_text(bxstring); }

}  // namespace

/*------------------------------------------------------------------------------*/
OpenDialog::OpenDialog(mbevq::SwathController *controller, QWidget *parent) : QDialog(parent), controller_(controller) {
  setWindowTitle(tr("Open Swath Data"));
  directory_ = QDir::currentPath();
  auto *layout = new QVBoxLayout(this);
  auto *form = new QFormLayout;
  auto *mode_row = new QHBoxLayout;
  edit_ = new QRadioButton(tr("Output Edits"));
  browse_ = new QRadioButton(tr("Browse Only"));
  auto *mode = new QButtonGroup(this);
  mode->addButton(edit_);
  mode->addButton(browse_);
  mode_row->addWidget(edit_);
  mode_row->addWidget(browse_);
  form->addRow(tr("Output Mode:"), mode_row);
  format_ = new QLineEdit(QStringLiteral("-1"));
  form->addRow(tr("MBIO Format ID:"), format_);
  auto *path_row = new QHBoxLayout;
  path_ = new QLineEdit;
  auto *browse_files = new QPushButton(tr("Browse..."));
  path_row->addWidget(path_, 1);
  path_row->addWidget(browse_files);
  form->addRow(tr("File:"), path_row);
  layout->addLayout(form);
  auto *buttons = new QHBoxLayout;
  auto *ok = new QPushButton(tr("OK"));
  auto *cancel = new QPushButton(tr("Cancel"));
  ok->setDefault(true);
  buttons->addStretch();
  buttons->addWidget(ok);
  buttons->addWidget(cancel);
  layout->addLayout(buttons);
  resize(520, height());

  /* the mode is shared with the main window, through the controller */
  connect(edit_, &QRadioButton::toggled, this, [this](bool on) { controller_->setEditMode(on); });
  connect(controller_, &mbevq::SwathController::modeChanged, this, [this] { show_mode(); });
  connect(browse_files, &QPushButton::clicked, this, [this] { browse(); });
  connect(ok, &QPushButton::clicked, this, [this] { open_file(); });
  connect(cancel, &QPushButton::clicked, this, &QDialog::hide);
  show_mode();
}

void OpenDialog::show_mode() {
  const bool edit = mbeditviz_qt_edit_mode();
  const QSignalBlocker block_edit(edit_);
  const QSignalBlocker block_browse(browse_);
  edit_->setChecked(edit);
  browse_->setChecked(!edit);
}

void OpenDialog::browse() {
  const QString file = QFileDialog::getOpenFileName(this, tr("Open Swath Data"), directory_,
                                                    tr("Swath data and datalists (*.mb*);;All files (*)"));
  if (file.isEmpty())
    return;
  directory_ = QFileInfo(file).absolutePath();
  path_->setText(file);
  /* look for the MB suffix convention, as the Motif file list callback */
  const QByteArray path = QFile::encodeName(file);
  const int format = mbeditviz_qt_format_of(path.constData());
  if (format != -1)
    format_->setText(QString::number(format));
}

void OpenDialog::open_file() {
  /* read the mbio format number, 0 if it is not a number (Motif) */
  bool ok = false;
  int format = format_->text().trimmed().toInt(&ok);
  if (!ok)
    format = 0;
  const QString file = path_->text().trimmed();
  hide();
  if (file.isEmpty())
    return;
  controller_->openFile(file, format);
}

/*------------------------------------------------------------------------------*/
GridDialog::GridDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Grid Parameters"));
  auto *layout = new QVBoxLayout(this);
  current_ = new QLabel;
  current_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(current_);

  layout->addWidget(new QLabel(tr("Grid Cell Size (m)")));
  auto *cell_row = new QHBoxLayout;
  cellsize_ = new QSlider(Qt::Horizontal);
  cellsize_->setMinimum(1);
  cellsize_->setTracking(false);
  cellsize_value_ = new QLabel;
  cell_row->addWidget(cellsize_, 1);
  cell_row->addWidget(cellsize_value_);
  layout->addLayout(cell_row);

  auto *form = new QFormLayout;
  auto *algorithm_row = new QVBoxLayout;
  auto *algorithm = new QButtonGroup(this);
  const char *const labels[3] = {"Simple Mean", "Footprint", "Shoal Bias"};
  for (int i = 0; i < 3; i++) {
    algorithm_[i] = new QRadioButton(tr(labels[i]));
    algorithm->addButton(algorithm_[i], i);
    algorithm_row->addWidget(algorithm_[i]);
    connect(algorithm_[i], &QRadioButton::toggled, this, [this](bool on) {
      if (on)
        changed();
    });
  }
  form->addRow(tr("Gridding Algorithm:"), algorithm_row);
  interpolation_ = new QLineEdit;
  form->addRow(tr("Interpolation scale in grid cells:"), interpolation_);
  layout->addLayout(form);
  implied_ = new QLabel;
  layout->addWidget(implied_);

  auto *buttons = new QHBoxLayout;
  auto *dismiss = new QPushButton(tr("Dismiss"));
  auto *apply = new QPushButton(tr("Apply"));
  buttons->addWidget(dismiss);
  buttons->addStretch();
  buttons->addWidget(apply);
  layout->addLayout(buttons);

  connect(cellsize_, &QSlider::sliderMoved, this,
          [this](int value) { cellsize_value_->setText(QString::number(value / 1000.0, 'f', 3)); });
  connect(cellsize_, &QSlider::valueChanged, this, [this](int value) {
    /* as the Motif do_mbeditviz_changecellsize: the maximum follows the value */
    {
      const QSignalBlocker blocker(cellsize_);
      if (value <= 1 && cellsize_->maximum() > 2)
        cellsize_->setMaximum(std::max(cellsize_->maximum() / 2, 2));
      else if (value == cellsize_->maximum())
        cellsize_->setMaximum(2 * cellsize_->maximum());
    }
    changed();
  });
  connect(interpolation_, &QLineEdit::editingFinished, this, [this] { changed(); });
  connect(dismiss, &QPushButton::clicked, this, &QDialog::hide);
  connect(apply, &QPushButton::clicked, this, [this] {
    hide();
    mbeditviz_qt_grid_apply();
  });
}

void GridDialog::load() {
  struct mbeditviz_qt_grid grid;
  mbeditviz_qt_grid_get(&grid);
  loading_ = true;
  cellsize_->setMaximum(std::max(5 * grid.cellsize, 2));
  cellsize_->setValue(grid.cellsize);
  cellsize_value_->setText(QString::number(grid.cellsize / 1000.0, 'f', 3));
  if (grid.algorithm >= 0 && grid.algorithm < 3)
    algorithm_[grid.algorithm]->setChecked(true);
  interpolation_->setText(QString::number(grid.interpolation));
  loading_ = false;
  show_texts();
}

void GridDialog::changed() {
  if (loading_)
    return;
  struct mbeditviz_qt_grid grid;
  mbeditviz_qt_grid_get(&grid);
  grid.cellsize = cellsize_->value();
  cellsize_value_->setText(QString::number(grid.cellsize / 1000.0, 'f', 3));
  for (int i = 0; i < 3; i++)
    if (algorithm_[i]->isChecked())
      grid.algorithm = i;
  /* as the Motif sscanf: a value that is not a number leaves the old one */
  bool ok = false;
  const int interpolation = interpolation_->text().trimmed().toInt(&ok);
  if (ok)
    grid.interpolation = interpolation;
  mbeditviz_qt_grid_set(&grid);
  show_texts();
}

void GridDialog::show_texts() {
  char current[MB_PATH_MAXLINE];
  char implied[MB_PATH_MAXLINE];
  mbeditviz_qt_grid_texts(current, implied, sizeof(current));
  current_->setText(text_of(current));
  implied_->setText(text_of(implied));
}
