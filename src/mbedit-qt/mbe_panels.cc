/*--------------------------------------------------------------------
 *    The MB-system:  mbe_panels.cc  2/11/2026
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

#include "mbe_panels.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

namespace mbe {

/*--------------------------------------------------------------------*/
void PingSlider::wheelEvent(QWheelEvent *event) {
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
  slider_->setMinimumWidth(80);
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

void ScaleControl::setRangeQuietly(int minimum, int maximum) {
  quiet_ = true;
  slider_->setRange(minimum, maximum);
  box_->setRange(minimum / unit_, std::max(maximum, limit_) / unit_);
  quiet_ = false;
}

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

/*--------------------------------------------------------------------*/
SuspectStrip::SuspectStrip(QWidget *parent) : QWidget(parent) {
  setFixedHeight(8);
  setToolTip("Suspect pings, worst darkest. Click to go there, or use the keys N and B.");
}

void SuspectStrip::setSuspects(const std::vector<mbe::Suspect> &suspects, int npings) {
  scores_.assign(npings, 0.0);
  double highest = 0.0;
  for (const mbe::Suspect &s : suspects)
    highest = std::max(highest, s.score);
  for (const mbe::Suspect &s : suspects) {
    if (s.ping < npings && highest > 0.0)
      scores_[s.ping] = s.score / highest;
  }
  update();
}

void SuspectStrip::paintEvent(QPaintEvent *) {
  if (scores_.empty())
    return;
  QPainter painter(this);
  const QColor color = palette().color(QPalette::Highlight);
  const int n = (int)scores_.size();
  for (int x = 0; x < width(); x++) {
    double value = 0.0;
    for (int i = x * n / width(); i < std::max((x + 1) * n / width(), x * n / width() + 1) && i < n; i++)
      value = std::max(value, scores_[i]);
    if (value > 0.0) {
      QColor c = color;
      c.setAlphaF(std::min(1.0, 0.15 + 0.85 * value));
      painter.fillRect(x, 0, 1, height(), c);
    }
  }
}

void SuspectStrip::mousePressEvent(QMouseEvent *event) {
  if (!scores_.empty() && width() > 0)
    emit pingClicked(std::min((int)(event->position().x() * scores_.size() / width()), (int)scores_.size() - 1));
}

/*--------------------------------------------------------------------*/
TimeSeriesPanel::TimeSeriesPanel(const mbe::Session &session, QWidget *parent) : QWidget(parent), session_(session) {
  setMinimumWidth(120);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

void TimeSeriesPanel::paintEvent(QPaintEvent *) {
  /* disabled while a thread is reading or writing the pings */
  if (!isEnabled())
    return;
  const int id = session_.settings.show_time;
  if (!session_.isOpen() || session_.nbuff() == 0 || session_.nplot() <= 0 || id <= mbe::PLOT_TIME)
    return;
  QPainter painter(this);
  painter.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  const QColor text = palette().color(QPalette::Text);
  const bool dark = palette().color(QPalette::Base).lightness() < 128;
  const mbe::PlotBox &box = session_.box();
  const int left = 10, right = width() - 10, plot_width = std::max(right - left, 10);
  const int first = session_.current(), nplot = session_.nplot();
  double tsmin, tsmax;
  session_.tsMinMax(first, nplot, id, &tsmin, &tsmax);
  const double scale = plot_width / (tsmax > tsmin ? tsmax - tsmin : 1.0);
  const double dy = ((double)(box.ymax - box.ymin)) / session_.settings.plot_size;

  QPen solid(text, 0), dash(text, 0, Qt::DashLine);
  painter.setPen(solid);
  painter.drawRect(left, box.ymin, plot_width, box.ymax - box.ymin);
  painter.setPen(dash);
  for (int k = 1; k < 4; k++)
    painter.drawLine(left + k * plot_width / 4, box.ymin, left + k * plot_width / 4, box.ymax);
  painter.setPen(solid);
  painter.drawText(QRect(0, box.ymin - painter.fontMetrics().lineSpacing() - 2, width(), painter.fontMetrics().lineSpacing()), Qt::AlignCenter,
                   mbe::Session::tsLabel(id));
  painter.drawText(left, box.ymax + painter.fontMetrics().ascent() + 3, QString::number(tsmin, 'g', 6));
  const QString max_text = QString::number(tsmax, 'g', 6);
  painter.drawText(right - painter.fontMetrics().horizontalAdvance(max_text), box.ymax + painter.fontMetrics().ascent() + 3, max_text);

  auto draw = [&](const QColor &color, bool markers, auto value) {
    painter.setPen(QPen(color, 0));
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPointF previous;
    for (int i = first; i < first + nplot; i++) {
      const QPointF point(left + (value(i) - tsmin) * scale, box.ymax - (int)(dy / 2) - (int)((i - first) * dy));
      if (i > first)
        painter.drawLine(previous, point);
      if (markers)
        painter.fillRect(QRectF(point.x() - 2, point.y() - 2, 4, 4), color);
      previous = point;
    }
  };
  draw(text, true, [&](int i) { return session_.tsValue(i, id); });

  /* with the roll, the acrosstrack slope and the difference: they show a timing error */
  if (id == mbe::PLOT_ROLL) {
    draw(dark ? QColor(255, 110, 110) : QColor(220, 0, 0), false, [&](int i) { return session_.xtrackSlope(i); });
    draw(dark ? QColor(120, 170, 255) : QColor(0, 0, 230), false, [&](int i) { return session_.tsValue(i, id) - session_.xtrackSlope(i); });
  }
}

void TimeSeriesPanel::mousePressEvent(QMouseEvent *event) {
  if (!isEnabled() || !session_.isOpen() || session_.nbuff() == 0 || session_.nplot() <= 0)
    return;
  const mbe::PlotBox &box = session_.box();
  const double dy = ((double)(box.ymax - box.ymin)) / session_.settings.plot_size;
  const int y = (int)event->position().y();
  if (dy <= 0.0 || y < box.ymin || y > box.ymax)
    return;
  const int row = (int)((box.ymax - y) / dy);
  emit pingClicked(session_.current() + std::min(row, session_.nplot() - 1));
}

/*--------------------------------------------------------------------*/
TrackMap::TrackMap(const mbe::Session &session, QWidget *parent) : QWidget(parent), session_(session) {
  setMinimumWidth(120);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

/* in degrees, and the size of a degree of longitude against one of latitude */
bool TrackMap::bounds(double *lonmin, double *lonmax, double *latmin, double *latmax, double *scale) const {
  *lonmin = *latmin = 1.0e30;
  *lonmax = *latmax = -1.0e30;
  for (const mbe::Ping &p : session_.pings()) {
    if (p.navlon == 0.0 && p.navlat == 0.0)
      continue;
    *lonmin = std::min(*lonmin, p.navlon);
    *lonmax = std::max(*lonmax, p.navlon);
    *latmin = std::min(*latmin, p.navlat);
    *latmax = std::max(*latmax, p.navlat);
  }
  if (*lonmin > *lonmax)
    return false;
  *scale = std::max(cos(0.5 * (*latmin + *latmax) * M_PI / 180.0), 0.01);
  return true;
}

bool TrackMap::pingPosition(int ping, QPointF *position) const {
  double lonmin, lonmax, latmin, latmax, scale;
  if (!isEnabled() || ping < 0 || ping >= session_.nbuff() || !bounds(&lonmin, &lonmax, &latmin, &latmax, &scale))
    return false;
  const mbe::Ping &p = session_.pings()[ping];
  if (p.navlon == 0.0 && p.navlat == 0.0)
    return false;
  const int margin = 14, line = fontMetrics().lineSpacing();
  const double area_x = std::max(width() - 2 * margin, 1), area_y = std::max(height() - 2 * margin - 2 * line, 1);
  const double size_x = std::max((lonmax - lonmin) * scale, 1.0e-9), size_y = std::max(latmax - latmin, 1.0e-9);
  const double pixels = std::min(area_x / size_x, area_y / size_y);
  const double x0 = margin + 0.5 * (area_x - size_x * pixels), y0 = margin + line + 0.5 * (area_y + size_y * pixels);
  *position = QPointF(x0 + (p.navlon - lonmin) * scale * pixels, y0 - (p.navlat - latmin) * pixels);
  return true;
}

void TrackMap::paintEvent(QPaintEvent *) {
  /* disabled while a thread is reading or writing the pings */
  if (!isEnabled() || !session_.isOpen() || session_.nbuff() == 0)
    return;
  double lonmin, lonmax, latmin, latmax, scale;
  QPainter painter(this);
  const QColor text = palette().color(QPalette::Text);
  const bool dark = palette().color(QPalette::Base).lightness() < 128;
  painter.setPen(text);
  const int line = painter.fontMetrics().lineSpacing();
  painter.drawText(QRect(0, 2, width(), line), Qt::AlignCenter, "Pings in Memory");
  if (!bounds(&lonmin, &lonmax, &latmin, &latmax, &scale)) {
    painter.drawText(rect(), Qt::AlignCenter, "No navigation");
    return;
  }
  painter.drawText(QRect(4, height() - line - 2, width() - 8, line), Qt::AlignCenter,
                   QString("%1 to %2, %3 to %4").arg(lonmin, 0, 'f', 3).arg(lonmax, 0, 'f', 3).arg(latmin, 0, 'f', 3).arg(latmax, 0, 'f', 3));
  const int nbuff = session_.nbuff();
  const int first = session_.current(), last = first + session_.nplot();
  const QColor view = dark ? QColor(255, 170, 60) : QColor(210, 90, 0);
  painter.setRenderHint(QPainter::Antialiasing, true);
  QPointF previous, point;
  bool have_previous = false;
  auto segments = [&](int from, int to, const QPen &pen) {
    painter.setPen(pen);
    have_previous = false;
    for (int i = from; i < to; i++) {
      if (!pingPosition(i, &point))
        continue;
      if (have_previous)
        painter.drawLine(previous, point);
      previous = point;
      have_previous = true;
    }
  };
  segments(0, nbuff, QPen(text, 1));
  segments(first, std::min(last, nbuff), QPen(view, 3));
  if (pingPosition(0, &point)) {
    painter.setPen(QPen(text, 1));
    painter.setBrush(palette().color(QPalette::Base));
    painter.drawEllipse(point, 3, 3);
  }
}

void TrackMap::mousePressEvent(QMouseEvent *event) {
  if (!isEnabled() || !session_.isOpen())
    return;
  int nearest = -1;
  double best = 1.0e30;
  QPointF point;
  for (int i = 0; i < session_.nbuff(); i++) {
    if (!pingPosition(i, &point))
      continue;
    const QPointF d = point - event->position();
    if (d.x() * d.x() + d.y() * d.y() < best) {
      best = d.x() * d.x() + d.y() * d.y();
      nearest = i;
    }
  }
  if (nearest >= 0)
    emit pingClicked(nearest);
}

}  // namespace mbe
