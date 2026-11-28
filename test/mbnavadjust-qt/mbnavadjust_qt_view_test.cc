/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_qt_view_test.cc  10/11/2026
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
 * Drives the mbnavadjust-qt plot widgets with mouse events, without a display.
 * usage: mbnavadjust_qt_view_test project.nvh
 * The project must have an inversion and at least one crossing with a tie.
 */

#include <cstdio>
#include <string>

#include <QApplication>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPixmap>

#include "mbnaq_core.h"
#include "mbnaq_plots.h"
#include "mbnaq_views.h"

using namespace mbnaq;

static int failures = 0;

static void check(bool ok, const char *what) {
  printf("%s %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok)
    failures++;
}

static void mouse(QWidget &widget, QEvent::Type type, Qt::MouseButton button, int x, int y,
                  Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
  const QPointF local(x, y);
  QMouseEvent event(type, local, widget.mapToGlobal(local), button, type == QEvent::MouseButtonRelease ? Qt::NoButton : button,
                    modifiers);
  QApplication::sendEvent(&widget, &event);
}

static void drag(QWidget &widget, Qt::MouseButton button, int x1, int y1, int x2, int y2,
                 Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
  mouse(widget, QEvent::MouseButtonPress, button, x1, y1, modifiers);
  mouse(widget, QEvent::MouseMove, button, (x1 + x2) / 2, (y1 + y2) / 2, modifiers);
  mouse(widget, QEvent::MouseMove, button, x2, y2, modifiers);
  mouse(widget, QEvent::MouseButtonRelease, button, x2, y2, modifiers);
}

static std::string joined(const std::vector<std::string> &lines) {
  std::string text;
  for (const std::string &line : lines)
    text += line + "\n";
  return text;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s project.nvh\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);

  Session *session = Session::create();
  check(Session::create() == nullptr, "a second Session is refused");
  PlotSet plots(*session);
  ContourView contour(plots);
  MisfitView misfit(plots);
  ZOffsetView zOffset(plots);
  ModelPlotView model(plots);
  contour.resize(501, 501);
  misfit.resize(251, 251);
  zOffset.resize(251, 61);
  model.resize(900, 500);
  contour.show();
  misfit.show();
  zOffset.show();
  model.show();
  QApplication::processEvents();

  check(session->openProject(argv[1]), "open the project");
  session->showList(ListCrossings);
  check(!session->list().rows.empty(), "the project has crossings");
  session->clickListRow(0);
  check(session->crossingLoaded(), "clicking a crossing loads it");
  QApplication::processEvents();

  /* left button drags the working offset */
  Selection before = session->selection();
  drag(contour, Qt::LeftButton, 200, 200, 240, 170);
  Selection after = session->selection();
  check(after.offsetX > before.offsetX, "dragging right moves the section east");
  check(after.offsetY > before.offsetY, "dragging up moves the section north");
  long moved = plots.compareContourWithShared();
  check(moved == 0, "the contours from the layers match the shared plot code after a drag");

  /* a drag shows the section moving */
  const QImage frame0 = contour.grab().toImage();
  mouse(contour, QEvent::MouseButtonPress, Qt::LeftButton, 100, 100);
  mouse(contour, QEvent::MouseMove, Qt::LeftButton, 160, 100);
  const QImage frame1 = contour.grab().toImage();
  mouse(contour, QEvent::MouseButtonRelease, Qt::LeftButton, 160, 100);
  check(frame0 != frame1, "the picture changes during a drag");

  /* speed of a drag */
  QElapsedTimer timer;
  timer.start();
  mouse(contour, QEvent::MouseButtonPress, Qt::LeftButton, 100, 100);
  for (int i = 0; i < 50; i++) {
    mouse(contour, QEvent::MouseMove, Qt::LeftButton, 100 + i % 3, 100 + i % 5);
    contour.grab();
  }
  mouse(contour, QEvent::MouseButtonRelease, Qt::LeftButton, 100, 100);
  printf("note %.2f ms per drag frame\n", timer.elapsed() / 50.0);

  /* middle button zooms; Alt with the left button does the same */
  const std::string zoomed0 = joined(session->crossingStatusLines());
  drag(contour, Qt::MiddleButton, 100, 100, 300, 300);
  const std::string zoomed1 = joined(session->crossingStatusLines());
  check(zoomed0 != zoomed1, "the middle button zooms");
  check(session->isEnabled("pushButton_naverr_fullsize"), "after a zoom Full Size is offered");
  session->fullSize();
  const std::string full = joined(session->crossingStatusLines());
  drag(contour, Qt::LeftButton, 100, 100, 300, 300, Qt::AltModifier);
  check(joined(session->crossingStatusLines()) != full, "Alt with the left button zooms");
  session->fullSize();
  mouse(contour, QEvent::MouseButtonPress, Qt::MiddleButton, 150, 150);
  mouse(contour, QEvent::MouseButtonRelease, Qt::MiddleButton, 150, 150);
  check(joined(session->crossingStatusLines()) == full, "a middle click without a drag changes nothing");

  /* right button picks tie points */
  const std::string offsets0 = joined(session->crossingOffsetLines());
  mouse(contour, QEvent::MouseButtonPress, Qt::RightButton, 120, 380);
  mouse(contour, QEvent::MouseButtonRelease, Qt::RightButton, 120, 380);
  check(joined(session->crossingOffsetLines()) != offsets0, "the right button picks other tie points");

  /* misfit and vertical plots set the offset */
  before = session->selection();
  mouse(misfit, QEvent::MouseButtonPress, Qt::LeftButton, 60, 60);
  mouse(misfit, QEvent::MouseButtonRelease, Qt::LeftButton, 60, 60);
  after = session->selection();
  check(after.offsetX != before.offsetX && after.offsetY != before.offsetY, "a click in the misfit plot sets the offset");
  before = session->selection();
  mouse(zOffset, QEvent::MouseButtonPress, Qt::LeftButton, 40, 30);
  mouse(zOffset, QEvent::MouseMove, Qt::LeftButton, 60, 30);
  mouse(zOffset, QEvent::MouseButtonRelease, Qt::LeftButton, 60, 30);
  after = session->selection();
  check(after.offsetZ != before.offsetZ, "dragging in the vertical plot sets the vertical offset");

  /* resizing keeps the same area on show */
  session->fullSize();
  contour.resize(800, 400);
  QApplication::processEvents();
  contour.resize(501, 501);
  QApplication::processEvents();
  const std::string size0 = joined(session->crossingStatusLines());
  contour.resize(300, 700);
  QApplication::processEvents();
  contour.resize(501, 501);
  QApplication::processEvents();
  check(joined(session->crossingStatusLines()) == size0, "the plot area comes back after resizing");
  check(plots.compareContourWithShared() == 0, "the layers match the shared plot code at the new size");

  /* model plot */
  plots.openModelPlot();
  QApplication::processEvents();
  check(session->modelPlotOpen() && !session->modelPlotLines().empty(), "the model plot opens");
  const QImage model0 = model.grab().toImage();
  drag(model, Qt::RightButton, 200, 100, 500, 100);
  const QImage model1 = model.grab().toImage();
  check(model0 != model1, "the right button zooms the model plot");
  session->modelPlotFullSize();
  session->setModelPlotStyle(ModelPlotStyle::TieOffsets);
  QApplication::processEvents();
  check(model.grab().toImage() != model1, "another plot style draws something else");
  session->setModelPlotStyle(ModelPlotStyle::TimeSeries);
  model.resize(1000, 400);
  QApplication::processEvents();
  session->dismissCrossingWindow();
  const int selectedBefore = session->selection().crossing;
  session->showList(ListTies);
  mouse(model, QEvent::MouseButtonPress, Qt::LeftButton, 500, 90);
  mouse(model, QEvent::MouseButtonRelease, Qt::LeftButton, 500, 90);
  printf("note model pick: crossing %d -> %d\n", selectedBefore, session->selection().crossing);

  /* dark plots */
  session->showList(ListCrossings);
  session->clickListRow(0);
  plots.setDark(true);
  QApplication::processEvents();
  const QImage dark = plots.contour().render(*session);
  check(dark.pixelColor(0, 0) == QColor(Qt::black), "the dark background is black");
  check(plots.compareContourWithShared() == 0, "the layers match the shared plot code on black");

  session->dismissCrossingWindow();
  session->closeProject(true);
  QApplication::processEvents();
  check(!session->crossingLoaded(), "closing unloads the crossing");
  mouse(contour, QEvent::MouseButtonPress, Qt::LeftButton, 100, 100);
  mouse(contour, QEvent::MouseMove, Qt::LeftButton, 110, 110);
  mouse(contour, QEvent::MouseButtonRelease, Qt::LeftButton, 110, 110);
  contour.grab();
  check(true, "the mouse on an empty plot does nothing");

  printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
