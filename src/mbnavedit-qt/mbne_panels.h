/*--------------------------------------------------------------------
 *    The MB-system:  mbne_panels.h  9/11/2026
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

#ifndef MBNAVEDIT_QT_MBNE_PANELS_H
#define MBNAVEDIT_QT_MBNE_PANELS_H

#include <QSlider>
#include <QWidget>

class QDoubleSpinBox;
class QWheelEvent;

namespace mbne {

/* slider that moves exactly one step per mouse wheel notch */
class TimeSlider : public QSlider {
  Q_OBJECT
 public:
  using QSlider::QSlider;

 protected:
  void wheelEvent(QWheelEvent *event) override;
};

/* A slider with its value. As in mbnavedit its range doubles when the slider
   reaches its end and halves at its start, up to a limit. */
class ScaleControl : public QWidget {
  Q_OBJECT
 public:
  ScaleControl(int minimum, int maximum, int limit, int decimals, QWidget *parent = nullptr);
  int value() const;
  void setValueQuietly(int value);

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

}  // namespace mbne

#endif
