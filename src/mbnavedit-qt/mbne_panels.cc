/*--------------------------------------------------------------------
 *    The MB-system:  mbne_panels.cc  9/11/2026
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

#include "mbne_panels.h"

#include <algorithm>
#include <cmath>

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWheelEvent>

namespace mbne {

/*--------------------------------------------------------------------*/
void TimeSlider::wheelEvent(QWheelEvent *event) {
  const int delta = event->angleDelta().y();
  if (delta > 0 && value() < maximum())
    setValue(value() + 1);
  else if (delta < 0 && value() > minimum())
    setValue(value() - 1);
  event->accept();
}

/*--------------------------------------------------------------------*/
ScaleControl::ScaleControl(int minimum, int maximum, int limit, int decimals, QWidget *parent)
    : QWidget(parent), unit_(std::pow(10.0, decimals)), limit_(limit) {
  slider_ = new QSlider(Qt::Horizontal);
  slider_->setRange(minimum, maximum);
  slider_->setFocusPolicy(Qt::NoFocus);
  slider_->setMinimumWidth(160);
  box_ = new QDoubleSpinBox;
  box_->setDecimals(decimals);
  box_->setRange(minimum / unit_, limit / unit_);
  box_->setSingleStep(1.0 / unit_);
  box_->setKeyboardTracking(false);
  box_->setFocusPolicy(Qt::ClickFocus);
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(slider_, 1);
  layout->addWidget(box_);
  connect(slider_, &QSlider::valueChanged, this, &ScaleControl::fromSlider);
  connect(box_, &QDoubleSpinBox::valueChanged, this, &ScaleControl::fromBox);
}

int ScaleControl::value() const { return slider_->value(); }

void ScaleControl::show(int value) {
  quiet_ = true;
  if (value > slider_->maximum())
    slider_->setMaximum(std::min(2 * value, limit_));
  if (value < slider_->minimum())
    slider_->setMinimum(value);
  slider_->setValue(value);
  box_->setValue(value / unit_);
  quiet_ = false;
}

void ScaleControl::setValueQuietly(int value) { show(value); }

void ScaleControl::fromSlider(int value) {
  if (quiet_)
    return;
  quiet_ = true;
  if (value == slider_->minimum() && value <= 1 && slider_->maximum() > 2)
    slider_->setMaximum(std::max(slider_->maximum() / 2, 2));
  else if (value == slider_->maximum())
    slider_->setMaximum(std::min(2 * slider_->maximum(), limit_));
  box_->setValue(value / unit_);
  quiet_ = false;
  emit valueChanged(value);
}

void ScaleControl::fromBox(double value) {
  if (quiet_)
    return;
  const int ivalue = (int)std::lround(value * unit_);
  show(ivalue);
  emit valueChanged(ivalue);
}

}  // namespace mbne
