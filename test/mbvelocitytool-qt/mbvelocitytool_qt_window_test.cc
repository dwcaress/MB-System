/*--------------------------------------------------------------------
 *    The MB-system:  mbvelocitytool_qt_window_test.cc  2/11/2026
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
 * Drives the main window of mbvelocitytool-qt with mouse events: editing a
 * profile, the menus of the profile plot, activating display profiles and
 * Reprocess. Every result of Reprocess is compared with the same calculation made
 * directly. Arguments: a swath file with stored profiles of more than 50 nodes,
 * another swath file, and a directory to write in. Whether mblevitus is
 * available does not matter.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include <QApplication>
#include <QContextMenuEvent>
#include <QDoubleSpinBox>
#include <QImage>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QRadioButton>
#include <QSlider>
#include <QTimer>

#include "MainWindow.h"
#include "mb_define.h"
#include "mb_process.h"

using mbvt::ActivateDialog;
using mbvt::MainWindow;

namespace {

int checks = 0, failures = 0;

#define CHECK(condition)                                         \
  do {                                                           \
    checks++;                                                    \
    if (!(condition)) {                                          \
      failures++;                                                \
      printf("FAIL line %d: %s\n", __LINE__, #condition);        \
    }                                                            \
  } while (0)

const int TIME_BEGIN[7] = {1962, 2, 21, 10, 30, 0, 0};
const int TIME_END[7] = {2062, 2, 21, 10, 30, 0, 0};

void mouse(QWidget *w, QEvent::Type type, const QPointF &p, Qt::MouseButton button, Qt::MouseButtons buttons) {
  QMouseEvent event(type, p, w->mapToGlobal(p), button, buttons, Qt::NoModifier);
  QApplication::sendEvent(w, &event);
}

void drag(QWidget *w, const QPointF &from, const QPointF &to) {
  mouse(w, QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
  mouse(w, QEvent::MouseMove, (from + to) / 2.0, Qt::NoButton, Qt::LeftButton);
  mouse(w, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
  mouse(w, QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
}

/* runs check on the menu that a right click at p opens */
bool right_click(mbvt::VelocityPlot *plot, const QPointF &p, const std::function<void(QMenu *)> &check) {
  bool opened = false;
  QTimer::singleShot(100, [&] {
    if (auto *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
      opened = true;
      check(menu);
      if (QApplication::activePopupWidget() == menu)
        menu->close();
    }
  });
  const QPoint point = p.toPoint();
  QContextMenuEvent event(QContextMenuEvent::Mouse, point, plot->mapToGlobal(point));
  QApplication::sendEvent(plot, &event);
  return opened;
}

QAction *action_named(QMenu *menu, const QString &text) {
  for (QAction *action : menu->actions())
    if (action->text() == text)
      return action;
  return nullptr;
}

/* answers the dialog of Activate, which is modal: 0 cancels, 1 takes all nodes, 2 takes the simplified profile */
void answer_activate_dialog(int answer) {
  QTimer::singleShot(300, [answer] {
    auto *dialog = qobject_cast<ActivateDialog *>(QApplication::activeModalWidget());
    CHECK(dialog != nullptr);
    if (dialog == nullptr)
      return;
    if (answer == 2)
      dialog->findChildren<QRadioButton *>()[1]->setChecked(true);
    if (answer == 0)
      dialog->reject();
    else
      dialog->accept();
  });
}

bool same_result(const mbvt::Residuals &a, const mbvt::Residuals &b) {
  return a.nbeams == b.nbeams && a.residual == b.residual && a.sigma == b.sigma && a.count == b.count &&
         a.acrosstrack == b.acrosstrack && a.rays_traced == b.rays_traced;
}

bool reprocess_is_direct(MainWindow &w, const mbvt::SwathFile &swath, int anglemode) {
  w.reprocess();
  w.waitUntilIdle();
  mbvt::Residuals direct;
  std::string error;
  return !w.residualsOutOfDate() && mbvt::process(swath, w.editProfile(), anglemode, 1, direct, error) &&
         same_result(direct, w.residuals());
}

/* index of a display profile by its number of nodes among those stored in the file, or -1 */
int stored_index(const MainWindow &w, int nodes) {
  const std::vector<mbvt::Profile> &display = w.displayProfiles();
  for (int i = 0; i < (int)display.size(); i++)
    if (display[i].source == mbvt::ProfileSource::SwathRecord && display[i].size() == nodes)
      return i;
  return -1;
}

void set_scale(MainWindow &w, double maxdepth, double velrange, double velcenter) {
  const auto spins = w.scalingDialog()->findChildren<QDoubleSpinBox *>();
  spins[0]->setValue(maxdepth);
  spins[1]->setValue(velrange);
  spins[2]->setValue(velcenter);
  QApplication::processEvents();
}

/*--------------------------------------------------------------------*/
void test_editing(const std::string &file, const mbvt::SwathFile &swath) {
  std::string error;
  MainWindow w(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, true);
  w.resize(1100, 750);
  w.show();
  auto *plot = w.velocityPlot();

  /* no data and no profile: nothing to edit */
  drag(plot, QPointF(300, 100), QPointF(320, 120));
  CHECK(!plot->addNodeAt(QPointF(300, 100)));
  CHECK(!plot->deleteNode(0));

  w.openSwath(QString::fromStdString(file), 0);
  CHECK(w.busy());
  w.waitUntilIdle();
  CHECK(!w.failed());
  CHECK(w.editProfile().size() == 6);
  mbvt::Residuals direct;
  CHECK(mbvt::process(swath, w.editProfile(), MBP_ANGLES_SNELL, 1, direct, error));
  CHECK(same_result(direct, w.residuals()));
  CHECK(!w.residualsOutOfDate());

  /* the dialog and the plot agree */
  mbvt::Axes a = plot->axes();
  CHECK(std::abs(w.scalingDialog()->scale().maxdepth - a.y1) < 0.51);
  set_scale(w, 2500.0, 200.0, 1500.0);
  a = plot->axes();
  CHECK(a.y1 == 2500.0 && a.x0 == 1400.0 && a.x1 == 1600.0);
  const auto sliders = w.scalingDialog()->findChildren<QSlider *>();
  CHECK(sliders[0]->value() == 2500);
  sliders[3]->setValue(100);
  CHECK(std::abs(w.scalingDialog()->scale().resrange - 10.0) < 1.0e-9);

  const auto node = [&](int i) { return QPointF(a.px(w.editProfile().velocity[i]), a.py(w.editProfile().depth[i])); };

  /* a click on a node without moving it, or away from any node, changes nothing */
  mouse(plot, QEvent::MouseButtonPress, node(1), Qt::LeftButton, Qt::LeftButton);
  mouse(plot, QEvent::MouseButtonRelease, node(1), Qt::LeftButton, Qt::NoButton);
  mbvt::Profile before = w.editProfile();
  drag(plot, QPointF(a.px(1590.0), a.py(1200.0)), QPointF(a.px(1590.0), a.py(1400.0)));
  CHECK(w.editProfile().depth == before.depth && w.editProfile().velocity == before.velocity);
  CHECK(!w.residualsOutOfDate());
  CHECK(!w.editModified());

  /* node 1 goes where it is dragged */
  const QPointF target(a.px(1520.0), (a.py(before.depth[0]) + a.py(before.depth[2])) / 2.0);
  drag(plot, node(1), target);
  CHECK(std::abs(w.editProfile().velocity[1] - 1520.0) < 1.0e-6);
  CHECK(std::abs(w.editProfile().depth[1] - a.vy(target.y())) < 1.0e-6);
  CHECK(w.residualsOutOfDate());
  CHECK(w.editModified());

  /* the first node keeps its depth */
  before = w.editProfile();
  drag(plot, node(0), QPointF(a.px(1490.0), a.py(300.0)));
  CHECK(w.editProfile().depth[0] == before.depth[0]);
  CHECK(std::abs(w.editProfile().velocity[0] - 1490.0) < 1.0e-6);

  /* a node stays above the next one and inside the plot */
  before = w.editProfile();
  drag(plot, node(1), QPointF(a.area.right() + 200.0, a.area.bottom() + 200.0));
  CHECK(w.editProfile().depth[1] < before.depth[2]);
  CHECK(std::abs(w.editProfile().velocity[1] - a.x1) < 1.0e-6);
  const double middle = (w.editProfile().depth[0] + w.editProfile().depth[2]) / 2.0;
  drag(plot, node(1), QPointF(a.px(1510.0), a.py(middle)));
  CHECK(std::abs(w.editProfile().depth[1] - middle) < 1.0);

  /* adding, in depth order */
  const int n = w.editProfile().size();
  const double below = w.editProfile().depth[2] - w.editProfile().depth[1];
  QPointF p(a.px(1450.0), a.py(w.editProfile().depth[1] + 0.25 * below));
  mouse(plot, QEvent::MouseButtonDblClick, p, Qt::LeftButton, Qt::LeftButton);
  CHECK(w.editProfile().size() == n + 1);
  CHECK(std::abs(w.editProfile().velocity[2] - 1450.0) < 1.0e-6);
  p = QPointF(a.px(1460.0), a.py(w.editProfile().depth[1] + 0.5 * below));
  mouse(plot, QEvent::MouseButtonPress, p, Qt::MiddleButton, Qt::MiddleButton);
  mouse(plot, QEvent::MouseButtonRelease, p, Qt::MiddleButton, Qt::NoButton);
  CHECK(w.editProfile().size() == n + 2);
  for (int i = 1; i < w.editProfile().size(); i++)
    CHECK(w.editProfile().depth[i] > w.editProfile().depth[i - 1]);
  mouse(plot, QEvent::MouseButtonDblClick, node(2), Qt::LeftButton, Qt::LeftButton);
  CHECK(w.editProfile().size() == n + 2);
  CHECK(!plot->addNodeAt(QPointF(2.0, 2.0)));

  /* the menu on a node deletes it, the menu elsewhere adds one */
  CHECK(right_click(plot, node(2), [](QMenu *menu) { action_named(menu, "Delete Node")->trigger(); }));
  CHECK(w.editProfile().size() == n + 1);
  p = QPointF(a.px(1590.0), a.py(w.editProfile().depth[1] + 0.75 * below));
  CHECK(right_click(plot, p, [](QMenu *menu) { action_named(menu, "Add Node Here")->trigger(); }));
  CHECK(w.editProfile().size() == n + 2);

  /* Reprocess uses the edited profile and the angle mode */
  CHECK(w.residualsOutOfDate());
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_SNELL));
  w.setAngleMode(MBP_ANGLES_OK);
  CHECK(w.residualsOutOfDate());
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_OK));
  w.setAngleMode(MBP_ANGLES_SNELLNULL);
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_SNELLNULL));

  /* an edit during Reprocess leaves the residuals of the profile as it was, marked out of date */
  const mbvt::Profile processed = w.editProfile();
  w.reprocess();
  CHECK(w.busy());
  CHECK(plot->deleteNode(1));
  w.waitUntilIdle();
  CHECK(w.residualsOutOfDate());
  CHECK(mbvt::process(swath, processed, MBP_ANGLES_SNELLNULL, 1, direct, error));
  CHECK(same_result(direct, w.residuals()));

  /* two nodes always stay */
  while (w.editProfile().size() > 2)
    CHECK(plot->deleteNode(1));
  CHECK(!plot->deleteNode(0));

  /* redraw at any size, with the image of the whole window opaque */
  for (const QSize &size : {QSize(640, 480), QSize(1016, 668), QSize(1800, 1100)}) {
    w.resize(size);
    QApplication::processEvents();
    CHECK(w.size() == size);
    CHECK(!w.grab().isNull());
  }

  /* bad input leaves everything as it was */
  CHECK(!w.openEditProfile("/nonexistent/file.svp"));
  CHECK(w.failed());
  CHECK(!w.openDisplayProfile("/nonexistent/file.svp"));
  CHECK(w.editProfile().size() == 2);
  w.openSwath("/nonexistent/name_without_format", 0);
  CHECK(!w.busy());
}

/*--------------------------------------------------------------------*/
void test_activate(const std::string &file, const std::string &other_file, const mbvt::SwathFile &swath, const std::string &dir) {
  std::string error;
  const int deep_nodes = swath.profiles[0].size();
  const int other_nodes = swath.profiles[1].size();
  MainWindow w(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, true);
  w.resize(1400, 900);
  w.show();
  w.openSwath(QString::fromStdString(file), 0);
  w.waitUntilIdle();
  auto *plot = w.velocityPlot();
  CHECK(w.editProfile().source == mbvt::ProfileSource::Default);
  set_scale(w, 900.0, 60.0, 1500.0);
  const mbvt::Axes a = plot->axes();

  const int first = stored_index(w, deep_nodes);
  const int second = stored_index(w, other_nodes);
  CHECK(first >= 0 && second >= 0);

  /* find a place on the second profile away from the others, and one where they cross */
  const mbvt::Profile &line = w.displayProfiles()[second];
  QPointF alone(-1.0, -1.0), together(-1.0, -1.0);
  for (int i = 1; i + 2 < line.size(); i++) {
    const QPointF p(a.px((line.velocity[i] + line.velocity[i + 1]) / 2.0), a.py((line.depth[i] + line.depth[i + 1]) / 2.0));
    const std::vector<int> found = plot->profilesAt(p);
    if (found.empty() || found[0] != second || !a.area.contains(p))
      continue;
    if (found.size() == 1 && alone.x() < 0.0)
      alone = p;
    if (found.size() >= 2 && together.x() < 0.0)
      together = p;
  }
  CHECK(alone.x() >= 0.0 && together.x() >= 0.0);
  CHECK(plot->profilesAt(QPointF(2.0, 2.0)).empty());

  /* the menu of one profile */
  const std::string second_name = line.name;
  QStringList texts;
  CHECK(right_click(plot, alone, [&](QMenu *menu) {
    for (QAction *action : menu->actions())
      texts << action->text();
  }));
  CHECK(texts.size() >= 7 && texts[0] == QString::fromStdString(second_name) && texts[1] == "Activate as Editing Profile");
  CHECK(texts.contains("Hide") && texts.contains("Remove") && texts.contains("Save Profile As...") && texts.contains("Add Node Here"));

  /* the menu of several: a submenu each, the nearest first */
  int submenus = 0;
  texts.clear();
  CHECK(right_click(plot, together, [&](QMenu *menu) {
    for (QAction *action : menu->actions()) {
      texts << action->text();
      submenus += action->menu() != nullptr ? 1 : 0;
    }
  }));
  CHECK(submenus >= 2 && texts[0] == QString::fromStdString(second_name));

  /* Cancel in the dialog changes nothing */
  const size_t ndisplay = w.displayProfiles().size();
  /* in the menu of the nearest profile, which is a submenu when others lie close */
  const auto activate = [](QMenu *menu) {
    QAction *action = action_named(menu, "Activate as Editing Profile");
    if (action == nullptr)
      action = action_named(menu->actions()[0]->menu(), "Activate as Editing Profile");
    action->trigger();
  };
  answer_activate_dialog(0);
  CHECK(right_click(plot, alone, activate));
  CHECK(w.editProfile().source == mbvt::ProfileSource::Default);
  CHECK(!w.residualsOutOfDate());

  /* all nodes: the same profile; the original stays; the plain default is not kept; Reprocess is still up to the user */
  answer_activate_dialog(1);
  CHECK(right_click(plot, alone, activate));
  CHECK(mbvt::same_nodes(w.editProfile(), swath.profiles[1], 0.0));
  CHECK(w.displayProfiles().size() == ndisplay);
  CHECK(!w.editModified());
  CHECK(w.residualsOutOfDate());
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_SNELL));
  const mbvt::Residuals original = w.residuals();

  /* its nodes can be moved and removed, and Reprocess applies that; the original is untouched */
  int node = -1;
  for (int i = 0; i < w.editProfile().size() && node < 0; i++)
    if (w.editProfile().depth[i] > 300.0)
      node = i;
  const double velocity = w.editProfile().velocity[node];
  const QPointF from(a.px(velocity), a.py(w.editProfile().depth[node]));
  const int picked = plot->nodeAt(from);
  CHECK(picked >= 0);
  drag(plot, from, from + QPointF(40.0, 0.0));
  CHECK(std::abs(w.editProfile().velocity[picked] - swath.profiles[1].velocity[picked]) > 0.5);
  CHECK(w.editModified());
  CHECK(plot->deleteNode(picked + 3));
  CHECK(w.editProfile().size() == other_nodes - 1);
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_SNELL));
  CHECK(!same_result(original, w.residuals()));
  CHECK(mbvt::same_nodes(w.displayProfiles()[second], swath.profiles[1], 0.0));

  /* activating another: the edited one joins the display profiles and stays marked unsaved */
  const mbvt::Profile edited = w.editProfile();
  CHECK(w.activateDisplayProfile(first, 0.0));
  CHECK(mbvt::same_nodes(w.editProfile(), swath.profiles[0], 0.0));
  CHECK(!w.editModified());
  CHECK(w.displayProfiles().size() == ndisplay + 1);
  const int kept = (int)ndisplay;
  CHECK(w.displayProfiles()[kept].source == mbvt::ProfileSource::Edited && w.displayProfiles()[kept].unsaved);
  CHECK(mbvt::same_nodes(w.displayProfiles()[kept], edited, 0.0));
  CHECK(w.displayProfiles()[kept].name.find("(edited)") != std::string::npos);

  /* and back: no copies pile up, and the changes are still known to be unsaved */
  CHECK(w.activateDisplayProfile(kept, 0.0));
  CHECK(w.editModified());
  CHECK(mbvt::same_nodes(w.editProfile(), edited, 0.0));
  CHECK(w.displayProfiles().size() == ndisplay + 1);
  CHECK(w.activateDisplayProfile(first, 0.0));
  CHECK(w.displayProfiles().size() == ndisplay + 1);
  CHECK(w.displayProfiles()[kept].unsaved);

  /* a menu left open while the profiles changed does nothing */
  const mbvt::Profile current = w.editProfile();
  emit plot->profileAction(second, "another name", mbvt::ProfileAction::Activate);
  CHECK(mbvt::same_nodes(current, w.editProfile(), 0.0));

  /* hide, show */
  w.hideDisplayProfile(second);
  CHECK(w.displayProfiles()[second].hidden);
  const std::vector<int> visible = plot->profilesAt(alone);
  CHECK(std::find(visible.begin(), visible.end(), second) == visible.end());
  CHECK(w.displayProfiles().size() == ndisplay + 1);
  CHECK(right_click(plot, alone, [](QMenu *menu) { action_named(menu, "Show Hidden Profiles")->trigger(); }));
  CHECK(!w.displayProfiles()[second].hidden);
  CHECK(plot->profilesAt(alone)[0] == second);

  /* simplified through the dialog: fewer nodes, unsaved */
  answer_activate_dialog(2);
  CHECK(right_click(plot, alone, activate));
  CHECK(w.editProfile().size() < other_nodes && w.editProfile().size() >= 2);
  CHECK(w.editModified());
  CHECK(w.editProfile().name.find("simplified") != std::string::npos);
  CHECK(reprocess_is_direct(w, swath, MBP_ANGLES_SNELL));

  /* remove */
  w.removeDisplayProfile(second);
  CHECK(w.displayProfiles().size() == ndisplay);
  w.removeDisplayProfile(99);
  CHECK(w.displayProfiles().size() == ndisplay);

  /* a new swath file replaces the profiles of the old one, not the edited ones */
  w.openSwath(QString::fromStdString(other_file), 0);
  w.waitUntilIdle();
  CHECK(stored_index(w, deep_nodes) < 0);
  CHECK(stored_index(w, 2) >= 0);
  bool edited_kept = false;
  for (const mbvt::Profile &p : w.displayProfiles())
    edited_kept = edited_kept || (p.source == mbvt::ProfileSource::Edited && mbvt::same_nodes(p, edited, 0.0));
  CHECK(edited_kept);

  /* the image of --export has no transparent part */
  const QString image = QString::fromStdString(dir + "/window_test.png");
  CHECK(w.saveImage(image, 800, 600));
  const QImage saved(image);
  CHECK(saved.width() == 800 && saved.height() == 600);
  CHECK(qAlpha(saved.pixel(2, 2)) == 255);
  w.close();
}

/*--------------------------------------------------------------------*/
void test_simplify(const mbvt::SwathFile &swath) {
  for (const mbvt::Profile &profile : swath.profiles)
    for (const double tolerance : {0.01, 0.25, 1.0, 5.0, 100.0}) {
      const mbvt::Profile simple = mbvt::simplify_profile(profile, tolerance);
      CHECK(simple.size() >= 2 && simple.size() <= profile.size());
      CHECK(simple.depth.front() == profile.depth.front() && simple.depth.back() == profile.depth.back());
      CHECK(simple.velocity.front() == profile.velocity.front() && simple.velocity.back() == profile.velocity.back());
      size_t k = 0;
      double worst = 0.0;
      for (int i = 0; i < profile.size(); i++) {
        while (k + 2 < simple.depth.size() && simple.depth[k + 1] < profile.depth[i])
          k++;
        const double t = (profile.depth[i] - simple.depth[k]) / (simple.depth[k + 1] - simple.depth[k]);
        worst = std::max(worst, std::abs(profile.velocity[i] - (simple.velocity[k] + t * (simple.velocity[k + 1] - simple.velocity[k]))));
      }
      CHECK(worst <= tolerance + 1.0e-9);
    }

  /* very short and repeated depths */
  mbvt::Profile small;
  small.depth = {0.0, 10.0};
  small.velocity = {1500.0, 1490.0};
  CHECK(mbvt::same_nodes(mbvt::simplify_profile(small, 1.0), small, 0.0));
  mbvt::Profile untidy;
  untidy.depth = {10.0, 10.0, 10.0};
  untidy.velocity = {1500.0, 1600.0, 1500.0};
  CHECK(mbvt::simplify_profile(untidy, 1.0).size() == 3);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: mbvelocitytool_qt_window_test file_with_long_profiles other_file directory\n");
    return 2;
  }
  QApplication app(argc, argv);
  const std::string file = argv[1], other_file = argv[2], dir = argv[3];

  mbvt::SwathFile swath;
  std::string warning, error;
  CHECK(mbvt::load_swath(file, 0, TIME_BEGIN, TIME_END, 0, swath, warning, error));
  CHECK(swath.profiles.size() == 3 && swath.profiles[0].size() > 50 && swath.profiles[1].size() > 50);
  if (failures > 0) {
    printf("%s\n%d checks, %d failed\n", error.c_str(), checks, failures);
    return 1;
  }

  test_editing(file, swath);
  test_activate(file, other_file, swath, dir);
  test_simplify(swath);

  /* quitting with work under way waits for it */
  {
    auto *w = new MainWindow(0, 0, MBP_ANGLES_SNELL, TIME_BEGIN, TIME_END, true);
    w->openSwath(QString::fromStdString(file), 0);
    delete w;
    checks++;
  }

  printf("%d checks, %d failed\n", checks, failures);
  return failures != 0;
}
