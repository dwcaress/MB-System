/*--------------------------------------------------------------------
 *    The MB-system:  wcd_widget.h  1/11/2026
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

#ifndef MBWCDVIEW_WCD_WIDGET_H
#define MBWCDVIEW_WCD_WIDGET_H

#include <utility>
#include <vector>

#include <QDialog>
#include <QSlider>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "wcd_data.h"
#include "wcd_scene.h"

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QThread;
class QTimer;

namespace wcd {

class WcdView;

/* slider that moves exactly one step per mouse wheel notch */
class OneStepWheelSlider : public QSlider {
  Q_OBJECT
 public:
  using QSlider::QSlider;

 protected:
  void wheelEvent(QWheelEvent *event) override;
};

/* modeless table of picked positions */
class PickDialog : public QDialog {
  Q_OBJECT
 public:
  explicit PickDialog(QWidget *parent = nullptr);
  void addRow(const QStringList &values);
  bool saveText(const QString &path);
  static const QStringList columns;

 private:
  void saveDialog();
  void removeSelected();
  void clearAndClose();
  QTableWidget *table_;
};

/* The tool: the plot, its controls and the data. WaterColumnWindow puts it in the window of the program. */
class WaterColumnWidget : public QWidget {
  Q_OBJECT

 public:
  explicit WaterColumnWidget(QWidget *parent = nullptr);
  ~WaterColumnWidget() override;

  bool openFile(const QString &path);  // runs in the background; false when another file is still loading
  bool busy() const { return worker_ != nullptr; }
  bool loaded() const { return !steps_.empty(); }
  void waitUntilIdle();
  void setStep(int step);
  void showPing(int ping);  // first slider step of water column ping index ping; waits for a file that is loading
  bool saveImage(const QString &path);
  QStringList pick(double across, double depth);  // for tests: pick the cell nearest to a point
  QString title() const;

 signals:
  void statusMessage(const QString &message, int timeout_ms);
  void diagnostic(const QString &message);  // the program writes these to stderr

 protected:
  void keyPressEvent(QKeyEvent *event) override;

 private:
  void showStatus(const QString &message, int timeout_ms = 0);
  void openDialog();
  void finishOpen(Line line, bool ok, const std::string &message);
  void saveImageDialog();
  void redraw();
  void updatePlotSettings();
  void onSlider(int value);
  void onClicked(double across, double depth);
  void togglePick(bool on);
  void showPickDialog();
  void toggleReplay(bool on);
  void updateReplayInterval();
  void replayTick();

  wcd::Line line_;
  std::vector<std::pair<int, int>> steps_;  // (water column ping, swath) per slider step
  int current_ = 0;
  QString file_label_;
  QThread *worker_ = nullptr;
  bool job_had_focus_ = false;
  int pending_ping_ = -1;

  QWidget *central_;
  WcdView *view_;
  QLabel *file_name_, *meta_, *step_label_;
  QCheckBox *bottom_, *wc_bottom_, *raytrace_, *pick_;
  QDoubleSpinBox *dmin_, *dmax_, *amin_, *amax_, *xmax_, *replay_rate_;
  QPushButton *replay_;
  OneStepWheelSlider *slider_;
  QTimer *replay_timer_;
  PickDialog *pick_dialog_ = nullptr;
  bool pick_shown_once_ = false;
};

}  // namespace wcd

#endif
