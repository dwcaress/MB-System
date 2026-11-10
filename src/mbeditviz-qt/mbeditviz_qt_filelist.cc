/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_filelist.cc  11/10/2026
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

#include "mbeditviz_qt_filelist.h"

#include <QButtonGroup>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>

#include "mbeditviz_qt_controller.h"
#include "mbvq.h"

namespace mbevq {

namespace {

/* a Motif "BX" label string as plain text */
QString text_of(const char *bxstring) { return mbvq::label_text(bxstring); }

}  // namespace

FileListWidget::FileListWidget(SwathController *controller, QWidget *parent)
    : QWidget(parent), controller_(controller) {
  auto *layout = new QGridLayout(this);

  /* status */
  layout->addWidget(new QLabel(tr("Status:")), 0, 0, Qt::AlignTop);
  status_ = new QLabel;
  status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  status_->setFrameShape(QFrame::StyledPanel);
  layout->addWidget(status_, 0, 1, 1, 2);

  /* mode */
  layout->addWidget(new QLabel(tr("Mode:")), 1, 0);
  auto *mode_row = new QHBoxLayout;
  edit_ = new QRadioButton(tr("Edit Bathymetry"));
  browse_ = new QRadioButton(tr("Browse Only"));
  auto *mode = new QButtonGroup(this);
  mode->addButton(edit_);
  mode->addButton(browse_);
  mode_row->addWidget(edit_);
  mode_row->addWidget(browse_);
  mode_row->addStretch();
  layout->addLayout(mode_row, 1, 1, 1, 2);
  connect(edit_, &QRadioButton::toggled, this, [this](bool on) { controller_->setEditMode(on); });

  /* the buttons */
  auto *buttons = new QHBoxLayout;
  delete_ = new QPushButton(text_of(":::t\"Remove\":t\"Selected\"\"Files\""));
  view_selected_ = new QPushButton(text_of(":::t\"View\":t\"Selected\"\"Files\""));
  view_all_ = new QPushButton(text_of(":::t\"View\":t\"All\"\"Files\""));
  buttons->addWidget(delete_);
  buttons->addWidget(view_selected_);
  buttons->addWidget(view_all_);
  layout->addLayout(buttons, 2, 0, 1, 3);
  connect(delete_, &QPushButton::clicked, this, [this] { controller_->removeSelected(); });
  connect(view_selected_, &QPushButton::clicked, this, [this] { controller_->viewSelected(); });
  connect(view_all_, &QPushButton::clicked, this, [this] { controller_->viewAll(); });

  /* the swath files */
  layout->addWidget(new QLabel(tr("Swath Files:")), 3, 0, 1, 3);
  list_ = new QListWidget;
  list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  list_->setFont(mono);
  layout->addWidget(list_, 4, 0, 1, 3);
  layout->setRowStretch(4, 1);
  layout->setColumnStretch(2, 1);
  connect(list_, &QListWidget::itemSelectionChanged, this, [this] {
    QList<int> rows;
    for (QListWidgetItem *item : list_->selectedItems())
      rows.append(list_->row(item));
    controller_->setSelectedRows(rows);
  });

  /* the state of the controller */
  connect(controller_, &SwathController::statusChanged, status_, &QLabel::setText);
  connect(controller_, &SwathController::sensitivityChanged, this, [this] { show_sensitivity(); });
  connect(controller_, &SwathController::modeChanged, this, [this] { show_mode(); });
  connect(controller_, &SwathController::filesChanged, this, [this] { show_files(); });
  status_->setText(controller_->status());
  show_sensitivity();
  show_mode();
  show_files();
}

void FileListWidget::show_mode() {
  const bool edit = controller_->editMode();
  const QSignalBlocker block_edit(edit_);
  const QSignalBlocker block_browse(browse_);
  edit_->setChecked(edit);
  browse_->setChecked(!edit);
}

void FileListWidget::show_sensitivity() {
  /* as do_mbeditviz_update_gui */
  const bool files = controller_->filesEnabled();
  delete_->setEnabled(files);
  view_selected_->setEnabled(files);
  view_all_->setEnabled(files);
  edit_->setEnabled(files);
  browse_->setEnabled(files);
}

void FileListWidget::show_files() {
  /* the rows and the selection kept by the controller */
  const QSignalBlocker block(list_);
  list_->clear();
  list_->addItems(controller_->files());
  for (int row : controller_->selectedRows())
    if (row < list_->count())
      list_->item(row)->setSelected(true);
}

}  // namespace mbevq
