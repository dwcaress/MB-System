/*--------------------------------------------------------------------
 *    The MB-system:  mbe_canvas.cc  2/11/2026
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

#include "mbe_canvas.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include <QApplication>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QNativeGestureEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QWheelEvent>

#include "mb_define.h"
#include "mb_status.h"

namespace mbe {

namespace {

constexpr int LABEL_CHARS_TIME = 40;   // "%5d mm/dd/yyyy hh:mm:ss.mmm %10.3f"
constexpr int LABEL_CHARS_PLAIN = 16;  // "%5d %10.3f"
constexpr int MANY_SOUNDINGS = 20000;  // above this the lines are not antialiased, to keep drags smooth

struct Colors {
  QColor text, red, blue, green;
};

/* The colours of a system palette may be partly transparent. The squares are
   written straight into the image, so everything here must be opaque. */
QColor over(const QColor &color, const QColor &background) {
  const double a = color.alphaF();
  return QColor::fromRgbF(a * color.redF() + (1.0 - a) * background.redF(), a * color.greenF() + (1.0 - a) * background.greenF(),
                          a * color.blueF() + (1.0 - a) * background.blueF());
}

QColor base_color(const QPalette &palette) {
  QColor base = palette.color(QPalette::Base);
  base.setAlpha(255);
  return base;
}

Colors colors_for(const QPalette &palette) {
  const QColor base = base_color(palette);
  const bool dark = base.lightness() < 128;
  Colors c;
  c.text = over(palette.color(QPalette::Text), base);
  c.red = dark ? QColor(255, 110, 110) : QColor(220, 0, 0);
  c.blue = dark ? QColor(120, 170, 255) : QColor(0, 0, 230);
  c.green = dark ? QColor(100, 215, 100) : QColor(0, 150, 0);
  return c;
}

/* a perceptually even scale that reads on light and dark backgrounds */
QRgb scale_color(double fraction) {
  static const int stops[][3] = {{68, 1, 84}, {59, 82, 139}, {33, 145, 140}, {94, 201, 98}, {253, 231, 37}};
  const double f = std::min(std::max(fraction, 0.0), 1.0) * 4.0;
  const int k = std::min((int)f, 3);
  const double t = f - k;
  return qRgb((int)(stops[k][0] + t * (stops[k + 1][0] - stops[k][0])), (int)(stops[k][1] + t * (stops[k + 1][1] - stops[k][1])),
              (int)(stops[k][2] + t * (stops[k + 1][2] - stops[k][2])));
}

/* the difference from a surface: blue is shallower, red is deeper, and grey agrees */
QRgb diverging_color(double fraction) {
  static const int stops[][3] = {{33, 102, 172}, {165, 165, 165}, {178, 24, 43}};
  const double f = std::min(std::max(fraction, 0.0), 1.0) * 2.0;
  const int k = std::min((int)f, 1);
  const double t = f - k;
  return qRgb((int)(stops[k][0] + t * (stops[k + 1][0] - stops[k][0])), (int)(stops[k][1] + t * (stops[k + 1][1] - stops[k][1])),
              (int)(stops[k][2] + t * (stops[k + 1][2] - stops[k][2])));
}

/* the range of a value over the soundings in view, without the extremes */
struct ValueRange {
  double low = 0.0, high = 1.0;
  double fraction(double value) const { return high > low ? (value - low) / (high - low) : 0.5; }
};

double beam_angle(const mbe::Ping &p, int j) { return p.altitude > 0.0 ? RTD * atan(p.bathacrosstrack[j] / p.altitude) : 0.0; }

ValueRange amplitude_range(const mbe::Session &s) {
  std::vector<double> values;
  for (int i = s.current(); i < s.current() + s.nplot(); i++) {
    const mbe::Ping &p = s.pings()[i];
    for (int j = 0; p.has_amp && j < p.beams_bath; j++) {
      if (mb_beam_ok(p.beamflag[j]))
        values.push_back(p.amp[j]);
    }
  }
  ValueRange range;
  if (!values.empty()) {
    std::sort(values.begin(), values.end());
    range.low = values[values.size() * 2 / 100];
    range.high = values[values.size() * 98 / 100];
  }
  return range;
}

QColor beam_color(const mbe::Session &s, const mbe::Ping &p, int j, const Colors &c, const ValueRange &range) {
  const char flag = p.beamflag[j];
  if (s.settings.show_mode == mbe::SHOW_AMPLITUDE)
    return p.has_amp ? QColor(scale_color(range.fraction(p.amp[j]))) : c.text;
  if (s.settings.show_mode == mbe::SHOW_ANGLE)
    return QColor(scale_color(range.fraction(beam_angle(p, j))));
  if (s.settings.show_mode == mbe::SHOW_FLAG) {
    if (mb_beam_ok(flag))
      return c.text;
    if (mb_beam_check_flag_filter2(flag) || mb_beam_check_flag_filter(flag))
      return c.blue;
    if (mb_beam_check_flag_sonar(flag))
      return c.green;
    return c.red;
  }
  if (s.settings.show_mode == mbe::SHOW_DETECT) {
    if (p.detect[j] == MB_DETECT_AMPLITUDE)
      return c.text;
    if (p.detect[j] == MB_DETECT_PHASE)
      return c.red;
    return c.green;
  }
  if (p.pulses[j] == MB_PULSE_CW)
    return c.text;
  if (p.pulses[j] == MB_PULSE_UPCHIRP)
    return c.red;
  if (p.pulses[j] == MB_PULSE_DOWNCHIRP)
    return c.blue;
  return c.green;
}

/* a square written straight into the image, much faster than a call per sounding */
void put_square(QImage &image, double ratio, int x, int y, int half, QRgb rgb, bool filled) {
  const int x0 = (int)((x - half) * ratio), y0 = (int)((y - half) * ratio);
  const int size = (int)(2 * half * ratio);
  if (x0 < 0 || y0 < 0 || x0 + size > image.width() || y0 + size > image.height())
    return;
  const int line = std::max(1, (int)ratio);
  const qsizetype stride = image.bytesPerLine() / 4;
  QRgb *bits = reinterpret_cast<QRgb *>(image.bits());
  for (int iy = y0; iy < y0 + size; iy++) {
    QRgb *row = bits + iy * stride;
    const bool edge_row = iy < y0 + line || iy >= y0 + size - line;
    for (int ix = x0; ix < x0 + size; ix++) {
      if (filled || edge_row || ix < x0 + line || ix >= x0 + size - line)
        row[ix] = rgb;
    }
  }
}

QFont plot_font() { return QFontDatabase::systemFont(QFontDatabase::FixedFont); }

QPixmap ring_pixmap(int radius, const QColor &color) {
  const int size = 2 * radius + 3;
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(color, 1.5));
  p.drawEllipse(QPointF(size / 2.0, size / 2.0), (double)radius, (double)radius);
  return pixmap;
}

}  // namespace

/*--------------------------------------------------------------------*/
Canvas::Canvas(mbe::Session &session, QWidget *parent) : QWidget(parent), session_(session) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setMinimumSize(320, 240);

  const QFontMetrics metrics(plot_font());
  const int char_width = metrics.horizontalAdvance('0');
  session_.text_width = [char_width](const char *text) { return char_width * (int)strlen(text); };
  session_.text_ascent = metrics.ascent();
  session_.text_descent = metrics.descent();

  step_timer_ = new QTimer(this);
  step_timer_->setSingleShot(false);
  connect(step_timer_, &QTimer::timeout, this, &Canvas::stepRepeat);
  updateCursor();
}

void Canvas::setMode(Mode mode) {
  mode_ = mode;
  hover_ = false;
  updateCursor();
  update();
}

void Canvas::setPreview(const mbe::FlagPreview *preview) {
  preview_ = preview;
  update();
}

void Canvas::setMarkedPing(int ping) {
  marked_ping_ = ping;
  update();
}

void Canvas::updateCursor() {
  if (mode_ == MODE_ERASE)
    setCursor(QCursor(ring_pixmap(mbe::ERASE_DISTANCE, colors_for(palette()).red)));
  else if (mode_ == MODE_RESTORE)
    setCursor(QCursor(ring_pixmap(mbe::ERASE_DISTANCE, colors_for(palette()).green)));
  else
    setCursor(Qt::CrossCursor);
}

void Canvas::setPlotRectFor(QSize size) {
  const QFontMetrics metrics(plot_font());
  const int char_width = metrics.horizontalAdvance('0');
  const int chars = session_.settings.show_time == mbe::PLOT_TIME ? LABEL_CHARS_TIME : LABEL_CHARS_PLAIN;
  const int xmin = chars * char_width + 32;
  const int xmax = size.width() - 7 * char_width;
  const int ymin = 3 * metrics.lineSpacing() + 10;
  const int ymax = size.height() - metrics.lineSpacing() - 8;
  session_.setPlotRect(size.width(), size.height(), xmin, xmax, ymin, ymax);
}

void Canvas::layoutChanged() {
  /* a thread is reading or writing the pings: the window is disabled, and does this when it ends */
  if (!isEnabled())
    return;
  setPlotRectFor(size());
  if (session_.isOpen() && session_.nbuff() > 0)
    session_.plot(false);
  redraw();
}

void Canvas::redraw() {
  image_ok_ = false;
  hover_ = false;
  update();
}

void Canvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  if (!isEnabled())
    return;
  layoutChanged();
  emit moved();
}

void Canvas::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange) {
    updateCursor();
    redraw();
  }
  QWidget::changeEvent(event);
}

/*--------------------------------------------------------------------*/
void Canvas::paintEvent(QPaintEvent *event) {
  const double ratio = devicePixelRatioF();
  QPainter painter(this);

  /* a thread is reading or writing the pings: only the last picture can be shown */
  if (!isEnabled()) {
    painter.fillRect(event->rect(), base_color(palette()));
    if (!image_.isNull())
      painter.drawImage(QRectF(QPointF(0, 0), QSizeF(image_.size()) / ratio), image_);
    return;
  }
  if (!image_ok_ || image_.size() != size() * ratio) {
    image_ = QImage(size() * ratio, QImage::Format_ARGB32_Premultiplied);
    paintPlot(image_, ratio);
    image_ok_ = true;
  }
  painter.drawImage(event->rect(), image_, QRectF(event->rect().topLeft() * ratio, event->rect().size() * ratio));

  if (preview_ != nullptr && session_.isOpen()) {
    const int first = session_.current(), last = first + session_.nplot();
    const mbe::PlotBox &box = session_.box();
    const bool dark = palette().color(QPalette::Base).lightness() < 128;
    const QPen flag_pen(dark ? QColor(255, 165, 60) : QColor(230, 110, 0), 1.5), unflag_pen(dark ? QColor(80, 215, 215) : QColor(0, 145, 150), 1.5);
    painter.setBrush(Qt::NoBrush);
    for (const mbe::FlagPreview::Item &item : preview_->items) {
      if (item.ping < first || item.ping >= last || item.ping >= session_.nbuff())
        continue;
      const mbe::Ping &p = session_.pings()[item.ping];
      if (item.beam >= (int)p.bath_x.size() || (p.bath_x[item.beam] == 0 && p.bath_y[item.beam] == 0))
        continue;
      const int x = p.bath_x[item.beam], y = p.bath_y[item.beam];
      if (session_.zoomed() && (x < box.xmin || x > box.xmax || y < box.ymin || y > box.ymax))
        continue;
      painter.setPen(mb_beam_ok(item.after) ? unflag_pen : flag_pen);
      painter.drawRect(x - 4, y - 4, 8, 8);
    }
  }
  if (marked_ping_ >= session_.current() && marked_ping_ < session_.current() + session_.nplot() && session_.isOpen() && marked_ping_ < session_.nbuff()) {
    const mbe::Ping &p = session_.pings()[marked_ping_];
    painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(p.label_x - p.label_width - 2, p.label_y - session_.text_ascent - 2, p.label_width + 4, session_.text_ascent + session_.text_descent + 4);
  }

  if (hover_ && session_.isOpen() && hover_ping_ < session_.nbuff()) {
    const mbe::Ping &p = session_.pings()[hover_ping_];
    if (hover_beam_ < (int)p.bath_x.size()) {
      painter.setRenderHint(QPainter::Antialiasing);
      painter.setPen(QPen(palette().color(QPalette::Highlight), 1.5));
      painter.setBrush(Qt::NoBrush);
      painter.drawEllipse(QPoint(p.bath_x[hover_beam_], p.bath_y[hover_beam_]), 6, 6);
    }
  }
  if (lasso_.size() > 1) {
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(colors_for(palette()).red, 1));
    painter.drawPolyline(lasso_);
  }
  int xmin, xmax, ymin, ymax;
  if (session_.grabRect(xmin, xmax, ymin, ymax)) {
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(colors_for(palette()).red, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(xmin, ymin, xmax - xmin, ymax - ymin);
  }
}

QImage Canvas::render(QSize size, double ratio) {
  setPlotRectFor(size);
  if (session_.isOpen() && session_.nbuff() > 0)
    session_.plot(false);
  QImage image(size * ratio, QImage::Format_ARGB32_Premultiplied);
  paintPlot(image, ratio);
  layoutChanged();
  return image;
}

void Canvas::paintPlot(QImage &image, double ratio) {
  const Colors colors = colors_for(palette());
  image.fill(base_color(palette()));
  QPainter painter(&image);
  painter.scale(ratio, ratio);
  painter.setFont(plot_font());
  const QFontMetrics metrics(painter.font());
  const int line_height = metrics.lineSpacing();

  if (!session_.isOpen() || session_.nbuff() == 0 || session_.nplot() <= 0) {
    painter.setPen(colors.text);
    painter.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, session_.isOpen() ? "No data" : "No file open");
    return;
  }

  const mbe::PlotBox &box = session_.box();
  const mbe::PlotSettings &set = session_.settings;
  const int width_px = image.width() / ratio;
  const int xcen = box.xmin + (box.xmax - box.xmin) / 2;
  const std::vector<mbe::Ping> &pings = session_.pings();
  const int first = session_.current(), last = first + session_.nplot();

  ValueRange range;
  if (set.show_mode == mbe::SHOW_AMPLITUDE)
    range = amplitude_range(session_);
  else if (set.show_mode == mbe::SHOW_ANGLE) {
    range.low = 1.0e30;
    range.high = -1.0e30;
    for (int i = first; i < last; i++) {
      for (int j = 0; j < pings[i].beams_bath; j++) {
        if (mb_beam_ok(pings[i].beamflag[j])) {
          range.low = std::min(range.low, beam_angle(pings[i], j));
          range.high = std::max(range.high, beam_angle(pings[i], j));
        }
      }
    }
  }

  else if (set.show_mode == mbe::SHOW_RESIDUAL) {
    /* symmetric about zero, without the extremes */
    std::vector<double> sizes, values;
    for (int i = first; i < last; i++) {
      session_.residuals(i, nullptr, values);
      for (int j = 0; j < pings[i].beams_bath; j++) {
        if (mb_beam_ok(pings[i].beamflag[j]) && !std::isnan(values[j]))
          sizes.push_back(fabs(values[j]));
      }
    }
    double size = 1.0;
    if (!sizes.empty()) {
      std::nth_element(sizes.begin(), sizes.begin() + sizes.size() * 98 / 100, sizes.end());
      size = std::max(sizes[sizes.size() * 98 / 100], 0.01);
    }
    range.low = -size;
    range.high = size;
  }

  /* the file and the state of the plot */
  painter.setPen(colors.text);
  painter.drawText(6, line_height, fileText);
  const std::vector<std::string> info = session_.infoText();
  auto centered = [&](int row, const QString &text) {
    painter.drawText(std::max(0, (width_px - metrics.horizontalAdvance(text)) / 2), row * line_height, text);
  };
  if (!info.empty()) {
    for (size_t i = 0; i < info.size(); i++)
      centered((int)i + 1, QString::fromStdString(info[i]));
  }
  else {
    centered(1, pings[first].dualprofile ? "Dual Swath per Ping" : "Single Swath per Ping");
    struct Part {
      QString text;
      QColor color;
    };
    std::vector<Part> parts;
    if (set.show_mode == mbe::SHOW_AMPLITUDE || set.show_mode == mbe::SHOW_ANGLE || set.show_mode == mbe::SHOW_RESIDUAL) {
      /* a colour bar with its range */
      const bool residual = set.show_mode == mbe::SHOW_RESIDUAL;
      const QString name = residual ? "Sounding Depth minus Reference (m):" : set.show_mode == mbe::SHOW_AMPLITUDE ? "Sounding Colors by Amplitude:" : "Sounding Colors by Beam Angle (deg):";
      const QString low = QString::number(range.low, 'f', 1), high = QString::number(range.high, 'f', 1);
      const int bar = 160;
      int x = std::max(0, (width_px - metrics.horizontalAdvance(name + low + high) - bar - 3 * 8) / 2);
      painter.drawText(x, 2 * line_height, name);
      x += metrics.horizontalAdvance(name) + 8;
      painter.drawText(x, 2 * line_height, low);
      x += metrics.horizontalAdvance(low) + 8;
      for (int k = 0; k < bar; k++)
        painter.fillRect(x + k, 2 * line_height - metrics.ascent(), 1, metrics.ascent(), QColor(residual ? diverging_color((double)k / bar) : scale_color((double)k / bar)));
      x += bar + 8;
      painter.drawText(x, 2 * line_height, high);
    }
    else if (set.show_mode == mbe::SHOW_FLAG)
      parts = {{"Sounding Colors by Flagging:  Unflagged  ", colors.text}, {"Manual  ", colors.red}, {"Filter  ", colors.blue}, {"Sonar", colors.green}};
    else if (set.show_mode == mbe::SHOW_DETECT)
      parts = {{"Sounding Colors by Bottom Detection:  Amplitude  ", colors.text}, {"Phase  ", colors.red}, {"Unknown", colors.green}};
    else
      parts = {{"Sounding Colors by Source Type:  CW  ", colors.text}, {"Up-Chirp  ", colors.red}, {"Down-Chirp  ", colors.blue}, {"Unknown", colors.green}};
    int total = 0;
    for (const Part &part : parts)
      total += metrics.horizontalAdvance(part.text);
    int x = std::max(0, (width_px - total) / 2);
    for (const Part &part : parts) {
      painter.setPen(part.color);
      painter.drawText(x, 2 * line_height, part.text);
      x += metrics.horizontalAdvance(part.text);
    }
    painter.setPen(colors.text);
    centered(3, QString("Vertical exaggeration: %1   All Distances and Depths in Meters").arg(set.exagger / 100.0, 4, 'f', 2));
  }

  /* axes and grid */
  const double xscale = 100.0 * set.plot_width / (box.xmax - box.xmin);
  const double yscale = (xscale * 100.0) / set.exagger;
  const double dxscale = 100.0 / xscale, dyscale = 100.0 / yscale;
  QPen dash(colors.text, 0, Qt::DashLine);
  QPen solid(colors.text, 0, Qt::SolidLine);
  painter.setPen(solid);
  painter.drawRect(box.xmin, box.ymin, box.xmax - box.xmin, box.ymax - box.ymin);
  const double half_width = 0.5 * (box.xmax - box.xmin) / dxscale;
  const double pan = session_.panAcross();
  if ((int)(set.x_interval * dxscale) > 0) {
    const int first_tick = (int)std::ceil((pan - half_width) / set.x_interval);
    const int last_tick = (int)std::floor((pan + half_width) / set.x_interval);
    for (int i = first_tick; i <= last_tick; i++) {
      const int x = xcen + (int)((i * set.x_interval - pan) * dxscale);
      const QString text = QString::number(std::abs(i) * set.x_interval);
      painter.setPen(dash);
      painter.drawLine(x, box.ymin, x, box.ymax);
      painter.setPen(solid);
      painter.drawText(x - metrics.horizontalAdvance(text) / 2, box.ymax + metrics.ascent() + 3, text);
    }
  }
  const int y_int = (int)(set.y_interval * dyscale);
  if (y_int > 0) {
    const double dy_height = (box.ymax - box.ymin) / dyscale;
    const int ny_int = (int)(dy_height / set.y_interval + 1);
    for (int i = 0; i < ny_int; i++) {
      const int yy = i * y_int;
      painter.setPen(dash);
      painter.drawLine(box.xmin, box.ymax - yy, box.xmax, box.ymax - yy);
      painter.setPen(solid);
      painter.drawText(box.xmax + 4, box.ymax - yy + metrics.ascent() / 2, QString::number(i * set.y_interval));
    }
  }

  /* profile lines, in one call per colour */
  long soundings = 0;
  std::vector<QLine> lines_good, lines_flagged;
  for (int i = first; i < last; i++) {
    const mbe::Ping &p = pings[i];
    bool started = false, last_flagged = false, dualbreak = false;
    int xold = 0, yold = 0;
    soundings += p.beams_bath;
    for (int j = 0; j < p.beams_bath; j++) {
      const char flag = p.beamflag[j];
      const bool ok = mb_beam_ok(flag);
      const bool flagged = !ok && !mb_beam_check_flag_unusable2(flag);
      if (p.dualprofile && p.dualprofilebeam == j)
        dualbreak = true;
      if (!ok && !(set.show_flaggedprofiles && flagged))
        continue;
      if (started) {
        if (!dualbreak) {
          const QLine line(xold, yold, p.bath_x[j], p.bath_y[j]);
          if (ok && !last_flagged)
            lines_good.push_back(line);
          else
            lines_flagged.push_back(line);
        }
        else {
          dualbreak = false;
        }
      }
      started = true;
      last_flagged = !ok;
      xold = p.bath_x[j];
      yold = p.bath_y[j];
    }
  }
  if (session_.zoomed())
    painter.setClipRect(box.xmin, box.ymin, box.xmax - box.xmin, box.ymax - box.ymin);
  painter.setRenderHint(QPainter::Antialiasing, soundings < MANY_SOUNDINGS);
  painter.setPen(QPen(colors.text, 0));
  painter.drawLines(lines_good.data(), (int)lines_good.size());
  painter.setPen(QPen(colors.red, 0));
  painter.drawLines(lines_flagged.data(), (int)lines_flagged.size());
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setClipping(false);

  /* soundings */
  const mbe::BeamInfo &beam_info = session_.info();
  std::vector<double> residuals;
  for (int i = first; i < last; i++) {
    const mbe::Ping &p = pings[i];
    if (set.show_mode == mbe::SHOW_RESIDUAL)
      session_.residuals(i, nullptr, residuals);
    for (int j = 0; j < p.beams_bath; j++) {
      const char flag = p.beamflag[j];
      if (mb_beam_check_flag_unusable2(flag))
        continue;
      if (session_.zoomed() && (p.bath_x[j] < box.xmin || p.bath_x[j] > box.xmax || p.bath_y[j] < box.ymin || p.bath_y[j] > box.ymax))
        continue;
      const bool ok = mb_beam_ok(flag);
      if (beam_info.set && beam_info.ping == i && beam_info.beam == j)
        put_square(image, ratio, p.bath_x[j], p.bath_y[j], 4, colors.blue.rgba(), true);
      else if (ok || (set.show_flaggedsoundings && (mbe::Session::flagReason(flag) & session_.shown_reasons) != 0))
        put_square(image, ratio, p.bath_x[j], p.bath_y[j], 2,
                   set.show_mode != mbe::SHOW_RESIDUAL ? beam_color(session_, p, j, colors, range).rgba()
                   : std::isnan(residuals[j])          ? colors.green.rgba()
                                                       : diverging_color(range.fraction(residuals[j])),
                   ok);
    }
  }

  /* ping labels: colours and zap boxes on every ping, the text only where it does not overlap */
  int last_text_y = INT_MAX;
  for (int i = first; i < last; i++) {
    const mbe::Ping &p = pings[i];
    const int top = p.label_y - session_.text_ascent, height_px = session_.text_ascent + session_.text_descent;
    if (p.outbounds == mbe::OUTBOUNDS_UNFLAGGED) {
      painter.fillRect(p.label_x - p.label_width, top, p.label_width, height_px, colors.red);
      painter.setPen(colors.text);
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(p.zap_x1, p.zap_y1, 10, height_px);
    }
    else if (p.outbounds == mbe::OUTBOUNDS_FLAGGED) {
      painter.fillRect(p.label_x - p.label_width, top, p.label_width, height_px, colors.green);
    }
    if (last_text_y - p.label_y >= line_height) {
      painter.setPen(p.outbounds == mbe::OUTBOUNDS_NONE ? colors.text : QColor(Qt::black));
      painter.drawText(p.label_x - p.label_width, p.label_y, QString::fromStdString(p.label));
      last_text_y = p.label_y;
    }
  }
}

/*--------------------------------------------------------------------*/
void Canvas::applyHeldKey() {
  if (key_z_)
    session_.badPing();
  else if (key_s_)
    session_.goodPing();
  else if (key_a_)
    reverseKeys ? session_.rightPing() : session_.leftPing();
  else if (key_d_)
    reverseKeys ? session_.leftPing() : session_.rightPing();
}

void Canvas::editAt(int x, int y, int grab_mode) {
  int status = MB_SUCCESS;
  switch (mode_) {
  case MODE_TOGGLE:
    status = session_.mouseToggle(x, y);
    break;
  case MODE_PICK:
    status = session_.mousePick(x, y);
    break;
  case MODE_ERASE:
    status = session_.mouseErase(x, y);
    break;
  case MODE_RESTORE:
    status = session_.mouseRestore(x, y);
    break;
  case MODE_GRAB:
    status = session_.mouseGrab(grab_mode, x, y);
    break;
  case MODE_LASSO:
    break;
  case MODE_INFO: {
    status = session_.mouseInfo(x, y);
    const std::vector<std::string> info = session_.infoText();
    if (!info.empty()) {
      QStringList lines;
      for (const std::string &line : info)
        lines << QString::fromStdString(line);
      emit soundingInfo(lines);
    }
    break;
  }
  }
  if (status == MB_SUCCESS)
    applyHeldKey();
  else
    QApplication::beep();
  redraw();
  emit edited();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
  const Qt::MouseButton button = event->button();
  const bool edit = (button == Qt::LeftButton && !reverseMouse) || (button == Qt::RightButton && reverseMouse);
  const bool forward = (button == Qt::RightButton && !reverseMouse) || (button == Qt::LeftButton && reverseMouse);
  const QPoint position = event->position().toPoint();
  if (edit && key_space_) {
    panning_ = true;
    last_ = position;
  }
  else if (edit && mode_ == MODE_LASSO) {
    lasso_.clear();
    lasso_ << position;
  }
  else if (edit) {
    dragging_ = mode_ == MODE_ERASE || mode_ == MODE_RESTORE || mode_ == MODE_GRAB;
    last_ = position;
    /* a click or a drag, with a key held or not, is one step of undo */
    session_.beginStroke();
    stroke_ = true;
    editAt(position.x(), position.y(), mbe::GRAB_START);
  }
  else if (forward && event->modifiers().testFlag(Qt::ShiftModifier)) {
    /* Shift and the right button open the context menu; -1 if no sounding is near */
    int iping = -1, jbeam = -1;
    if (!session_.nearestBeam(position.x(), position.y(), false, &iping, &jbeam))
      iping = jbeam = -1;
    emit contextMenuAt(event->globalPosition().toPoint(), iping, jbeam);
  }
  else if (forward || button == Qt::MiddleButton) {
    /* one step now, and if the button stays down, more after 2 seconds */
    step_direction_ = forward ? 1 : -1;
    stepBy(step_direction_);
    step_timer_->start(2000);
  }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
  const QPoint position = event->position().toPoint();
  if (panning_) {
    session_.panBy(position.x() - last_.x(), position.y() - last_.y());
    last_ = position;
    redraw();
  }
  else if (!lasso_.isEmpty()) {
    lasso_ << position;
    update();
  }
  else if (dragging_) {
    if (mode_ == MODE_GRAB) {
      editAt(position.x(), position.y(), mbe::GRAB_MOVE);
    }
    else {
      /* no gaps between the positions the system reports on a fast stroke */
      const QPoint delta = position - last_;
      const int nsteps = std::max(1, std::max(std::abs(delta.x()), std::abs(delta.y())) / (mbe::ERASE_DISTANCE / 2));
      for (int i = 1; i <= nsteps; i++)
        editAt(last_.x() + delta.x() * i / nsteps, last_.y() + delta.y() * i / nsteps, mbe::GRAB_MOVE);
    }
    last_ = position;
  }
  else if (step_direction_ == 0) {
    updateHover(position);
  }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
  const QPoint position = event->position().toPoint();
  if (dragging_ && mode_ == MODE_GRAB)
    editAt(position.x(), position.y(), mbe::GRAB_END);
  dragging_ = false;
  panning_ = false;
  if (stroke_) {
    static const char *names[] = {"Toggle", "Pick", "Erase", "Restore", "Grab", "Info", "Lasso"};
    session_.endStroke(names[mode_]);
    stroke_ = false;
    emit edited();
  }
  if (!lasso_.isEmpty()) {
    lasso_ << position;
    std::vector<std::pair<int, int>> polygon;
    for (const QPoint &point : lasso_)
      polygon.emplace_back(point.x(), point.y());
    lasso_.clear();
    if (session_.mouseLasso(polygon) != MB_SUCCESS)
      QApplication::beep();
    redraw();
    emit edited();
  }
  step_direction_ = 0;
  step_timer_->stop();
}

void Canvas::updateHover(QPoint position) {
  const bool had = hover_;
  hover_ = false;
  if (mode_ == MODE_TOGGLE || mode_ == MODE_PICK || mode_ == MODE_INFO)
    hover_ = session_.nearestBeam(position.x(), position.y(), mode_ == MODE_PICK, &hover_ping_, &hover_beam_);
  if (had || hover_)
    update();
}

void Canvas::leaveEvent(QEvent *event) {
  hover_ = false;
  update();
  QWidget::leaveEvent(event);
}

void Canvas::stepBy(int direction) {
  if (!session_.isOpen())
    return;
  int nsteps = direction * stepSize;
  if (key_g_)
    nsteps = direction > 0 ? session_.nbuff() - session_.current() - 1 : -session_.current();
  if (session_.step(nsteps) != MB_SUCCESS)
    QApplication::beep();
  redraw();
  emit moved();
}

void Canvas::stepRepeat() {
  if (step_direction_ == 0) {
    step_timer_->stop();
    return;
  }
  step_timer_->setInterval(50);
  stepBy(step_direction_);
}

void Canvas::wheelEvent(QWheelEvent *event) {
  /* with a modifier the wheel zooms; up makes things bigger */
  const int delta = event->angleDelta().y() != 0 ? event->angleDelta().y() : event->angleDelta().x();
  if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
    if (event->modifiers() & Qt::ControlModifier)
      session_.zoomWidth((int)event->position().x(), std::pow(1.1, -delta / 120.0));
    else
      session_.zoomExaggeration(std::pow(1.1, delta / 120.0));
    redraw();
    emit viewChanged();
    event->accept();
    return;
  }

  /* a mouse wheel notch is 120, a trackpad sends many small amounts */
  wheel_ += event->angleDelta().y();
  while (std::abs(wheel_) >= 120) {
    stepBy(wheel_ > 0 ? 1 : -1);
    wheel_ -= wheel_ > 0 ? 120 : -120;
  }
  event->accept();
}

bool Canvas::event(QEvent *event) {
  /* the pinch of a trackpad */
  if (event->type() == QEvent::NativeGesture) {
    auto *gesture = static_cast<QNativeGestureEvent *>(event);
    if (gesture->gestureType() == Qt::ZoomNativeGesture) {
      session_.zoomWidth((int)gesture->position().x(), 1.0 / std::max(1.0 + gesture->value(), 0.1));
      redraw();
      emit viewChanged();
      return true;
    }
  }
  return QWidget::event(event);
}

void Canvas::keyPressEvent(QKeyEvent *event) {
  if (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier)) {
    QWidget::keyPressEvent(event);
    return;
  }
  if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
    if (session_.isOpen() && session_.step(event->key() == Qt::Key_Right ? 1 : -1) == MB_SUCCESS) {
      redraw();
      emit moved();
    }
    return;
  }
  const QString text = event->text();
  if (text.isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  const char key = text[0].toLatin1();

  if (event->key() == Qt::Key_Space) {
    key_space_ = true;
    setCursor(Qt::OpenHandCursor);
    event->accept();
    return;
  }

  /* only the stepping keys repeat while held */
  if (event->isAutoRepeat() && !strchr("FfVv", key)) {
    event->accept();
    return;
  }
  auto hold = [this](bool z, bool s, bool a, bool d) {
    key_z_ = z;
    key_s_ = s;
    key_a_ = a;
    key_d_ = d;
  };
  auto finish_edit = [this](int status) {
    (void)status;
    redraw();
    emit edited();
  };
  auto mode = [this](Mode m) {
    setMode(m);
    emit modeKey(m);
  };
  switch (key) {
  case 'F':
  case 'f':
    stepBy(1);
    break;
  case 'V':
  case 'v':
    stepBy(-1);
    break;
  case 'G':
  case 'g':
    key_g_ = true;
    break;
  case 'M':
  case 'm':
  case 'Z':
  case 'z':
    finish_edit(session_.badPing());
    hold(true, false, false, false);
    break;
  case 'K':
  case 'k':
  case 'S':
  case 's':
    finish_edit(session_.goodPing());
    hold(false, true, false, false);
    break;
  case 'J':
  case 'j':
  case 'A':
  case 'a':
    finish_edit(reverseKeys ? session_.rightPing() : session_.leftPing());
    hold(false, false, true, false);
    break;
  case 'L':
  case 'l':
  case 'D':
  case 'd':
    finish_edit(reverseKeys ? session_.leftPing() : session_.rightPing());
    hold(false, false, false, true);
    break;
  case '<':
  case ',':
  case 'X':
  case 'x':
    finish_edit(session_.flagView());
    break;
  case '>':
  case '.':
  case 'C':
  case 'c':
    finish_edit(session_.unflagView());
    break;
  case '!':
    finish_edit(session_.zeroPing());
    break;
  case 'U':
  case 'u':
  case 'Q':
  case 'q':
    mode(MODE_TOGGLE);
    break;
  case 'I':
  case 'i':
  case 'W':
  case 'w':
    mode(MODE_PICK);
    break;
  case 'O':
  case 'o':
  case 'E':
  case 'e':
    mode(MODE_ERASE);
    break;
  case 'P':
  case 'p':
  case 'R':
  case 'r':
    mode(MODE_RESTORE);
    break;
  case '{':
  case '[':
  case 'T':
  case 't':
    mode(MODE_GRAB);
    break;
  case '}':
  case ']':
  case 'Y':
  case 'y':
    mode(MODE_INFO);
    break;
  case 'H':
  case 'h':
    mode(MODE_LASSO);
    break;
  case 'N':
  case 'n':
    emit suspectKey(1);
    break;
  case 'B':
  case 'b':
    emit suspectKey(-1);
    break;
  case '0':
    session_.resetView();
    redraw();
    emit viewChanged();
    break;
  case '2':
  case '@':
    emit viewKey(mbe::VIEW_WATERFALL);
    break;
  case '3':
  case '#':
    emit viewKey(mbe::VIEW_ALONGTRACK);
    break;
  case '4':
  case '$':
    emit viewKey(mbe::VIEW_ACROSSTRACK);
    break;
  default:
    QWidget::keyPressEvent(event);
    return;
  }
  event->accept();
}

void Canvas::keyReleaseEvent(QKeyEvent *event) {
  if (!event->isAutoRepeat() && event->key() == Qt::Key_Space) {
    key_space_ = false;
    updateCursor();
    event->accept();
    return;
  }
  if (event->isAutoRepeat() || event->text().isEmpty()) {
    QWidget::keyReleaseEvent(event);
    return;
  }
  switch (event->text()[0].toLatin1()) {
  case 'G':
  case 'g':
    key_g_ = false;
    break;
  case 'M':
  case 'm':
  case 'Z':
  case 'z':
    key_z_ = false;
    break;
  case 'K':
  case 'k':
  case 'S':
  case 's':
    key_s_ = false;
    break;
  case 'J':
  case 'j':
  case 'A':
  case 'a':
    key_a_ = false;
    break;
  case 'L':
  case 'l':
  case 'D':
  case 'd':
    key_d_ = false;
    break;
  default:
    QWidget::keyReleaseEvent(event);
    return;
  }
}

void Canvas::focusOutEvent(QFocusEvent *event) {
  key_g_ = key_z_ = key_s_ = key_a_ = key_d_ = false;
  if (key_space_) {
    key_space_ = false;
    updateCursor();
  }
  QWidget::focusOutEvent(event);
}

}  // namespace mbe
