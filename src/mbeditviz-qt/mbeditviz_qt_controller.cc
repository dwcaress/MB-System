/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_controller.cc  11/10/2026
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

#include "mbeditviz_qt_controller.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

#include "mbeditviz_qt_bridge.h"
#include "mbeditviz_qt_dialogs.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
}

namespace mbevq {

namespace {

SwathController *the_controller = nullptr;

}  // namespace

SwathController::SwathController(QObject *parent) : QObject(parent) {
  the_controller = this;

  /* menus, as in Motif */
  QMenu *file = new QMenu(tr("File"));
  menus_.append(file);
  open_ = file->addAction(tr("Open Swath Data"));
  connect(open_, &QAction::triggered, this, [this] { showOpenDialog(); });
  connect(file->addAction(tr("Quit")), &QAction::triggered, this, &SwathController::closeRequested);
  QMenu *options = new QMenu(tr("Options"));
  menus_.append(options);
  update_grid_ = options->addAction(tr("Generate New Grid"));
  connect(update_grid_, &QAction::triggered, this, [this] { updateGrid(); });
  QMenu *help = new QMenu(tr("Help"));
  menus_.append(help);
  connect(help->addAction(tr("About")), &QAction::triggered, this, [this] { about(); });

  /* initialize the gui code, as do_mbeditviz_init */
  mbev_message_on = false;
  do_mbeditviz_update_gui();

  /* initialize the vizualization code */
  mbvq::start(mbev_verbose);
  mb3dsoundings_set_dismiss_notify(mbev_verbose, mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mb3dsoundings_set_edit_notify(mbev_verbose, mbeditviz_mb3dsoundings_edit, &mbev_error);

  /* keep the file list up to date, as the Motif work function (every second) */
  auto *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [] { mbeditviz_qt_tick(); });
  timer->start(1000);
}

SwathController::~SwathController() {
  qDeleteAll(menus_);
  /* the dialogs of a window are deleted with it */
  for (QDialog *dialog : {static_cast<QDialog *>(open_dialog_.data()), static_cast<QDialog *>(grid_dialog_.data()),
                          message_.data()})
    if (dialog != nullptr && dialog->parent() == nullptr)
      delete dialog;
  if (the_controller == this)
    the_controller = nullptr;
}

SwathController *SwathController::current() { return the_controller; }

int SwathController::init(int argc, char **argv) {
  static char program_name[] = "mbeditviz-qt";
  static char help_message[] = "MBeditviz is a bathymetry editor and ptch test tool";
  static char usage_message[] =
      "mbeditviz-qt\n"
      "\t--delete-input-file {-R}\n"
      "\t--format=format_id {-Fformat_id}\n"
      "\t--help {-H}\n"
      "\t--input=file {-Ifile}\n"
      "\t--simple-mean-grid {-G}\n"
      "\t--verbose {-V}\n";
  char *no_options[] = {program_name, nullptr};
  if (argc < 1 || argv == nullptr) {
    argc = 1;
    argv = no_options;
  }
  const int status = mbeditviz_init(argc, argv, program_name, help_message, usage_message, &do_mbeditviz_message_on,
                                    &do_mbeditviz_message_off, &do_mbeditviz_update_gui, &mbeditviz_qt_error_dialog);
  emit modeChanged(editMode());
  return status;
}

bool SwathController::openFile(const QString &path, int format) {
  if (path.isEmpty())
    return false;
  const QByteArray file = QFile::encodeName(path);
  mbeditviz_qt_open(file.constData(), format);
  return mbev_status == MB_SUCCESS;
}

void SwathController::showOpenDialog() {
  if (open_dialog_ == nullptr)
    open_dialog_ = new OpenDialog(this, dialog_parent_);
  open_dialog_->show_mode();
  open_dialog_->show();
  open_dialog_->raise();
}

void SwathController::viewAll() { mbeditviz_qt_view_all(); }

void SwathController::viewSelected() { mbeditviz_qt_view_selected(); }

void SwathController::removeSelected() { mbeditviz_qt_delete_selected(); }

void SwathController::updateGrid() { mbeditviz_qt_regrid(); }

bool SwathController::prepareToClose() {
  /* as the Motif Quit: unload the files (unlocking them) */
  mbeditviz_qt_quit();
  return true;
}

void SwathController::about() {
  QMessageBox::about(dialog_parent_, tr("About MBeditviz..."),
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

bool SwathController::editMode() const { return mbeditviz_qt_edit_mode(); }

void SwathController::setEditMode(bool edit) {
  mbeditviz_qt_set_edit_mode(edit);
  emit modeChanged(edit);
}

void SwathController::setSelectedRows(const QList<int> &rows) {
  selected_.clear();
  for (int row : rows)
    if (row >= 0 && row < files_.size() && !selected_.contains(row))
      selected_.append(row);
  std::sort(selected_.begin(), selected_.end());
}

void SwathController::setStatus(const QString &text) {
  status_ = text;
  emit statusChanged(text);
}

void SwathController::setSensitivity(bool open, bool files) {
  /* as do_mbeditviz_update_gui */
  open_enabled_ = open;
  files_enabled_ = files;
  open_->setEnabled(open);
  update_grid_->setEnabled(!files);
  emit sensitivityChanged(open, files);
}

void SwathController::listBegin() {
  /* keep the current selection, if any */
  saved_count_ = files_.size();
  files_.clear();
}

void SwathController::listAdd(const QString &label) { files_.append(label); }

void SwathController::listEnd() {
  /* reinstate selection if the number of items is the same as before */
  if (files_.size() != saved_count_)
    selected_.clear();
  emit filesChanged();
}

void SwathController::showMessage(const QString &message, bool on) {
  if (message_ == nullptr) {
    message_ = new QDialog(dialog_parent_);
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

void SwathController::showError(const QString &text) {
  QApplication::beep();
  QMessageBox::warning(dialog_parent_, tr("Error"), text);
}

void SwathController::showGridParameters() {
  if (grid_dialog_ == nullptr)
    grid_dialog_ = new GridDialog(dialog_parent_);
  grid_dialog_->load();
  grid_dialog_->show();
  grid_dialog_->raise();
}

}  // namespace mbevq

/*------------------------------------------------------------------------------*/
extern "C" {

void mbeditviz_qt_status(const char *bxstring) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->setStatus(mbvq::label_text(bxstring));
}

void mbeditviz_qt_sensitivity(bool open, bool files) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->setSensitivity(open, files);
}

int mbeditviz_qt_list_count(void) {
  mbevq::SwathController *controller = mbevq::SwathController::current();
  return controller != nullptr ? static_cast<int>(controller->files().size()) : 0;
}

void mbeditviz_qt_list_begin(void) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->listBegin();
}

void mbeditviz_qt_list_add(const char *label) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->listAdd(QString::fromLocal8Bit(label));
}

void mbeditviz_qt_list_end(void) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->listEnd();
}

int mbeditviz_qt_list_selection(int *positions, int max) {
  mbevq::SwathController *controller = mbevq::SwathController::current();
  if (controller == nullptr)
    return 0;
  int count = 0;
  for (int row : controller->selectedRows())
    if (count < max)
      positions[count++] = row + 1;
  return count;
}

void mbeditviz_qt_message(const char *message, bool on) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->showMessage(message != nullptr ? QString::fromLocal8Bit(message) : QString(), on);
}

void mbeditviz_qt_error(const char *s1, const char *s2, const char *s3) {
  const QString text = QStringLiteral("%1\n%2\n%3")
                           .arg(QString::fromLocal8Bit(s1), QString::fromLocal8Bit(s2), QString::fromLocal8Bit(s3));
  if (mbevq::SwathController *controller = mbevq::SwathController::current()) {
    controller->showError(text);
  }
  else {
    QApplication::beep();
    QMessageBox::warning(nullptr, QObject::tr("Error"), text);
  }
}

void mbeditviz_qt_gridparameters(void) {
  if (mbevq::SwathController *controller = mbevq::SwathController::current())
    controller->showGridParameters();
}

void mbeditviz_qt_bell(void) { QApplication::beep(); }

}  // extern "C"
