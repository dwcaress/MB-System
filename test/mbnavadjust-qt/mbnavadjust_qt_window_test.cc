/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_qt_window_test.cc  10/11/2026
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
 * Drives the mbnavadjust-qt main window without a display.
 * usage: mbnavadjust_qt_window_test project.nvh [screenshot.png]
 * The project needs an inversion and crossings.
 */

#include <cstdio>
#include <cstdlib>
#include <string>

#include <QAction>
#include <QApplication>
#include <QPixmap>
#include <QPushButton>
#include <QTime>
#include <QTableView>

#include <QDockWidget>
#include <QKeyEvent>

#include "NavAdjustWindow.h"
#include "mbnaq_controller.h"
#include "mbnaq_core.h"
#include "mbnaq_plots.h"
#include "mbnaq_terminal.h"
#include "mbnaq_views.h"
#include "mbnaq_widgets.h"

using namespace mbnaq;

static int failures = 0;

static void check(bool ok, const char *what) {
  printf("%s %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok)
    failures++;
}

static std::string statusLine(Session &session, const char *start) {
  for (const std::string &line : session.statusLines()) {
    if (line.rfind(start, 0) == 0)
      return line;
  }
  return std::string();
}

/* the Offline text is the only grey in the contour plot */
static int greyPixels(const QImage &image) {
  int count = 0;
  for (int y = 0; y < image.height(); y++) {
    for (int x = 0; x < image.width(); x++) {
      const QRgb pixel = image.pixel(x, y);
      if (qRed(pixel) == qGreen(pixel) && qGreen(pixel) == qBlue(pixel) && qRed(pixel) > 110 && qRed(pixel) < 210)
        count++;
    }
  }
  return count;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s project.nvh [screenshot.png]\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);

  check(OutputCapture::instance().start(), "the terminal output is captured");
  ProjectController *controller = ProjectController::create();
  check(controller != nullptr, "the first controller is made");
  check(ProjectController::create() == nullptr, "a second controller is refused");
  Session &session = controller->session();
  {
    NavAdjustWindow window(*controller, false);
    window.show();
    QApplication::processEvents();

    check(window.action("pushButton_open")->isEnabled(), "Open is offered without a project");
    check(!window.action("pushButton_close")->isEnabled(), "Close is not offered without a project");
    check(!window.action("pushButton_invertnav")->isEnabled(), "Invert is not offered without a project");

    /* with no crossing the plots say so */
    check(greyPixels(window.editor()->contourView()->grab().toImage()) > 500, "the contour plot shows Offline without a crossing");

    /* opening runs on a background thread */
    bool sawBusy = false;
    QObject::connect(controller, &ProjectController::busyChanged, [&](bool busy) { sawBusy = sawBusy || busy; });
    controller->openProject(argv[1]);
    check(controller->busy(), "opening runs in the background");
    controller->waitUntilIdle();
    QApplication::processEvents();
    check(sawBusy && !controller->busy(), "busy is reported and ends");
    check(session.selection().projectIsOpen, "the project is open");
    check(window.windowTitle().startsWith("mbnavadjust-qt"), "the window has a title");
    check(!window.action("pushButton_open")->isEnabled() && window.action("pushButton_close")->isEnabled(),
          "the file actions follow the project");

    /* the list as a table */
    window.action("pushButton_showcrossings")->trigger();
    QApplication::processEvents();
    QTableView *table = window.browser()->table();
    check(table->model()->rowCount() == static_cast<int>(session.list().rows.size()), "the table has the rows of the list");
    check(table->model()->columnCount() == 7, "the crossing list has columns");
    int crossingColumn = -1;
    int overlapColumn = -1;
    for (int c = 0; c < table->model()->columnCount(); c++) {
      if (table->model()->headerData(c, Qt::Horizontal).toString() == "Crossing")
        crossingColumn = c;
      if (table->model()->headerData(c, Qt::Horizontal).toString() == "Overlap %")
        overlapColumn = c;
    }
    table->sortByColumn(overlapColumn, Qt::DescendingOrder);
    QApplication::processEvents();
    const int largest = table->model()->index(0, crossingColumn).data().toInt();
    const double overlap0 = table->model()->index(0, overlapColumn).data().toDouble();
    const double overlap1 = table->model()->index(1, overlapColumn).data().toDouble();
    check(overlap0 >= overlap1, "the table sorts by a column as numbers");
    emit table->clicked(table->model()->index(0, 0));
    QApplication::processEvents();
    check(session.crossingLoaded() && session.selection().crossing == largest, "a click in the sorted table loads that crossing");

    /* the crossing editor */
    CrossingEditor *editor = window.editor();
    QPushButton *next = editor->button("pushButton_naverr_next");
    check(next != nullptr && next->isEnabled(), "Next is offered");
    next->click();
    QApplication::processEvents();
    check(session.selection().crossing != largest, "Next loads another crossing");

    /* undo */
    const std::string ties0 = statusLine(session, "Number of Ties Set:");
    const int crossing0 = session.selection().currentCrossing;
    check(!session.canUndo(), "nothing to undo after loading");
    session.moveToMinimumMisfit();
    editor->button("pushButton_naverr_addtie")->click();
    QApplication::processEvents();
    const std::string ties1 = statusLine(session, "Number of Ties Set:");
    check(ties1 != ties0, "Add Tie adds a tie");
    check(session.canUndo() && window.action("pushButton_naverr_addtie") == nullptr, "the edit can be undone");
    controller->undo();
    QApplication::processEvents();
    check(statusLine(session, "Number of Ties Set:") == ties0, "Undo removes the tie");
    check(session.selection().currentCrossing == crossing0, "Undo stays on the crossing");
    check(session.canRedo(), "Redo is offered");
    controller->redo();
    QApplication::processEvents();
    check(statusLine(session, "Number of Ties Set:") == ties1, "Redo adds the tie again");

    /* Enter opens the row the keyboard has moved to */
    window.action("pushButton_showcrossings")->trigger();
    QApplication::processEvents();
    const QModelIndex keyRow = table->model()->index(2, 0);
    table->setCurrentIndex(keyRow);
    const int keyCrossing = table->model()->index(2, crossingColumn).data().toInt();
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(table, &enter);
    QApplication::processEvents();
    check(session.crossingLoaded() && session.selection().crossing == keyCrossing, "Enter on a list row opens it");

    /* the forms */
    ControlValues controls = session.controlValues();
    controls.contourInterval = 2.5;
    session.applyControls(controls);
    check(session.controlValues().contourInterval == 2.5, "Controls Apply changes the project");
    const BiasValues biases = session.biasValues();
    check(biases.available, "the loaded sections have biases");
    session.applyBiases(1.0, 0.5, 1.0, 0.5);
    check(session.biasValues().heading1 == 1.0 && session.biasValues().roll2 == 0.5, "Biases Apply changes the files");
    session.applyBiases(biases.heading1, biases.roll1, biases.heading2, biases.roll2);

    /* the plot shows the crossing, and Offline again after Dismiss */
    check(greyPixels(window.editor()->contourView()->grab().toImage()) < 500, "the contour plot shows the crossing");

    /* steps in the background: as in Motif they are offered when no crossing is loaded */
    check(!window.action("pushButton_invertnav")->isEnabled(), "Invert is not offered with a crossing loaded");
    editor->button("pushButton_naverr_dismiss")->click();
    QApplication::processEvents();
    check(window.action("pushButton_invertnav")->isEnabled(), "Invert is offered after Dismiss");
    check(greyPixels(window.editor()->contourView()->grab().toImage()) > 500, "Dismiss shows Offline again");
    window.action("pushButton_invertnav")->trigger();
    check(controller->busy(), "Invert runs in the background");
    check(!window.action("pushButton_invertnav")->isEnabled(), "actions are not offered while busy");
    controller->waitUntilIdle();
    QApplication::processEvents();
    check(statusLine(session, "Inversion Performed:").find("Current") != std::string::npos, "the inversion is current");

    /* the model plot dock */
    window.action("pushButton_showmodelplot")->trigger();
    QApplication::processEvents();
    check(session.modelPlotOpen(), "Show Modelplot opens the plot");
    QDockWidget *modelDock = window.findChild<QDockWidget *>("modelDock");
    check(modelDock != nullptr && modelDock->isFloating() && modelDock->width() >= 1000, "the model plot opens as a large floating window");
    QDockWidget *logDock = window.findChild<QDockWidget *>("logDock");
    QDockWidget *terminalDock = window.findChild<QDockWidget *>("terminalDock");
    check(logDock != nullptr && terminalDock != nullptr && logDock->isVisible() && terminalDock->isVisible() &&
              logDock->y() == terminalDock->y() && logDock->x() < terminalDock->x(),
          "Messages and Terminal are side by side");
    check(std::abs(logDock->width() - terminalDock->width()) < 40, "Messages and Terminal have equal widths");
    printf("terminal-marker-from-the-test\n");
    fflush(stdout);
    const QTime waitUntil = QTime::currentTime().addMSecs(600);
    while (QTime::currentTime() < waitUntil)
      QApplication::processEvents(QEventLoop::AllEvents, 50);
    check(terminalDock->findChild<TerminalLog *>() != nullptr && terminalDock->findChild<TerminalLog *>()->text().contains("terminal-marker-from-the-test"),
          "the Terminal widget shows the program output");
    if (argc > 2) {
      window.resize(1500, 950);
      QApplication::processEvents();
      window.grab().save(argv[2]);
    }

    /* every list fits a table */
    static const char *const lists[] = {"pushButton_showdata", "pushButton_showsurveys", "pushButton_showblocks", "pushButton_showsections",
                                        "pushButton_showcrossingties", "pushButton_showcrossingtiessortedall", "pushButton_showmediocrecrossings"};
    for (const char *name : lists) {
      window.action(name)->trigger();
      QApplication::processEvents();
      check(table->model()->rowCount() == static_cast<int>(session.list().rows.size()), name);
    }

    /* the view filters are exclusive */
    window.action("pushButton_showdata")->trigger();
    emit table->clicked(table->model()->index(0, 0));
    window.action("toggleButton_showselectedfile")->trigger();
    QApplication::processEvents();
    check(session.selection().mode == ViewFile && window.action("toggleButton_showselectedfile")->isChecked(), "a view filter applies");
    window.action("toggleButton_showallsurveys")->trigger();
    QApplication::processEvents();

    window.close();
    QApplication::processEvents();
    check(!session.selection().projectIsOpen, "closing the window closes the project");
  }

  /* the edits were written when the window closed */
  controller->openProject(argv[1]);
  controller->waitUntilIdle();
  QApplication::processEvents();
  check(session.controlValues().contourInterval == 2.5, "the Controls value is in the project file");
  const std::string saved = statusLine(session, "Number of Ties Set:");
  controller->closeProject();
  printf("note %s\n", saved.c_str());

  delete controller;
  OutputCapture::instance().stop();
  printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
