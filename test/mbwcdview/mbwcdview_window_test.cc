/*--------------------------------------------------------------------
 *    The MB-system:  mbwcdview_window_test.cc  9/11/2026
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
 * Drives the window of mbwcdview: opening files, moving between pings with the
 * slider, keys and wheel, the check boxes, picking and replay. It needs no
 * OpenGL: it reads what is drawn from the data of the plot. Arguments: the
 * .kmall file of a line with its .kmwcd beside it, and a directory to write in.
 */

#include <QApplication>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextStream>
#include <QWheelEvent>

#include <cmath>
#include <cstdio>

#include "WcdView.h"
#include "wcd_window.h"

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

void wait(int ms) {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < ms)
    QApplication::processEvents(QEventLoop::AllEvents, 10);
}

QCheckBox *box(QWidget &w, const QString &text) {
  for (QCheckBox *b : w.findChildren<QCheckBox *>())
    if (b->text() == text)
      return b;
  return nullptr;
}

QPushButton *button(QWidget &w, const QString &text) {
  for (QPushButton *b : w.findChildren<QPushButton *>())
    if (b->text() == text)
      return b;
  return nullptr;
}

bool has_label(QWidget &w, const QString &text) {
  for (QLabel *l : w.findChildren<QLabel *>())
    if (l->text() == text)
      return true;
  return false;
}

QString meta_text(QWidget &w) {
  for (QLabel *l : w.findChildren<QLabel *>())
    if (l->wordWrap())
      return l->text();
  return QString();
}

void key(QWidget &w, int code) {
  QKeyEvent event(QEvent::KeyPress, code, Qt::NoModifier);
  QApplication::sendEvent(&w, &event);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: mbwcdview_window_test file.kmall directory\n");
    return 2;
  }
  QApplication app(argc, argv);
  const QString kmall = argv[1], dir = argv[2];

  wcd::WaterColumnWindow window;
  wcd::WaterColumnWidget &w = *window.editor();
  window.show();
  wait(100);
  QStatusBar *status = window.statusBar();
  auto *view = window.findChild<wcd::WcdView *>();
  auto *slider = window.findChild<QSlider *>();
  const auto spins = window.findChildren<QDoubleSpinBox *>();  // depth, amplitude, across-track, replay rate
  CHECK(view != nullptr && slider != nullptr && spins.size() == 6);

  /* nothing open */
  CHECK(window.windowTitle().startsWith("mbwcdview"));
  CHECK(status->currentMessage() == "Ready");
  CHECK(has_label(window, "No file loaded"));
  CHECK(!w.saveImage(dir + "/empty.png"));
  CHECK(w.pick(0.0, 100.0).isEmpty());
  CHECK(status->currentMessage().startsWith("Nothing to pick"));

  /* a file that cannot be read */
  CHECK(!w.openFile(dir + "/missing.kmall"));
  CHECK(status->currentMessage().startsWith("Cannot read missing.kmall"));

  /* a line */
  CHECK(w.openFile(kmall));
  const int last = slider->maximum();
  CHECK(last > 2);
  CHECK(slider->value() == 0);
  const QString name = kmall.mid(kmall.lastIndexOf('/') + 1);
  CHECK(has_label(window, name));
  CHECK(has_label(window, QString("0 / %1").arg(last)));
  CHECK(!view->plot().data().across.empty());
  CHECK(box(window, "Ray trace (SVP)")->isEnabled() && box(window, "Ray trace (SVP)")->isChecked());
  CHECK(spins[2]->value() == spins[2]->minimum() && spins[3]->value() == spins[3]->maximum());
  CHECK(spins[0]->value() == 0.0 && spins[1]->value() > 0.0);
  wait(1100);
  CHECK(status->currentMessage().startsWith("Nav summary"));

  /* between the pings */
  const QString first_meta = meta_text(window);
  CHECK(!first_meta.isEmpty());
  w.setStep(1);
  CHECK(slider->value() == 1);
  CHECK(has_label(window, QString("1 / %1").arg(last)));
  CHECK(meta_text(window) != first_meta);
  key(w, Qt::Key_Right);
  CHECK(slider->value() == 2);
  key(w, Qt::Key_Left);
  key(w, Qt::Key_Left);
  CHECK(slider->value() == 0);
  key(w, Qt::Key_Left);
  CHECK(slider->value() == 0);
  w.setStep(last);
  key(w, Qt::Key_Right);
  CHECK(slider->value() == last);
  w.setStep(0);
  {
    QWheelEvent up(QPointF(5, 5), QPointF(5, 5), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(slider, &up);
    CHECK(slider->value() == 1);
    QWheelEvent down(QPointF(5, 5), QPointF(5, 5), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(slider, &down);
    CHECK(slider->value() == 0);
  }
  w.showPing(2);
  CHECK(slider->value() > 1);

  /* what the check boxes draw */
  w.setStep(0);
  CHECK(view->plot().data().soundings.empty());
  box(window, "Bottom")->click();
  CHECK(!view->plot().data().soundings.empty());
  box(window, "Bottom")->click();
  CHECK(view->plot().data().soundings.empty());
  CHECK(!view->plot().data().show_wc_bottom);
  box(window, "WC bottom")->click();
  CHECK(view->plot().data().show_wc_bottom);
  box(window, "WC bottom")->click();

  /* picking the cell nearest to a point */
  const wcd::PlotData &data = view->plot().data();
  size_t cell = 0;
  while (cell < data.amplitude.size() && !(std::isfinite(data.amplitude[cell]) && std::isfinite(data.across[cell]) && std::isfinite(data.depth[cell])))
    cell++;
  CHECK(cell < data.amplitude.size());
  const double x = data.across[cell], z = data.depth[cell];
  const QStringList row = w.pick(x, z);
  CHECK(row.size() == wcd::PickDialog::columns.size());
  CHECK(row.size() > 3 && std::abs(row[2].toDouble() - x) < 0.01 && std::abs(row[3].toDouble() - z) < 0.01);
  auto *dialog = window.findChild<wcd::PickDialog *>();
  CHECK(dialog != nullptr && dialog->isVisible());
  auto *table = dialog->findChild<QTableWidget *>();
  CHECK(table->rowCount() == 1);
  const QString picks = dir + "/picks.txt";
  CHECK(dialog->saveText(picks));
  {
    QFile file(picks);
    CHECK(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QStringList lines = QTextStream(&file).readAll().split('\n', Qt::SkipEmptyParts);
    CHECK(lines.size() == 2 && lines[0].startsWith("Ping\tUTC"));
  }

  /* a click picks only with Pick on */
  emit view->clicked(x, z);
  CHECK(table->rowCount() == 1);
  box(window, "Pick")->click();
  emit view->clicked(x, z);
  CHECK(table->rowCount() == 2);

  /* nothing to pick outside of the depth window */
  const double dmin = spins[0]->value();
  spins[0]->setValue(spins[0]->maximum());
  CHECK(w.pick(x, z).isEmpty());
  spins[0]->setValue(dmin);

  /* closing the table empties it and stops picking */
  button(*dialog, "Close")->click();
  CHECK(table->rowCount() == 0 && !dialog->isVisible());
  CHECK(!box(window, "Pick")->isChecked());

  /* replay goes round the steps */
  spins[5]->setValue(10.0);
  w.setStep(last);
  QPushButton *replay = button(window, "Replay");
  replay->click();
  CHECK(replay->text() == "Stop (10.0 p/s)");
  wait(300);
  CHECK(slider->value() < last);
  replay->click();
  CHECK(replay->text() == "Replay");
  const int stopped = slider->value();
  wait(250);
  CHECK(slider->value() == stopped);

  /* an image of the plot */
  const QString image = dir + "/window.png";
  CHECK(w.saveImage(image));
  CHECK(QFile::exists(image));

  /* opening again stops replay and starts at the first step */
  w.setStep(1);
  replay->click();
  CHECK(replay->isChecked());
  CHECK(w.openFile(kmall));
  CHECK(!replay->isChecked());
  CHECK(slider->value() == 0 && slider->maximum() == last);
  CHECK(has_label(window, QString("0 / %1").arg(last)));

  window.close();
  CHECK(!window.isVisible());

  printf("%d checks, %d failed\n", checks, failures);
  return failures != 0;
}
