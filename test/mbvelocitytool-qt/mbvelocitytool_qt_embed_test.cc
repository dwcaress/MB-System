/*--------------------------------------------------------------------
 *    The MB-system:  mbvelocitytool_qt_embed_test.cc  9/11/2026
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
/*
 * The tool as a widget inside another program's window, with no window of its
 * own: what a future unified application will use. Two of them in docks of one
 * main window. Arguments: two swath files with stored profiles, which get new
 * ancillary files beside them, and a directory to write in. Whether mblevitus
 * is available does not matter.
 */

#include <QAction>
#include <QAbstractButton>
#include <QApplication>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include <cstdio>
#include <initializer_list>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbvt_widget.h"
#include "mbvt_window.h"

using mbvt::VelocityWidget;
using mbvt::VelocityWindow;

namespace {

int checks = 0, failures = 0;

#define CHECK(condition)                                  \
  do {                                                    \
    checks++;                                             \
    if (!(condition)) {                                   \
      failures++;                                         \
      printf("FAIL line %d: %s\n", __LINE__, #condition); \
    }                                                     \
  } while (0)

const int TIME_BEGIN[7] = {1962, 2, 21, 10, 30, 0, 0};
const int TIME_END[7] = {2062, 2, 21, 10, 30, 0, 0};
const int PANEL_MAX_MIN_WIDTH = 420, PANEL_MAX_MIN_HEIGHT = 360;

struct Notes {
  int titles = 0, statuses = 0, opened = 0, failed = 0, busy_changes = 0, edits = 0, residuals = 0, saves = 0, opened_format = -1;
  bool busy = false, unsaved = false;
  QString title, opened_path, failed_path, saved_path;
};

void watch(VelocityWidget *v, Notes &n) {
  QObject::connect(v, &VelocityWidget::titleChanged, [&n](const QString &title) { n.titles++; n.title = title; });
  QObject::connect(v, &VelocityWidget::statusMessage, [&n](const QString &, int) { n.statuses++; });
  QObject::connect(v, &VelocityWidget::unsavedChanged, [&n](bool unsaved) { n.unsaved = unsaved; });
  QObject::connect(v, &VelocityWidget::busyChanged, [&n](bool busy) { n.busy_changes++; n.busy = busy; });
  QObject::connect(v, &VelocityWidget::swathOpened, [&n](const QString &path, int format) { n.opened++; n.opened_path = path; n.opened_format = format; });
  QObject::connect(v, &VelocityWidget::swathOpenFailed, [&n](const QString &path) { n.failed++; n.failed_path = path; });
  QObject::connect(v, &VelocityWidget::editProfileChanged, [&n] { n.edits++; });
  QObject::connect(v, &VelocityWidget::residualsChanged, [&n] { n.residuals++; });
  QObject::connect(v, &VelocityWidget::fileSaved, [&n](const QString &path) { n.saves++; n.saved_path = path; });
}

void wait_for(std::initializer_list<VelocityWidget *> tools) {
  QElapsedTimer timer;
  timer.start();
  bool busy = true;
  while (busy && timer.elapsed() < 60000) {
    QApplication::processEvents(QEventLoop::AllEvents, 20);
    busy = false;
    for (VelocityWidget *v : tools)
      busy = busy || v->busy();
  }
  QApplication::processEvents();
}

QAction *find_action(VelocityWidget *v, const QString &text) {
  for (QMenu *menu : v->menus())
    for (QAction *action : menu->actions())
      if (action->text() == text)
        return action;
  return nullptr;
}

bool same(const mbvt::Residuals &x, const mbvt::Residuals &y) {
  return x.beam_first == y.beam_first && x.beam_last == y.beam_last && x.angle == y.angle && x.acrosstrack == y.acrosstrack &&
         x.altitude == y.altitude && x.residual == y.residual && x.sigma == y.sigma && x.count == y.count && x.rays_traced == y.rays_traced &&
         x.rays_failed == y.rays_failed;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: mbvelocitytool_qt_embed_test file1 file2 directory\n");
    return 2;
  }
  QApplication app(argc, argv);
  const QString a_path = argv[1], b_path = argv[2], dir = argv[3];

  /* what a program with several tools has to do */
  int error = MB_ERROR_NO_ERROR;
  mb_mem_list_disable(0, &error);

  /* every question is answered with this, and counted */
  QMessageBox::StandardButton answer = QMessageBox::Discard;
  int asked = 0;
  QTimer answers;
  answers.setInterval(20);
  QObject::connect(&answers, &QTimer::timeout, [&] {
    auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
    if (box == nullptr)
      return;
    asked++;
    QAbstractButton *button = box->button(answer);
    if (button == nullptr)
      button = box->button(QMessageBox::Ok);
    if (button == nullptr)
      button = box->button(QMessageBox::Yes);
    if (button != nullptr)
      button->click();
  });
  answers.start();

  /* in the window of the program the status widgets are in its status bar and Quit is in its menu */
  {
    VelocityWindow window(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, true);
    CHECK(qobject_cast<QStatusBar *>(window.editor()->pointerLabel()->parent()) != nullptr);
    CHECK(qobject_cast<QStatusBar *>(window.editor()->progressBar()->parent()) != nullptr);
    CHECK(find_action(window.editor(), "&Quit") != nullptr);
    CHECK(window.windowTitle() == window.editor()->title());
  }

  /* one tool alone, for comparison */
  mbvt::Residuals alone;
  {
    VelocityWidget solo(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, false);
    solo.show();
    CHECK(find_action(&solo, "&Reprocess")->shortcutContext() == Qt::WindowShortcut);
    CHECK(solo.pointerLabel()->parentWidget() == &solo);
    CHECK(solo.pointerLabel()->isVisible());
    CHECK(solo.openSwath(a_path, 0));
    wait_for({&solo});
    alone = solo.residuals();
    CHECK(alone.rays_traced > 0);
    CHECK(solo.prepareToClose());
    CHECK(asked == 0);
  }

  /* two tools in docks of one main window */
  QMainWindow host;
  host.setWindowTitle("host");
  auto *a = new VelocityWidget(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, false);
  auto *b = new VelocityWidget(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, false, mbvt::Presentation::Panel);
  auto *dock_a = new QDockWidget("A");
  auto *dock_b = new QDockWidget("B");
  dock_a->setWidget(a);
  dock_b->setWidget(b);
  host.addDockWidget(Qt::LeftDockWidgetArea, dock_a);
  host.addDockWidget(Qt::RightDockWidgetArea, dock_b);
  host.resize(1800, 900);
  Notes na, nb;
  watch(a, na);
  watch(b, nb);
  a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  host.show();
  QApplication::processEvents();

  /* nothing of a window of its own */
  CHECK(host.findChild<QMenuBar *>() == nullptr);
  CHECK(host.findChild<QStatusBar *>() == nullptr);
  CHECK(a->menus().size() == 4);
  CHECK(find_action(a, "&Quit") == nullptr);
  CHECK(find_action(a, "&Reprocess")->shortcutContext() == Qt::WidgetWithChildrenShortcut);
  CHECK(a->actions().contains(find_action(a, "&Reprocess")));
  CHECK(a->pointerLabel()->isVisible());
  CHECK(!a->progressBar()->isVisible());

  /* the panel form */
  CHECK(b->minimumSizeHint().width() <= PANEL_MAX_MIN_WIDTH);
  CHECK(b->minimumSizeHint().height() <= PANEL_MAX_MIN_HEIGHT);
  CHECK(b->focusProxy() == b->velocityPlot());
  CHECK(find_action(b, "&Reprocess")->shortcutContext() == Qt::WidgetWithChildrenShortcut);
  CHECK(b->findChild<QToolBar *>("panel_bar")->isVisible());
  CHECK(a->findChild<QToolBar *>("panel_bar") == nullptr);
  CHECK(b->findChild<QToolButton *>("panel_menu")->menu()->actions().size() == 4);
  CHECK(b->findChild<QToolButton *>("panel_scales") != nullptr);
  CHECK(b->pointerLabel()->isVisible());

  /* its two scale controls follow each other */
  {
    const auto popup = b->findChild<QWidget *>("panel_scales_controls")->findChildren<QDoubleSpinBox *>();
    const auto dialog = b->scalingDialog()->findChildren<QDoubleSpinBox *>();
    CHECK(popup.size() == 4 && dialog.size() == 4);
    popup[0]->setValue(1234.0);
    CHECK(dialog[0]->value() == 1234.0);
    CHECK(b->scalingDialog()->scale().maxdepth == 1234.0);
    dialog[0]->setValue(2345.0);
    CHECK(popup[0]->value() == 2345.0);
  }

  /* its dialog goes and comes with it */
  b->scalingDialog()->show();
  QApplication::processEvents();
  CHECK(b->scalingDialog()->isVisible());
  dock_b->hide();
  QApplication::processEvents();
  CHECK(!b->scalingDialog()->isVisible());
  dock_b->show();
  QApplication::processEvents();
  CHECK(b->scalingDialog()->isVisible());
  b->scalingDialog()->hide();

  /* loading at the same time, without questions */
  const QString popup_before = QString::number(b->findChild<QWidget *>("panel_scales_controls")->findChildren<QDoubleSpinBox *>()[0]->value());
  CHECK(a->openSwath(a_path, 0));
  CHECK(b->openSwath(b_path, 0));
  CHECK(na.busy && nb.busy);
  CHECK(a->progressBar()->isVisible());
  CHECK(!a->openSwath(b_path, 0));
  wait_for({a, b});
  CHECK(asked == 0);
  CHECK(na.opened == 1 && nb.opened == 1);
  CHECK(na.opened_path == a_path);
  CHECK(na.opened_format == 261);
  CHECK(na.failed == 0);
  CHECK(na.busy_changes == 2 && !na.busy);
  CHECK(na.residuals == 1 && nb.residuals == 1);
  CHECK(na.edits == 1);
  CHECK(na.title.contains("embed_a.kmall") && a->title() == na.title);
  CHECK(nb.title.contains("embed_b.kmall"));
  CHECK(host.windowTitle() == "host");
  CHECK(same(a->residuals(), alone));
  CHECK(same(b->residuals(), alone));
  CHECK(!a->failed());
  /* the plots follow the depth of the new data, and so does the popup */
  CHECK(QString::number(b->findChild<QWidget *>("panel_scales_controls")->findChildren<QDoubleSpinBox *>()[0]->value()) != popup_before);
  CHECK(b->findChild<QWidget *>("panel_scales_controls")->findChildren<QDoubleSpinBox *>()[0]->value() == b->scalingDialog()->scale().maxdepth);

  /* a file that cannot be read leaves the tool as it was */
  CHECK(a->openSwath(dir + "/missing.kmall", 0));
  wait_for({a});
  CHECK(na.failed == 1 && na.failed_path == dir + "/missing.kmall");
  CHECK(na.opened == 1);
  CHECK(asked == 1);
  CHECK(a->failed());
  CHECK(same(a->residuals(), alone));

  /* an edit in one stays in that one */
  asked = 0;
  CHECK(a->activateDisplayProfile(0, 0.25));
  CHECK(na.unsaved && !nb.unsaved);
  CHECK(na.edits == 2 && nb.edits == 1);
  CHECK(a->residualsOutOfDate() && !b->residualsOutOfDate());
  a->reprocess();
  CHECK(na.busy);
  CHECK(!a->saveSwathSvp());
  CHECK(!a->saveResidualsAsOffsets());
  wait_for({a});
  CHECK(na.residuals == 2 && nb.residuals == 1);
  CHECK(na.busy_changes == 6 && !na.busy);
  CHECK(!a->residualsOutOfDate());
  CHECK(same(b->residuals(), alone));

  /* saving without file dialogs */
  const int statuses = na.statuses;
  const QString saved_edit = dir + "/embed_edit.svp";
  CHECK(a->saveEditProfile(saved_edit));
  CHECK(!na.unsaved);
  CHECK(na.saves == 1 && na.saved_path == saved_edit);
  CHECK(QFile::exists(saved_edit));
  CHECK(a->saveSwathSvp());
  CHECK(na.saves == 2 && na.saved_path.endsWith(".svp") && QFile::exists(na.saved_path));
  CHECK(a->saveResidualsAsOffsets());
  CHECK(na.saves == 4 && na.saved_path == a_path + ".sbao" && QFile::exists(na.saved_path));
  CHECK(QFile::exists(a_path + ".sbo"));
  CHECK(na.statuses == statuses + 3);
  CHECK(!a->saveEditProfile(dir + "/no_such_directory/edit.svp"));
  CHECK(asked == 1);
  CHECK(na.saves == 4);

  /* closing with unsaved edits */
  asked = 0;
  CHECK(a->activateDisplayProfile(1, 0.25));
  CHECK(na.unsaved);
  answer = QMessageBox::Cancel;
  CHECK(!a->prepareToClose());
  CHECK(asked == 1);
  CHECK(na.unsaved);
  answer = QMessageBox::Discard;
  CHECK(a->prepareToClose());
  CHECK(asked == 2);
  CHECK(b->prepareToClose());
  CHECK(asked == 2);

  /* removing a tool from the host with work under way waits for it */
  a->reprocess();
  CHECK(a->busy());
  delete dock_a;
  CHECK(same(b->residuals(), alone));

  printf("%d checks, %d failed\n", checks, failures);
  return failures != 0;
}
