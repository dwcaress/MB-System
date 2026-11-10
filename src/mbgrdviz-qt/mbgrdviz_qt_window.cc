/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_window.cc  11/7/2026
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

#include "mbgrdviz_qt_window.h"

#include <cstdio>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "mbgrdviz_qt_bridge.h"
#include "mbgrdviz_qt_dialogs.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"

/* mbgrdviz_state.h is for C only */
extern int mbgrdviz_verbose;
}

namespace {

MbgrdvizWindow *main_window = nullptr;

/* Motif callbacks take (Widget, client data, call data); the main window menu
   passes instance 0, as in Motif */
using Callback = void (*)(Widget, XtPointer, XtPointer);
void call(Callback callback) { callback(nullptr, reinterpret_cast<XtPointer>(static_cast<size_t>(0)), nullptr); }

bool is_save(int mode) {
  return (mode >= MBGRDVIZ_SAVEROUTE && mode <= MBGRDVIZ_SAVEPROFILE) ||
         (mode >= MBGRDVIZ_SAVEPOLYLINE && mode <= MBGRDVIZ_EXPORTGISPOINT);
}

}  // namespace

MbgrdvizWindow::MbgrdvizWindow() {
  main_window = this;
  setWindowTitle(QStringLiteral("MBgrdviz"));
  directory_ = QDir::currentPath();

  /* File menu, as in Motif */
  QMenu *file = menuBar()->addMenu(tr("File"));
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
  connect(file->addAction(tr("Quit")), &QAction::triggered, this, &QWidget::close);

  /* Realtime menu: as in Motif, only the setup dialog; the realtime display is
     not implemented in mbgrdviz, so Start, Stop, Pause and Resume, which do
     nothing in Motif, are disabled */
  QMenu *realtime = menuBar()->addMenu(tr("Realtime"));
  connect(realtime->addAction(tr("Setup")), &QAction::triggered, this, [this] {
    RealtimeDialog *dialog = RealtimeDialog::instance(this);
    dialog->show();
    dialog->raise();
  });
  for (const char *label : {"Start", "Stop", "Pause", "Resume"}) {
    QAction *action = realtime->addAction(tr(label));
    action->setEnabled(false);
    action->setToolTip(tr("The realtime display is not implemented in mbgrdviz yet"));
  }

  QMenu *help = menuBar()->addMenu(tr("Help"));
  connect(help->addAction(tr("About")), &QAction::triggered, this, [this] { about(); });

  auto *label = new QLabel(QStringLiteral("MBgrdviz\nMB-System %1").arg(QString::fromLatin1(MB_VERSION)));
  label->setAlignment(Qt::AlignCenter);
  setCentralWidget(label);
  move(108, 241);
  resize(260, 215);
  set_sensitivity(true, false, false, false);
}

MbgrdvizWindow *MbgrdvizWindow::instance() { return main_window; }

QString MbgrdvizWindow::find_tool(const QString &tool, const QString &directory) {
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

void MbgrdvizWindow::set_sensitivity(bool open_primary, bool open_other, bool save_site, bool save_route) {
  open_primary_->setEnabled(open_primary);
  for (QAction *action : open_other_)
    action->setEnabled(open_other);
  save_site_->setEnabled(save_site);
  save_route_->setEnabled(save_route);
}

void MbgrdvizWindow::file_dialog(int mode, size_t instance, const QString &title, const QString &pattern) {
  /* the pattern of the Motif dialog, and all files */
  const QString filter = pattern == QStringLiteral("*")
                             ? tr("All files (*)")
                             : tr("Files (%1);;All files (*)").arg(pattern);
  const QString file = is_save(mode) ? QFileDialog::getSaveFileName(this, title, directory_, filter)
                                     : QFileDialog::getOpenFileName(this, title, directory_, filter);
  if (file.isEmpty())
    return;
  directory_ = QFileInfo(file).absolutePath();
  QByteArray path = QFile::encodeName(file);
  mbgrdviz_openfile(static_cast<size_t>(mode), instance, path.data());
}

void MbgrdvizWindow::about() {
  QMessageBox::about(this, tr("About MBgrdviz"),
                     tr("<h3>MBgrdviz</h3><p>Interactive 2D/3D visualization of GMT grids.</p>"
                        "<p>MB-System Release %1, %2</p>"
                        "<p>Created by David W. Caress and Dale N. Chayes</p>"
                        "<p>Qt6 user interface by Christian dos Santos Ferreira<br>"
                        "MARUM, Center for Marine Environmental Sciences, University of Bremen</p>")
                         .arg(QString::fromLatin1(MB_VERSION), QString::fromLatin1(MB_VERSION_DATE)));
}

void MbgrdvizWindow::closeEvent(QCloseEvent *event) {
  /* as the Motif Quit: close the mbview windows, then the program */
  int error = MB_ERROR_NO_ERROR;
  mbview_quit(mbgrdviz_verbose, &error);
  event->accept();
  QApplication::quit();
}

/*------------------------------------------------------------------------------*/
extern "C" {

void mbgrdviz_qt_bell(void) { QApplication::beep(); }

void mbgrdviz_qt_realtime_path(const char *path) {
  if (main_window != nullptr)
    RealtimeDialog::instance(main_window)->set_path(QFile::decodeName(path));
}

void mbgrdviz_qt_sensitivity(bool open_primary, bool open_other, bool save_site, bool save_route) {
  if (main_window != nullptr)
    main_window->set_sensitivity(open_primary, open_other, save_site, save_route);
}

void mbgrdviz_qt_file_dialog(int mode, size_t instance, const char *title, const char *pattern) {
  if (main_window != nullptr)
    main_window->file_dialog(mode, instance, QString::fromLatin1(title), QString::fromLatin1(pattern));
}

void mbgrdviz_qt_launch(const char *const argv[]) {
  /* as the Motif mbgrdviz, without a shell; QProcess also works on Windows */
  const QString tool = QString::fromLocal8Bit(argv[0]);
  const QString program = MbgrdvizWindow::find_tool(tool, QCoreApplication::applicationDirPath());
  if (program.isEmpty()) {
    fprintf(stderr, "Unable to find %s\n", argv[0]);
    QApplication::beep();
    return;
  }
  QStringList arguments;
  for (int i = 1; argv[i] != nullptr; i++)
    arguments << QString::fromLocal8Bit(argv[i]);
  fprintf(stderr, "Starting %s\n", QFile::encodeName(program).constData());
  if (!QProcess::startDetached(program, arguments)) {
    fprintf(stderr, "Unable to start %s\n", QFile::encodeName(program).constData());
    QApplication::beep();
  }
}

}  // extern "C"
