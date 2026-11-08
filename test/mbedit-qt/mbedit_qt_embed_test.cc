/*--------------------------------------------------------------------
 *    The MB-system:  mbedit_qt_embed_test.cc  8/11/2026
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
 * The editor as a widget inside another program's window, with no window of its
 * own: what a future unified application will use. Two editors in docks of one
 * main window. Arguments: the MBIO format and three swath files. They are
 * edited, so they must be scratch copies.
 */

#include <QAbstractButton>
#include <QApplication>
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
#include <QTimer>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbe_editor.h"

using mbe::EditorWidget;
using mbe::SavedEdits;

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
  int opened = 0, closed = 0, saved = 0, views = 0, titles = 0, statuses = 0, close_requests = 0, busy = 0;
  long changes = -1, first = -1, last = -1;
  bool unsaved = false;
  QString title, opened_path;
  QStringList info;
};

void watch(EditorWidget *e, Notes &n) {
  QObject::connect(e, &EditorWidget::fileOpened, [&n](const QString &path, int) { n.opened++; n.opened_path = path; });
  QObject::connect(e, &EditorWidget::fileClosed, [&n](const QString &) { n.closed++; });
  QObject::connect(e, &EditorWidget::editsSaved, [&n](const QString &) { n.saved++; });
  QObject::connect(e, &EditorWidget::editsChanged, [&n](const QString &, long changes) { n.changes = changes; });
  QObject::connect(e, &EditorWidget::viewChanged, [&n](const QString &, long first, long last, double, double) { n.views++; n.first = first; n.last = last; });
  QObject::connect(e, &EditorWidget::titleChanged, [&n](const QString &title) { n.titles++; n.title = title; });
  QObject::connect(e, &EditorWidget::statusMessage, [&n](const QString &, int) { n.statuses++; });
  QObject::connect(e, &EditorWidget::closeRequested, [&n] { n.close_requests++; });
  QObject::connect(e, &EditorWidget::busyChanged, [&n](bool) { n.busy++; });
  QObject::connect(e, &EditorWidget::unsavedChanged, [&n](bool unsaved) { n.unsaved = unsaved; });
  QObject::connect(e, &EditorWidget::soundingInfo, [&n](const QStringList &lines) { n.info = lines; });
}

void wait_for(std::initializer_list<EditorWidget *> editors) {
  QElapsedTimer timer;
  timer.start();
  bool busy = true;
  while (busy && timer.elapsed() < 60000) {
    QApplication::processEvents(QEventLoop::AllEvents, 20);
    busy = false;
    for (EditorWidget *e : editors)
      busy = busy || e->busy();
  }
  QApplication::processEvents();
}

void press(EditorWidget *e, const char *name) {
  e->findChild<QPushButton *>(name)->click();
  QApplication::processEvents();
}

void edit(EditorWidget *e) {
  for (int i = 0; i < 4; i++) {
    press(e, "button_forward");
    press(e, "button_flag_view");
  }
}

QAction *find_action(EditorWidget *e, const QString &text) {
  for (QMenu *menu : e->menus())
    for (QAction *action : menu->actions())
      if (action->text() == text)
        return action;
  return nullptr;
}

std::string edits_of(const std::string &path) {
  std::ifstream in(path + ".esf", std::ios::binary);
  const std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  return data.size() > MB_PATH_MAXLINE ? data.substr(MB_PATH_MAXLINE) : std::string();
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "usage: mbedit_qt_embed_test format file1 file2 file3\n");
    return 2;
  }
  QApplication app(argc, argv);
  const int format = atoi(argv[1]);
  const QString a_path = argv[2], b_path = argv[3], c_path = argv[4];
  const char *history = getenv(MB_HISTORY_ENV);
  const std::string history_before = history != nullptr ? history : "(unset)";

  /* what a program with several editors has to do */
  int error = MB_ERROR_NO_ERROR;
  mb_mem_list_disable(0, &error);

  /* every question is answered with this, and counted */
  QMessageBox::StandardButton answer = QMessageBox::Save;
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
    if (button != nullptr)
      button->click();
  });
  answers.start();

  /* the edits of one editor alone, for comparison */
  std::string alone;
  {
    EditorWidget solo;
    solo.show();
    CHECK(solo.openFile(c_path, format, SavedEdits::Ignore));
    wait_for({&solo});
    edit(&solo);
    CHECK(solo.save());
    CHECK(solo.prepareToClose(nullptr));
    alone = edits_of(c_path.toStdString());
    CHECK(!alone.empty());
  }

  /* two editors in docks of one main window */
  QMainWindow host;
  host.setWindowTitle("host");
  auto *a = new EditorWidget;
  auto *b = new EditorWidget;
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
  a->setDisplayName("Swath Editor");
  host.show();

  /* nothing of a window of its own */
  CHECK(host.findChild<QMenuBar *>() == nullptr);
  CHECK(host.findChild<QStatusBar *>() == nullptr);
  CHECK(a->menus().size() == 6);
  CHECK(find_action(a, "Quit") == nullptr);
  CHECK(find_action(a, "Save")->shortcutContext() == Qt::WidgetWithChildrenShortcut);
  CHECK(find_action(b, "Save")->shortcutContext() == Qt::WindowShortcut);
  CHECK(a->title().startsWith("Swath Editor"));

  /* loading at the same time, without questions */
  asked = 0;
  CHECK(a->openFile(a_path, format, SavedEdits::Ignore));
  CHECK(b->openFile(b_path, format, SavedEdits::Ignore));
  wait_for({a, b});
  CHECK(asked == 0);
  CHECK(na.opened == 1 && nb.opened == 1);
  CHECK(na.opened_path == a_path);
  CHECK(na.title.contains("Swath Editor") && na.title.contains(QFileInfo(a_path).fileName()));
  CHECK(na.statuses > 0 && na.busy >= 2);
  CHECK(na.views > 0);
  CHECK(host.windowTitle() == "host");
  const int npings = a->session().nbuff();
  CHECK(npings > 20);

  /* moving from outside */
  const mbe::Ping target = a->session().pings()[npings / 2];
  CHECK(a->goToTime(target.time_d));
  CHECK(na.first <= target.record && target.record <= na.last);
  const mbe::Ping other = a->session().pings()[3];
  CHECK(a->showRecord(other.record));
  CHECK(na.first <= other.record && other.record <= na.last);
  CHECK(!a->showRecord(1000000));
  CHECK(!a->goToTime(a->session().pings().front().time_d - 1000.0));
  CHECK(a->showRecord(a->session().pings().front().record));
  CHECK(b->showRecord(b->session().pings().front().record));

  /* the same edits in both give the edits of one editor alone */
  edit(a);
  edit(b);
  CHECK(na.unsaved && nb.unsaved);
  CHECK(na.changes > 0);
  CHECK(a->save() && b->save());
  CHECK(!na.unsaved);
  CHECK(na.saved == 1 && nb.saved == 1);
  CHECK(edits_of(a_path.toStdString()) == alone);
  CHECK(edits_of(b_path.toStdString()) == alone);

  /* a click in Info mode */
  {
    press(b, "button_unflag_view");
    b->canvas()->setMode(mbe::Canvas::MODE_INFO);
    const mbe::Session &s = b->session();
    const mbe::Ping &p = s.pings()[s.current()];
    int beam = -1;
    for (int j = 0; j < p.beams_bath && beam < 0; j++)
      if (mb_beam_ok(p.beamflag[j]))
        beam = j;
    CHECK(beam >= 0);
    const QPointF at(p.bath_x[std::max(beam, 0)], p.bath_y[std::max(beam, 0)]);
    QMouseEvent press_event(QEvent::MouseButtonPress, at, b->canvas()->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(b->canvas(), &press_event);
    QMouseEvent release_event(QEvent::MouseButtonRelease, at, b->canvas()->mapToGlobal(at), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(b->canvas(), &release_event);
    CHECK(!nb.info.isEmpty());
    b->canvas()->setMode(mbe::Canvas::MODE_TOGGLE);
  }

  /* the Quit button asks, and the editor stays */
  press(a, "button_quit");
  CHECK(na.close_requests == 1);
  CHECK(a->session().isOpen());

  /* other tools are opened by the surrounding program */
  QString tool, tool_path;
  int tool_format = -2;
  long tool_ping = -2;
  a->setToolOpener([&](const QString &name, const QString &path, int form, long ping) {
    tool = name;
    tool_path = path;
    tool_format = form;
    tool_ping = ping;
    return true;
  });
  QAction *water = find_action(a, "Water Column at Center Ping");
  CHECK(water != nullptr && water->isVisible());
  water->trigger();
  CHECK(tool == "mbwcdview" && tool_ping >= 0);
  find_action(a, "Save and Open in mbeditviz")->trigger();
  wait_for({a});
  QApplication::processEvents();
  CHECK(tool == "mbeditviz" && tool_path == a_path && tool_format == format && tool_ping == -1);
  CHECK(!a->session().isOpen());
  CHECK(na.closed == 1);

  /* the saved edits come back, without a question */
  asked = 0;
  CHECK(a->openFile(a_path, format, SavedEdits::Apply));
  wait_for({a});
  CHECK(asked == 0);
  CHECK(na.opened == 2);
  CHECK(a->session().isOpen());

  /* a file open in another editor stays locked */
  {
    EditorWidget third;
    QString holder;
    third.setLockChoice([&](const QString &, const QString &who) {
      holder = who;
      return mbe::LockChoice::Refuse;
    });
    third.setProgramDirectory("/nonexistent");
    third.show();
    asked = 0;
    CHECK(third.openFile(a_path, format, SavedEdits::Ignore));
    wait_for({&third});
    for (int i = 0; i < 20 && asked == 0; i++) {
      QApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    CHECK(holder.contains("mbedit-qt"));
    CHECK(!third.session().isOpen());
    CHECK(a->session().isOpen());
    CHECK(QFile::exists(a_path + ".lck"));
    CHECK(asked == 1);
  }

  /* a lock left by a crash */
  {
    QFile::copy(a_path + ".lck", c_path + ".lck");
    EditorWidget third;
    third.setLockChoice([](const QString &, const QString &) { return mbe::LockChoice::OpenAnyway; });
    third.show();
    CHECK(third.openFile(c_path, format, SavedEdits::Ignore));
    wait_for({&third});
    CHECK(third.session().isOpen());
    /* the file has old edits that would be replaced, so this asks */
    answer = QMessageBox::Discard;
    CHECK(third.prepareToClose(nullptr));
  }

  /* closing with unsaved edits: Save, then Cancel and Discard */
  press(a, "button_forward");
  press(a, "button_unflag_view");
  answer = QMessageBox::Save;
  bool closed_later = false;
  const int saved_before = na.saved;
  CHECK(!a->prepareToClose([&closed_later] { closed_later = true; }));
  wait_for({a});
  QElapsedTimer timer;
  timer.start();
  while (!closed_later && timer.elapsed() < 20000)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  CHECK(closed_later);
  CHECK(!a->session().isOpen());
  CHECK(na.saved == saved_before + 1);
  CHECK(na.closed == 2);

  edit(b);
  answer = QMessageBox::Cancel;
  CHECK(!b->prepareToClose(nullptr));
  CHECK(b->session().isOpen());
  answer = QMessageBox::Discard;
  CHECK(b->prepareToClose(nullptr));
  CHECK(!b->session().isOpen());
  CHECK(nb.closed == 1);

  history = getenv(MB_HISTORY_ENV);
  CHECK((history != nullptr ? history : "(unset)") == history_before);

  answers.stop();
  printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
