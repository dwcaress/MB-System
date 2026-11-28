/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_controller.cc  11/10/2026
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

#include "mbgrdviz_qt_controller.h"

#include <cstdio>

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

#include "mbgrdviz_qt_bridge.h"
#include "mbgrdviz_qt_dialogs.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"

/* mbgrdviz_state.h is for C only */
extern int mbgrdviz_verbose;
extern bool mbgrdviz_mbview_id[MBV_MAX_WINDOWS];
}

namespace mbgvq {

namespace {

Controller *the_controller = nullptr;

/* Motif callbacks take (Widget, client data, call data); the main window menu
   passes instance 0, as in Motif */
using Callback = void (*)(Widget, XtPointer, XtPointer);
void call(Callback callback) { callback(nullptr, reinterpret_cast<XtPointer>(static_cast<size_t>(0)), nullptr); }

bool is_save(int mode) {
  return (mode >= MBGRDVIZ_SAVEROUTE && mode <= MBGRDVIZ_SAVEPROFILE) ||
         (mode >= MBGRDVIZ_SAVEPOLYLINE && mode <= MBGRDVIZ_EXPORTGISPOINT);
}

}  // namespace

Controller::Controller(QObject *parent) : QObject(parent), open_maps_(MBV_MAX_WINDOWS, false) {
  the_controller = this;
  directory_ = QDir::currentPath();

  /* File menu, as in Motif */
  QMenu *file = new QMenu(tr("File"));
  menus_.append(file);
  open_primary_ = file->addAction(tr("Open Primary Grid"));
  connect(open_primary_, &QAction::triggered, this, [] { call(do_mbgrdviz_fileSelectionBox); });
  struct Item {
    const char *label;
    Callback callback;
  };
  const Item opens[8] = {{"Open Site", do_mbgrdviz_fileSelectionBox_opensite},
                         {"Open Route", do_mbgrdviz_fileSelectionBox_openroute},
                         {"Open Navigation", do_mbgrdviz_fileSelectionBox_opennav},
                         {"Open Swath", do_mbgrdviz_fileSelectionBox_openswath},
                         {"Open Vector", do_mbgrdviz_fileSelectionBox_openvector},
                         {"Open Polyline", do_mbgrdviz_fileSelectionBox_openpolyline},
                         {"Open Polygon", do_mbgrdviz_fileSelectionBox_openpolygon},
                         {"Open Point", do_mbgrdviz_fileSelectionBox_opengispoint}};
  for (int i = 0; i < 8; i++) {
    const Callback callback = opens[i].callback;
    open_other_[i] = file->addAction(tr(opens[i].label));
    connect(open_other_[i], &QAction::triggered, this, [callback] { call(callback); });
  }
  save_site_ = file->addAction(tr("Save Site"));
  connect(save_site_, &QAction::triggered, this, [] { call(do_mbgrdviz_fileSelectionBox_savesite); });
  save_route_ = file->addAction(tr("Save Route"));
  connect(save_route_, &QAction::triggered, this, [] { call(do_mbgrdviz_fileSelectionBox_saveroute); });
  file->addSeparator();
  connect(file->addAction(tr("Quit")), &QAction::triggered, this, &Controller::closeRequested);

  /* Realtime menu: as in Motif, only the setup dialog; the realtime display is
     not implemented in mbgrdviz, so Start, Stop, Pause and Resume, which do
     nothing in Motif, are disabled */
  QMenu *realtime = new QMenu(tr("Realtime"));
  menus_.append(realtime);
  connect(realtime->addAction(tr("Setup")), &QAction::triggered, this, [this] {
    RealtimeDialog *dialog = RealtimeDialog::instance(dialog_parent_);
    dialog->show();
    dialog->raise();
  });
  for (const char *label : {"Start", "Stop", "Pause", "Resume"}) {
    QAction *action = realtime->addAction(tr(label));
    action->setEnabled(false);
    action->setToolTip(tr("The realtime display is not implemented in mbgrdviz yet"));
  }

  QMenu *help = new QMenu(tr("Help"));
  menus_.append(help);
  connect(help->addAction(tr("About")), &QAction::triggered, this, [this] { about(); });

  setSensitivity(true, false, false, false);
}

Controller::~Controller() {
  qDeleteAll(menus_);
  if (the_controller == this)
    the_controller = nullptr;
}

Controller *Controller::current() { return the_controller; }

size_t Controller::openPrimary(const QString &path) {
  std::vector<bool> before(mbgrdviz_mbview_id, mbgrdviz_mbview_id + MBV_MAX_WINDOWS);
  QByteArray file = QFile::encodeName(path);
  do_mbgrdviz_openprimary(path.isEmpty() ? nullptr : file.data());
  for (size_t i = 0; i < MBV_MAX_WINDOWS; i++)
    if (mbgrdviz_mbview_id[i] && !before[i])
      return i;
  return MBV_NO_WINDOW;
}

bool Controller::openOverlay(size_t instance, const QString &path) {
  if (instance >= MBV_MAX_WINDOWS || !mbgrdviz_mbview_id[instance])
    return false;
  QByteArray file = QFile::encodeName(path);
  return do_mbgrdviz_openoverlay(instance, path.isEmpty() ? nullptr : file.data()) == MB_SUCCESS;
}

QString Controller::findTool(const QString &tool, const QString &directory) {
  /* the Qt versions of the tools */
  QStringList names;
  if (tool == QStringLiteral("mbedit") || tool == QStringLiteral("mbeditviz") ||
      tool == QStringLiteral("mbvelocitytool"))
    names << tool + QStringLiteral("-qt");
  names << tool;
  for (const QString &name : names) {
    /* installed next to this program, or in its own directory of a CMake
       build tree (src/mbedit-qt beside src/mbgrdviz-qt) */
    QString found = QStandardPaths::findExecutable(name, {directory, directory + QStringLiteral("/../") + name});
    if (found.isEmpty())
      found = QStandardPaths::findExecutable(name);
    if (!found.isEmpty())
      return found;
  }
  return QString();
}

bool Controller::prepareToClose() {
  /* as the Motif Quit: close the mbview windows */
  int error = MB_ERROR_NO_ERROR;
  mbview_quit(mbgrdviz_verbose, &error);
  followMaps();
  return true;
}

void Controller::setSensitivity(bool open_primary, bool open_other, bool save_site, bool save_route) {
  open_primary_->setEnabled(open_primary);
  for (QAction *action : open_other_)
    action->setEnabled(open_other);
  save_site_->setEnabled(save_site);
  save_route_->setEnabled(save_route);
  /* the core sets the sensitivity after every map opened or closed */
  followMaps();
}

void Controller::followMaps() {
  for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (mbgrdviz_mbview_id[i] == open_maps_[i])
      continue;
    open_maps_[i] = mbgrdviz_mbview_id[i];
    if (open_maps_[i])
      emit mapOpened(i);
    else
      emit mapClosed(i);
  }
}

void Controller::fileDialog(int mode, size_t instance, const QString &title, const QString &pattern) {
  /* the pattern of the Motif dialog, and all files */
  const QString filter = pattern == QStringLiteral("*")
                             ? tr("All files (*)")
                             : tr("Files (%1);;All files (*)").arg(pattern);
  const QString file = is_save(mode) ? QFileDialog::getSaveFileName(dialog_parent_, title, directory_, filter)
                                     : QFileDialog::getOpenFileName(dialog_parent_, title, directory_, filter);
  if (file.isEmpty())
    return;
  directory_ = QFileInfo(file).absolutePath();
  QByteArray path = QFile::encodeName(file);
  mbgrdviz_openfile(static_cast<size_t>(mode), instance, path.data());
}

void Controller::launch(const QStringList &argv) {
  if (argv.isEmpty())
    return;
  const QString tool = argv.first();
  const QStringList arguments = argv.mid(1);
  if (tool_opener_) {
    if (!tool_opener_(tool, arguments))
      QApplication::beep();
    return;
  }
  /* as the Motif mbgrdviz, without a shell; QProcess also works on Windows */
  const QString program = findTool(tool, QCoreApplication::applicationDirPath());
  if (program.isEmpty()) {
    fprintf(stderr, "Unable to find %s\n", QFile::encodeName(tool).constData());
    emit statusMessage(tr("Unable to find %1").arg(tool));
    QApplication::beep();
    return;
  }
  fprintf(stderr, "Starting %s\n", QFile::encodeName(program).constData());
  if (!QProcess::startDetached(program, arguments)) {
    fprintf(stderr, "Unable to start %s\n", QFile::encodeName(program).constData());
    emit statusMessage(tr("Unable to start %1").arg(program));
    QApplication::beep();
    return;
  }
  emit statusMessage(tr("Started %1").arg(QFileInfo(program).fileName()));
}

void Controller::about() {
  QMessageBox::about(dialog_parent_, tr("About MBgrdviz"),
                     tr("<h3>MBgrdviz</h3><p>Interactive 2D/3D visualization of GMT grids.</p>"
                        "<p>MB-System Release %1, %2</p>"
                        "<p>Created by David W. Caress and Dale N. Chayes</p>"
                        "<p>Qt6 user interface by Christian dos Santos Ferreira<br>"
                        "MARUM, Center for Marine Environmental Sciences, University of Bremen</p>")
                         .arg(QString::fromLatin1(MB_VERSION), QString::fromLatin1(MB_VERSION_DATE)));
}

}  // namespace mbgvq

/*------------------------------------------------------------------------------*/
extern "C" {

void mbgrdviz_qt_bell(void) { QApplication::beep(); }

void mbgrdviz_qt_realtime_path(const char *path) {
  if (mbgvq::Controller *controller = mbgvq::Controller::current())
    RealtimeDialog::instance(controller->dialogParent())->set_path(QFile::decodeName(path));
}

void mbgrdviz_qt_sensitivity(bool open_primary, bool open_other, bool save_site, bool save_route) {
  if (mbgvq::Controller *controller = mbgvq::Controller::current())
    controller->setSensitivity(open_primary, open_other, save_site, save_route);
}

void mbgrdviz_qt_file_dialog(int mode, size_t instance, const char *title, const char *pattern) {
  if (mbgvq::Controller *controller = mbgvq::Controller::current())
    controller->fileDialog(mode, instance, QString::fromLatin1(title), QString::fromLatin1(pattern));
}

void mbgrdviz_qt_launch(const char *const argv[]) {
  QStringList list;
  for (int i = 0; argv[i] != nullptr; i++)
    list << QString::fromLocal8Bit(argv[i]);
  if (mbgvq::Controller *controller = mbgvq::Controller::current())
    controller->launch(list);
}

}  // extern "C"
