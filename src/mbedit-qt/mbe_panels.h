/*--------------------------------------------------------------------
 *    The MB-system:  mbe_panels.h  2/11/2026
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

#ifndef MBEDIT_QT_MBE_PANELS_H
#define MBEDIT_QT_MBE_PANELS_H

#include <vector>

#include <QSlider>
#include <QWidget>

#include "mbe_core.h"

class QDoubleSpinBox;

namespace mbe {

/* slider that moves exactly one step per mouse wheel notch */
class PingSlider : public QSlider {
  Q_OBJECT
 public:
  using QSlider::QSlider;

 protected:
  void wheelEvent(QWheelEvent *event) override;
};

/* A slider with its value. As in mbedit its range doubles when the slider
   reaches its end and halves at its start, up to a limit. */
class ScaleControl : public QWidget {
  Q_OBJECT
 public:
  ScaleControl(int minimum, int maximum, int limit, int decimals, QWidget *parent = nullptr);
  int value() const;
  void setValueQuietly(int value);
  void setRangeQuietly(int minimum, int maximum);

 signals:
  void valueChanged(int value);

 private:
  void fromSlider(int value);
  void fromBox(double value);
  void show(int value);
  QSlider *slider_;
  QDoubleSpinBox *box_;
  double unit_;
  int limit_;
  bool quiet_ = false;
};

/* a strip above the ping slider: the darker a column, the more suspect its pings */
class SuspectStrip : public QWidget {
  Q_OBJECT
 public:
  explicit SuspectStrip(QWidget *parent = nullptr);
  void setSuspects(const std::vector<mbe::Suspect> &suspects, int npings);

 signals:
  void pingClicked(int ping);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  std::vector<double> scores_;
};

/* the time series beside the soundings, one value per ping at the height of its profile */
class TimeSeriesPanel : public QWidget {
  Q_OBJECT
 public:
  explicit TimeSeriesPanel(const mbe::Session &session, QWidget *parent = nullptr);

 signals:
  void pingClicked(int ping);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  const mbe::Session &session_;
};

/* the navigation of the pings in memory, with the pings in view marked */
class TrackMap : public QWidget {
  Q_OBJECT
 public:
  explicit TrackMap(const mbe::Session &session, QWidget *parent = nullptr);
  QSize sizeHint() const override { return QSize(260, 300); }
  /* where a ping is drawn; false if it has no navigation */
  bool pingPosition(int ping, QPointF *position) const;

 signals:
  void pingClicked(int ping);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  bool bounds(double *lonmin, double *lonmax, double *latmin, double *latmax, double *scale) const;
  const mbe::Session &session_;
};

}  // namespace mbe

#endif
