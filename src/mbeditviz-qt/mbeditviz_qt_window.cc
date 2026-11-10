/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_window.cc  11/8/2026
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

#include "mbeditviz_qt_window.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

#include "mbeditviz_qt_bridge.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
}

namespace {

MbeditvizWindow *main_window = nullptr;

/* a Motif "BX" label string as plain text */
QString text_of(const char *bxstring) { return mbvq::label_text(bxstring); }

}  // namespace

/*------------------------------------------------------------------------------*/
MbeditvizWindow::MbeditvizWindow() {
  main_window = this;
  setWindowTitle(QStringLiteral("MBeditviz"));

  /* menus, as in Motif */
  QMenu *file = menuBar()->addMenu(tr("File"));
  open_ = file->addAction(tr("Open Swath Data"));
  connect(open_, &QAction::triggered, this, [this] {
    if (open_dialog_ == nullptr)
      open_dialog_ = new OpenDialog(this);
    open_dialog_->show_mode();
    open_dialog_->show();
    open_dialog_->raise();
  });
  connect(file->addAction(tr("Quit")), &QAction::triggered, this, &QWidget::close);
  QMenu *options = menuBar()->addMenu(tr("Options"));
  update_grid_ = options->addAction(tr("Generate New Grid"));
  connect(update_grid_, &QAction::triggered, this, [] { mbeditviz_qt_regrid(); });
  QMenu *help = menuBar()->addMenu(tr("Help"));
  connect(help->addAction(tr("About")), &QAction::triggered, this, [this] { about(); });

  auto *central = new QWidget;
  auto *layout = new QGridLayout(central);

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
  connect(edit_, &QRadioButton::toggled, this, [this](bool on) {
    mbeditviz_qt_set_edit_mode(on);
    show_mode();
  });

  /* the buttons */
  auto *buttons = new QHBoxLayout;
  delete_ = new QPushButton(text_of(":::t\"Remove\":t\"Selected\"\"Files\""));
  view_selected_ = new QPushButton(text_of(":::t\"View\":t\"Selected\"\"Files\""));
  view_all_ = new QPushButton(text_of(":::t\"View\":t\"All\"\"Files\""));
  buttons->addWidget(delete_);
  buttons->addWidget(view_selected_);
  buttons->addWidget(view_all_);
  layout->addLayout(buttons, 2, 0, 1, 3);
  connect(delete_, &QPushButton::clicked, this, [] { mbeditviz_qt_delete_selected(); });
  connect(view_selected_, &QPushButton::clicked, this, [] { mbeditviz_qt_view_selected(); });
  connect(view_all_, &QPushButton::clicked, this, [] { mbeditviz_qt_view_all(); });

  /* the swath files */
  layout->addWidget(new QLabel(tr("Swath Files:")), 3, 0, 1, 3);
  list_ = new QListWidget;
  list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  list_->setFont(mono);
  layout->addWidget(list_, 4, 0, 1, 3);
  layout->setRowStretch(4, 1);
  layout->setColumnStretch(2, 1);
  setCentralWidget(central);
  resize(453, 557);
  show_mode();
}

MbeditvizWindow *MbeditvizWindow::instance() { return main_window; }

void MbeditvizWindow::set_status(const QString &text) { status_->setText(text); }

void MbeditvizWindow::set_sensitivity(bool open, bool files) {
  /* as do_mbeditviz_update_gui */
  open_->setEnabled(open);
  delete_->setEnabled(files);
  view_selected_->setEnabled(files);
  view_all_->setEnabled(files);
  edit_->setEnabled(files);
  browse_->setEnabled(files);
  update_grid_->setEnabled(!files);
}

void MbeditvizWindow::show_mode() {
  const bool edit = mbeditviz_qt_edit_mode();
  {
    const QSignalBlocker block_edit(edit_);
    const QSignalBlocker block_browse(browse_);
    edit_->setChecked(edit);
    browse_->setChecked(!edit);
  }
  if (open_dialog_ != nullptr)
    open_dialog_->show_mode();
}

void MbeditvizWindow::list_begin() {
  /* get the current selection, if any, from the list */
  saved_count_ = list_->count();
  saved_rows_.clear();
  for (QListWidgetItem *item : list_->selectedItems())
    saved_rows_.append(list_->row(item));
  list_->clear();
}

void MbeditvizWindow::list_add(const QString &label) { list_->addItem(label); }

void MbeditvizWindow::list_end() {
  /* reinstate selection if the number of items is the same as before */
  if (list_->count() == saved_count_)
    for (int row : saved_rows_)
      if (row < list_->count())
        list_->item(row)->setSelected(true);
}

void MbeditvizWindow::show_message(const QString &message, bool on) {
  if (message_ == nullptr) {
    message_ = new QDialog(this);
    message_->setWindowTitle(tr("Please Wait..."));
    auto *layout = new QVBoxLayout(message_);
    layout->addWidget(new QLabel(tr("Thank you for your patience...")));
    message_text_ = new QLabel;
    layout->addWidget(message_text_);
  }
  if (on) {
    message_text_->setText(message);
    message_->show();
    message_->raise();
  }
  else {
    message_->hide();
  }
  /* the work goes on without the event loop: draw the message now, as the
     Motif XmUpdateDisplay, without taking user input */
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MbeditvizWindow::about() {
  QMessageBox::about(this, tr("About MBeditviz..."),
                     tr("<h3>MBeditviz</h3><p>Swath Bathymetry Editing and Patch Test Tool</p>"
                        "<p>Created By: David W. Caress and Dale N. Chayes</p>"
                        "<p>Qt6 user interface by Christian dos Santos Ferreira<br>"
                        "MARUM, Center for Marine Environmental Sciences, University of Bremen</p>"
                        "<p>One component of the MB-System, an open source software package for the "
                        "processing and display of swath sonar data.</p>"
                        "<p>MB-System Release %1, %2</p>"
                        "<p>Monterey Bay Aquarium Research Institute<br>"
                        "Lamont-Doherty Earth Observatory of Columbia University</p>")
                         .arg(QString::fromLatin1(MB_VERSION), QString::fromLatin1(MB_VERSION_DATE)));
}

GridDialog *MbeditvizWindow::grid_dialog() {
  if (grid_dialog_ == nullptr)
    grid_dialog_ = new GridDialog(this);
  return grid_dialog_;
}

void MbeditvizWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif Quit: unload the files (unlocking them), then end */
  if (!quitting_) {
    quitting_ = true;
    mbeditviz_qt_quit();
  }
  event->accept();
  QApplication::quit();
}

void mbeditviz_qt_start(MbeditvizWindow *window) {
  /* initialize the gui code, as do_mbeditviz_init */
  mbev_message_on = false;
  window->show_mode();
  do_mbeditviz_update_gui();

  /* initialize the vizualization code */
  mbvq::start(mbev_verbose);
  mb3dsoundings_set_dismiss_notify(mbev_verbose, mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mb3dsoundings_set_edit_notify(mbev_verbose, mbeditviz_mb3dsoundings_edit, &mbev_error);

  /* keep the file list up to date, as the Motif work function (every second) */
  auto *timer = new QTimer(window);
  QObject::connect(timer, &QTimer::timeout, window, [] { mbeditviz_qt_tick(); });
  timer->start(1000);
}

/*------------------------------------------------------------------------------*/
OpenDialog::OpenDialog(QWidget *parent) : QDialog(parent) {
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

  connect(edit_, &QRadioButton::toggled, this, [](bool on) {
    mbeditviz_qt_set_edit_mode(on);
    if (MbeditvizWindow::instance() != nullptr)
      MbeditvizWindow::instance()->show_mode();
  });
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
  const QByteArray path = QFile::encodeName(file);
  mbeditviz_qt_open(path.constData(), format);
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

/*------------------------------------------------------------------------------*/
extern "C" {

void mbeditviz_qt_status(const char *bxstring) {
  if (main_window != nullptr)
    main_window->set_status(text_of(bxstring));
}

void mbeditviz_qt_sensitivity(bool open, bool files) {
  if (main_window != nullptr)
    main_window->set_sensitivity(open, files);
}

int mbeditviz_qt_list_count(void) { return main_window != nullptr ? main_window->list()->count() : 0; }

void mbeditviz_qt_list_begin(void) {
  if (main_window != nullptr)
    main_window->list_begin();
}

void mbeditviz_qt_list_add(const char *label) {
  if (main_window != nullptr)
    main_window->list_add(QString::fromLocal8Bit(label));
}

void mbeditviz_qt_list_end(void) {
  if (main_window != nullptr)
    main_window->list_end();
}

int mbeditviz_qt_list_selection(int *positions, int max) {
  if (main_window == nullptr)
    return 0;
  QList<int> rows;
  for (QListWidgetItem *item : main_window->list()->selectedItems())
    rows.append(main_window->list()->row(item));
  std::sort(rows.begin(), rows.end());
  int count = 0;
  for (int row : rows)
    if (count < max)
      positions[count++] = row + 1;
  return count;
}

void mbeditviz_qt_message(const char *message, bool on) {
  if (main_window != nullptr)
    main_window->show_message(message != nullptr ? QString::fromLocal8Bit(message) : QString(), on);
}

void mbeditviz_qt_error(const char *s1, const char *s2, const char *s3) {
  QApplication::beep();
  QMessageBox::warning(main_window, QObject::tr("Error"),
                       QStringLiteral("%1\n%2\n%3")
                           .arg(QString::fromLocal8Bit(s1), QString::fromLocal8Bit(s2), QString::fromLocal8Bit(s3)));
}

void mbeditviz_qt_gridparameters(void) {
  if (main_window == nullptr)
    return;
  GridDialog *dialog = main_window->grid_dialog();
  dialog->load();
  dialog->show();
  dialog->raise();
}

void mbeditviz_qt_bell(void) { QApplication::beep(); }

}  // extern "C"
