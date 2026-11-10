/*--------------------------------------------------------------------
 *    The MB-system:  mbnavedit_qt_embed_test.cc  9/11/2026
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

/* The navigation editor as a widget inside another program's window, with no window of its
   own: what a future unified application will use. Two editors in docks of one main window.
   Arguments: two swath files and a third one. They are edited, so they must be scratch copies. */

#include <cmath>
#include <cstdio>
#include <functional>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QStatusBar>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include "mb_status.h"
#include "mbne_widget.h"

using namespace mbne;

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

struct Notes {
  int opened = 0, closed = 0, views = 0, edits = 0, saved = 0, close_requests = 0, busy_changes = 0, statuses = 0;
  bool busy = false, unsaved = false;
  long changes = -1, first = -1, last = -1;
  QString title, opened_path, closed_path, saved_path;
};

void watch(NavEditWidget *w, Notes &n) {
  QObject::connect(w, &NavEditWidget::titleChanged, [&n](const QString &t) { n.title = t; });
  QObject::connect(w, &NavEditWidget::unsavedChanged, [&n](bool u) { n.unsaved = u; });
  QObject::connect(w, &NavEditWidget::statusMessage, [&n](const QString &, int) { n.statuses++; });
  QObject::connect(w, &NavEditWidget::busyChanged, [&n](bool b) {
    n.busy = b;
    n.busy_changes++;
  });
  QObject::connect(w, &NavEditWidget::closeRequested, [&n] { n.close_requests++; });
  QObject::connect(w, &NavEditWidget::fileOpened, [&n](const QString &p, int) {
    n.opened++;
    n.opened_path = p;
  });
  QObject::connect(w, &NavEditWidget::fileClosed, [&n](const QString &p) {
    n.closed++;
    n.closed_path = p;
  });
  QObject::connect(w, &NavEditWidget::viewChanged, [&n](const QString &, long a, long b, double, double) {
    n.views++;
    n.first = a;
    n.last = b;
  });
  QObject::connect(w, &NavEditWidget::editsChanged, [&n](const QString &, long c) {
    n.edits++;
    n.changes = c;
  });
  QObject::connect(w, &NavEditWidget::editsSaved, [&n](const QString &p) {
    n.saved++;
    n.saved_path = p;
  });
}

void wait_for(NavEditWidget *w) {
  QElapsedTimer timer;
  timer.start();
  while (w->busy() && timer.elapsed() < 30000) {
    QApplication::processEvents(QEventLoop::AllEvents, 20);
    QThread::msleep(2);
  }
  QApplication::processEvents();
}

void click(QWidget &w, const QPoint &at) {
  QMouseEvent press(QEvent::MouseButtonPress, QPointF(at), QPointF(w.mapToGlobal(at)), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  QCoreApplication::sendEvent(&w, &press);
  QMouseEvent release(QEvent::MouseButtonRelease, QPointF(at), QPointF(w.mapToGlobal(at)), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
  QCoreApplication::sendEvent(&w, &release);
}

/* an edit: the sonar depth of the records around record 2 changes, and a spike in it is interpolated */
void edit(NavEditWidget *w) {
  Session &s = w->session();
  s.recordsForTest()[1].draft += 1.0;
  s.recordsForTest()[3].draft += 1.0;
  s.recordsForTest()[2].draft = 90.0;
  w->canvas()->setMode(Canvas::MODE_SELECT);
  const Record &r = s.records()[2];
  click(*w->canvas(), QPoint(r.pixel[2 * PLOT_DRAFT], r.pixel[2 * PLOT_DRAFT + 1]));
  w->findChild<QPushButton *>("interpolate")->click();
}

QAction *find_action(NavEditWidget *w, const QString &start) {
  for (QMenu *menu : w->menus())
    for (QAction *action : menu->actions())
      if (action->text().startsWith(start))
        return action;
  return nullptr;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s file1 file2 file3\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);
  const QString a_path = argv[1], b_path = argv[2], c_path = argv[3];
  const int format = 261;

  QMessageBox::StandardButton reply = QMessageBox::Cancel;
  int asked = 0;
  QTimer answers;
  answers.setInterval(5);
  QObject::connect(&answers, &QTimer::timeout, [&] {
    auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
    if (box == nullptr)
      return;
    asked++;
    QAbstractButton *button = box->button(reply);
    (button != nullptr ? button : box->buttons().front())->click();
  });
  answers.start();

  QMainWindow host;
  auto *a = new NavEditWidget(Presentation::Panel);
  auto *b = new NavEditWidget(Presentation::Panel);
  Notes na, nb;
  watch(a, na);
  watch(b, nb);
  a->setDisplayName("Navigation A");
  auto *dock_a = new QDockWidget("A");
  dock_a->setObjectName("A");
  dock_a->setWidget(a);
  auto *dock_b = new QDockWidget("B");
  dock_b->setObjectName("B");
  dock_b->setWidget(b);
  host.addDockWidget(Qt::LeftDockWidgetArea, dock_a);
  host.addDockWidget(Qt::RightDockWidgetArea, dock_b);
  host.resize(1600, 900);
  host.show();
  QApplication::processEvents();

  /* the panel has one toolbar and no window of its own */
  CHECK(a->findChild<QToolBar *>("panel_bar") != nullptr);
  CHECK(a->findChild<QToolButton *>("panel_menu") != nullptr);
  CHECK(a->findChild<QToolButton *>("panel_scales") != nullptr);
  CHECK(!a->findChild<QWidget *>("controls")->isVisible());
  CHECK(!a->isWindow());
  CHECK(a->minimumSizeHint().width() < 700);
  CHECK(!a->findChild<QPushButton *>("done")->isVisible());
  CHECK(!a->findChild<QPushButton *>("quit")->isVisible());
  CHECK(!find_action(a, "File Selection List")->isVisible());
  CHECK(a->title() == "Navigation A");
  CHECK(host.menuBar()->actions().isEmpty());

  /* two files at once, each in its own editor */
  CHECK(a->openFile(a_path, format));
  CHECK(a->busy());
  CHECK(!a->openFile(c_path, format));
  CHECK(!a->save());
  wait_for(a);
  CHECK(na.busy_changes == 2 && !na.busy);
  CHECK(b->openFile(b_path, format));
  wait_for(b);
  CHECK(a->session().isOpen() && b->session().isOpen());
  CHECK(na.opened == 1 && na.opened_path == a_path);
  CHECK(nb.opened == 1 && nb.opened_path == b_path);
  CHECK(na.title.contains(QFileInfo(a_path).fileName()) && na.title.contains("Navigation A"));
  CHECK(na.views >= 1 && na.last > na.first);
  const int n = a->session().nbuff();
  CHECK(na.changes == 0);
  const double original_depth = a->session().records()[1].draft;

  /* an edit in one leaves the other alone */
  edit(a);
  CHECK(na.unsaved);
  CHECK(!nb.unsaved);
  CHECK(na.changes >= 1);
  CHECK(nb.changes == 0);
  CHECK(a->session().records()[2].draft < 80.0);
  CHECK(b->session().records()[2].draft < 80.0);
  auto *undo_button = a->findChild<QToolBar *>("panel_bar");
  CHECK(undo_button != nullptr);
  QAction *undo = find_action(a, "Undo");
  CHECK(undo != nullptr && undo->text() == "Undo" && undo->toolTip() == "Undo Interpolate");
  undo->trigger();
  CHECK(!na.unsaved);
  CHECK(a->session().records()[2].draft == 90.0);
  find_action(a, "Redo")->trigger();
  CHECK(na.unsaved);

  /* shortcuts stay inside the panel: a key in one dock is not for the other */
  CHECK(undo->shortcutContext() == Qt::WidgetWithChildrenShortcut);
  CHECK(a->actions().contains(undo));

  /* the host moves the view */
  a->findChild<QPushButton *>("show_all")->click();
  const int views = na.views;
  a->session().data_show_size = 1;
  a->canvas()->layoutChanged();
  CHECK(a->goToTime(a->session().records()[1].time_d));
  CHECK(a->session().current() == 1);
  CHECK(na.views > views);
  CHECK(!a->goToTime(a->session().records()[n - 1].time_d + 1000.0));
  a->findChild<QPushButton *>("show_all")->click();

  /* a dialog goes and comes with the dock */
  find_action(a, "Position Offset")->trigger();
  auto *offset = a->findChild<QDialog *>("dialog_offset");
  CHECK(offset != nullptr && offset->isVisible());
  dock_a->hide();
  QApplication::processEvents();
  CHECK(!offset->isVisible());
  dock_a->show();
  QApplication::processEvents();
  CHECK(offset->isVisible());
  offset->hide();

  /* Save: one report, and closing with nothing new does not report it again */
  CHECK(a->save());
  CHECK(na.saved == 1 && na.saved_path == a_path);
  CHECK(!na.unsaved);
  CHECK(QFile::exists(a_path + ".nve"));

  /* the Quit button of the host's panel is hidden: the host closes it */
  asked = 0;
  CHECK(a->prepareToClose(nullptr));
  CHECK(asked == 0);
  CHECK(na.saved == 1);
  CHECK(na.closed == 1 && na.closed_path == a_path);
  CHECK(!a->session().isOpen());
  CHECK(a->title() == "Navigation A");

  /* the saved edits come back, without a question */
  CHECK(a->openFile(a_path, format, SavedEdits::Apply));
  wait_for(a);
  CHECK(asked == 0);
  CHECK(na.opened == 2);
  CHECK(std::fabs(a->session().records()[1].draft - original_depth - 1.0) < 0.001);

  /* and can be left out, without a question */
  CHECK(a->prepareToClose(nullptr));
  CHECK(a->openFile(a_path, format, SavedEdits::Ignore));
  wait_for(a);
  CHECK(asked == 0);
  CHECK(std::fabs(a->session().records()[1].draft - original_depth) < 0.001);

  /* a file open in another editor stays locked */
  {
    NavEditWidget third(Presentation::Panel);
    QString holder;
    third.setLockChoice([&](const QString &, const QString &who) {
      holder = who;
      return LockChoice::Refuse;
    });
    third.show();
    asked = 0;
    CHECK(third.openFile(a_path, format, SavedEdits::Ignore));
    wait_for(&third);
    for (int i = 0; i < 20 && asked == 0; i++)
      QApplication::processEvents(QEventLoop::AllEvents, 20);
    CHECK(holder.contains("mbnavedit-qt"));
    CHECK(!third.session().isOpen());
    CHECK(a->session().isOpen());
    CHECK(QFile::exists(a_path + ".lck"));
    CHECK(asked == 1);
  }

  /* a lock left by a crash */
  {
    QFile::copy(a_path + ".lck", c_path + ".lck");
    NavEditWidget third(Presentation::Panel);
    third.setLockChoice([](const QString &, const QString &) { return LockChoice::OpenAnyway; });
    third.show();
    CHECK(third.openFile(c_path, format, SavedEdits::Ignore));
    wait_for(&third);
    CHECK(third.session().isOpen());
    CHECK(third.prepareToClose(nullptr));
  }

  /* closing with unsaved edits: Cancel, then Discard, then Save */
  edit(b);
  CHECK(nb.unsaved);
  reply = QMessageBox::Cancel;
  CHECK(!b->prepareToClose(nullptr));
  CHECK(b->session().isOpen());
  reply = QMessageBox::Discard;
  CHECK(b->prepareToClose(nullptr));
  CHECK(!b->session().isOpen());
  CHECK(nb.closed == 1);
  CHECK(nb.saved == 0);
  CHECK(!QFile::exists(b_path + ".nve"));

  CHECK(b->openFile(b_path, format));
  wait_for(b);
  edit(b);
  reply = QMessageBox::Save;
  bool closed_later = false;
  CHECK(!b->prepareToClose([&closed_later] { closed_later = true; }));
  wait_for(b);
  QElapsedTimer timer;
  timer.start();
  while (!closed_later && timer.elapsed() < 20000)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  CHECK(closed_later);
  CHECK(nb.saved == 1 && nb.saved_path == b_path);
  CHECK(nb.closed == 2);
  CHECK(QFile::exists(b_path + ".nve"));

  /* a file that does not exist */
  asked = 0;
  reply = QMessageBox::Ok;
  CHECK(b->openFile("/nonexistent/file.mb261", format));
  wait_for(b);
  for (int i = 0; i < 40 && asked == 0; i++)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  CHECK(!b->session().isOpen());
  CHECK(asked >= 1);

  /* it would replace the saved edits, so this asks */
  reply = QMessageBox::Discard;
  asked = 0;
  CHECK(a->prepareToClose(nullptr));
  CHECK(asked == 1);
  CHECK(QFile::exists(a_path + ".nve"));
  answers.stop();
  printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
