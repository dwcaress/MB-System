/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_qt_embed_test.cc  10/11/2026
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
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Another program opens the whole mbnavadjust window, as the unified
 * application will, and works with it through the calls and signals of the
 * controller. Runs without a display.
 * usage: mbnavadjust_qt_embed_test project.nvh
 * The project needs at least three crossings.
 */

#include <cstdio>
#include <memory>

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QTableView>

#include "NavAdjustWindow.h"
#include "mbnaq_controller.h"
#include "mbnaq_core.h"
#include "mbnaq_plots.h"
#include "mbnaq_terminal.h"
#include "mbnaq_widgets.h"

using namespace mbnaq;

static int failures = 0;

static void check(bool ok, const char *what) {
  printf("%s %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok)
    failures++;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s project.nvh\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);

  QMainWindow host;
  host.setWindowTitle("A host program");
  host.resize(800, 500);
  host.show();

  ProjectController *controller = ProjectController::create();
  check(controller != nullptr, "the host can make the controller");
  check(ProjectController::create() == nullptr, "a second controller is refused");
  check(!OutputCapture::instance().active(), "the terminal is captured only if the host asks");
  Session &session = controller->session();

  int opened = 0, closed = 0, selections = 0, ties = 0, lastTies = -1, inversions = 0, lastInversion = -1;
  QObject::connect(controller, &ProjectController::projectOpened, [&] { opened++; });
  QObject::connect(controller, &ProjectController::projectClosed, [&] { closed++; });
  QObject::connect(controller, &ProjectController::selectionChanged, [&] { selections++; });
  QObject::connect(controller, &ProjectController::tiesChanged, [&](int n) {
    ties++;
    lastTies = n;
  });
  QObject::connect(controller, &ProjectController::inversionChanged, [&](int status) {
    inversions++;
    lastInversion = status;
  });

  {
    /* the whole window, as a window of its own */
    NavAdjustWindow window(*controller);
    int windowClosed = 0;
    QObject::connect(&window, &NavAdjustWindow::windowClosed, [&] { windowClosed++; });
    window.show();
    QApplication::processEvents();
    check(window.browser() != nullptr && window.editor() != nullptr && window.modelPlot() != nullptr, "the window has all its parts");
    check(window.findChild<QDockWidget *>("logDock") != nullptr && window.findChild<QDockWidget *>("terminalDock") != nullptr,
          "the window has the Messages and Terminal docks");

    check(!controller->showList(ListCrossings), "a list cannot be shown without a project");
    check(!controller->openCrossing(0), "a crossing cannot be opened without a project");

    controller->openProject(argv[1]);
    check(controller->busy() && !controller->prepareToClose(), "the host may not close during a background step");
    controller->waitUntilIdle();
    QApplication::processEvents();
    check(opened == 1, "opening is reported");

    /* the host works with the window through the controller */
    check(controller->showList(ListCrossings), "the host can show a list");
    QApplication::processEvents();
    QTableView *table = window.browser()->table();
    check(table->model()->rowCount() == static_cast<int>(session.list().rows.size()), "the window shows it");
    check(!controller->openCrossing(1000000), "a crossing that does not exist is refused");
    const int selections0 = selections;
    check(controller->openCrossing(1) && session.crossingLoaded(), "the host can open a crossing");
    QApplication::processEvents();
    check(session.selection().crossing == 1 && selections > selections0, "the selection is reported");
    check(window.editor()->button("pushButton_naverr_next")->isEnabled(), "the crossing editor has it");

    session.moveToMinimumMisfit();
    window.editor()->button("pushButton_naverr_addtie")->click();
    QApplication::processEvents();
    check(ties >= 1 && lastTies == session.selection().numTies, "new ties are reported");

    controller->dismissCrossing();
    QApplication::processEvents();
    check(!session.crossingLoaded(), "the host can dismiss the crossing");
    window.action("pushButton_invertnav")->trigger();
    check(controller->busy(), "the menu runs Invert in the background");
    check(!controller->openCrossing(0), "calls are refused while busy");
    controller->waitUntilIdle();
    QApplication::processEvents();
    check(inversions >= 1 && lastInversion == 2, "the inversion is reported");

    /* the model plot is its own window */
    window.action("pushButton_showmodelplot")->trigger();
    QApplication::processEvents();
    QDockWidget *modelDock = window.findChild<QDockWidget *>("modelDock");
    check(session.modelPlotOpen() && modelDock->isFloating(), "the model plot is a floating window");

    /* closing the mbnavadjust window leaves the host running */
    window.close();
    QApplication::processEvents();
    check(windowClosed == 1 && closed == 1 && !session.selection().projectIsOpen, "closing the window closes the project and is reported");
    check(host.isVisible(), "the host stays open");
  }

  /* the host can open the window again on the same controller */
  {
    NavAdjustWindow again(*controller);
    again.show();
    controller->openProject(argv[1]);
    controller->waitUntilIdle();
    QApplication::processEvents();
    check(session.selection().projectIsOpen && opened == 2, "the window can be opened again");
    check(controller->prepareToClose(), "the host may close when idle");
  }

  delete controller;
  check(Session::current() == nullptr, "deleting the controller ends the session");
  std::unique_ptr<ProjectController> next(ProjectController::create());
  check(next != nullptr, "a controller can be made again afterwards");
  next.reset();

  printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
