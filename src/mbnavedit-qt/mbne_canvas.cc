/*--------------------------------------------------------------------
 *    The MB-system:  mbne_canvas.cc  9/11/2026
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

#include "mbne_canvas.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include <QFileInfo>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#include "mb_define.h"

namespace mbne {

namespace {

constexpr int DECIMATE_ABOVE = 20000;     // records in view: above this, one mark per pixel column
constexpr int ANTIALIAS_UP_TO = 100000;  // records in view
constexpr int SQUARE = 4;
constexpr int BAR_WIDTH = 160;

struct Colors {
  QColor background, text, grid, red, blue, green, purple, orange, bar;
};

Colors colors_for(const QPalette &palette) {
  Colors c;
  QColor base = palette.color(QPalette::Base);
  base.setAlpha(255);
  const bool dark = base.lightness() < 128;
  c.background = dark ? base : QColor(Qt::white);
  c.text = dark ? QColor(235, 235, 235) : QColor(Qt::black);
  c.grid = dark ? QColor(150, 150, 150) : QColor(Qt::black);
  c.red = dark ? QColor(255, 110, 110) : QColor(220, 0, 0);
  c.blue = dark ? QColor(120, 170, 255) : QColor(0, 0, 230);
  c.green = dark ? QColor(100, 215, 100) : QColor(0, 150, 0);
  c.purple = dark ? QColor(215, 140, 255) : QColor(140, 0, 170);
  c.orange = dark ? QColor(255, 180, 80) : QColor(235, 120, 0);
  c.bar = dark ? QColor(95, 95, 95) : QColor(211, 211, 211);
  return c;
}

QString format_value(int type, double value) {
  char text[64];
  snprintf(text, sizeof(text), type == PLOT_LONGITUDE || type == PLOT_LATITUDE ? "%11.6f" : "%6.2f", value);
  return QString::fromLatin1(text).trimmed();
}

int x_of(const Plot &p, const Record &r) { return p.ixmin + p.xscale * (r.file_time_d - p.xmin); }
int y_of(const Plot &p, double value) { return p.iymin + p.yscale * (value - p.ymin); }

/* a line through the records of the view */
void draw_series(QPainter &painter, const Plot &p, const std::vector<Record> &rec, int first, int count, double Record::*field, const QColor &color,
                 long *counter) {
  if (count < 2)
    return;
  if (counter != nullptr)
    *counter += count - 1;
  painter.setPen(QPen(color, 1.0));
  if (count > DECIMATE_ABOVE) {
    /* one vertical line per pixel column, reaching the end of the column before */
    std::vector<QLine> columns;
    int i = first;
    int previous = y_of(p, rec[i].*field);
    while (i < first + count) {
      const int x = x_of(p, rec[i]);
      int lo = previous, hi = previous;
      while (i < first + count && x_of(p, rec[i]) == x) {
        previous = y_of(p, rec[i].*field);
        lo = std::min(lo, previous);
        hi = std::max(hi, previous);
        i++;
      }
      columns.emplace_back(x, lo, x, hi);
    }
    painter.drawLines(columns.data(), (int)columns.size());
    return;
  }
  std::vector<QPoint> points;
  points.reserve(count);
  for (int i = first; i < first + count; i++)
    points.emplace_back(x_of(p, rec[i]), y_of(p, rec[i].*field));
  painter.drawPolyline(points.data(), (int)points.size());
}

/* squares, joined into columns when there are many */
class Squares {
 public:
  explicit Squares(bool many) : many_(many) {}
  void add(int x, int y) {
    if (!many_) {
      rects_.emplace_back(x - SQUARE / 2, y - SQUARE / 2, SQUARE, SQUARE);
      return;
    }
    if (open_ && x == x_) {
      lo_ = std::min(lo_, y);
      hi_ = std::max(hi_, y);
      return;
    }
    flush();
    open_ = true;
    x_ = x;
    lo_ = hi_ = y;
  }
  void fill(QPainter &painter, const QColor &color) {
    flush();
    if (rects_.empty())
      return;
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRects(rects_.data(), (int)rects_.size());
    painter.setBrush(Qt::NoBrush);
  }
  void outline(QPainter &painter, const QColor &color) {
    flush();
    if (rects_.empty())
      return;
    painter.setPen(QPen(color, 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRects(rects_.data(), (int)rects_.size());
  }

 private:
  void flush() {
    if (open_)
      rects_.emplace_back(x_ - SQUARE / 2, lo_ - SQUARE / 2, SQUARE, hi_ - lo_ + SQUARE);
    open_ = false;
  }
  bool many_;
  bool open_ = false;
  int x_ = 0, lo_ = 0, hi_ = 0;
  std::vector<QRect> rects_;
};

}  // namespace

void layout_plots(Session &session, const QSize &size, const QFont &font) {
  const QFontMetrics metrics(font);
  const int count = std::max(session.numberPlots(), 1);
  const int plot_height = std::max(size.height() / count, 1);
  const int left = metrics.horizontalAdvance("-179.123456") + 8 + metrics.height() + 6;
  const int top = metrics.height() + 8;
  const int bottom = metrics.height() + 8 + (plot_height >= 170 ? metrics.height() + 2 : 0);
  session.setPlotSize(size.width(), plot_height);
  session.setPlotMargins(left, 12, top, bottom);
  if (session.isOpen())
    session.plotAll();
}

void draw_plots(QPainter &painter, const Session &session, const QSize &size, const QPalette &palette, DrawCounts *counts) {
  const Colors c = colors_for(palette);
  painter.fillRect(QRect(QPoint(0, 0), size), c.background);
  const int count = session.nplot();
  if (!session.isOpen() || count <= 0 || session.plots().empty())
    return;
  const std::vector<Record> &rec = session.records();
  const int first = std::max(session.current(), 0);
  const QFontMetrics metrics(painter.font());
  const Show &show = session.show;
  const bool many = count > DECIMATE_ABOVE;
  painter.setRenderHint(QPainter::Antialiasing, false);

  DrawCounts local;
  for (size_t k = 0; k < session.plots().size(); k++) {
    const Plot &p = session.plots()[k];
    painter.setPen(c.text);

    /* the file and the place in it, once */
    if (k == 0) {
      const int y = p.iymax - 6;
      const int bar_x = p.ixmax - BAR_WIDTH - metrics.horizontalAdvance(QString::number(session.nbuff())) - 4;
      const QString file = "Data File: " + QString::fromLocal8Bit(session.file());
      painter.drawText(p.ixmin, y, metrics.elidedText(file, Qt::ElideMiddle, std::max(bar_x - p.ixmin - 30, 40)));
      int fill_x = bar_x + (BAR_WIDTH * session.current()) / session.nbuff();
      const int fill_w = std::max((BAR_WIDTH * count) / session.nbuff(), 5);
      if (fill_w > BAR_WIDTH)
        fill_x = bar_x;
      const int bar_h = metrics.ascent();
      const int bar_y = y - bar_h + 1;
      painter.fillRect(fill_x, bar_y, std::min(fill_w, BAR_WIDTH), bar_h, c.bar);
      painter.setPen(c.grid);
      painter.drawRect(bar_x, bar_y, BAR_WIDTH, bar_h);
      painter.setPen(c.text);
      painter.drawText(bar_x - metrics.horizontalAdvance("0 "), y, "0");
      painter.drawText(bar_x + BAR_WIDTH + 4, y, QString::number(session.nbuff()));
    }

    /* the label of the time axis, when there is room */
    const int bottom_room = k * session.plotHeight() + session.plotHeight() - p.iymin;
    if (bottom_room >= 2 * metrics.height())
      painter.drawText(QRect(p.ixmin, p.iymin + 6 + metrics.height(), p.ixmax - p.ixmin, metrics.height()), Qt::AlignHCenter,
                       QString::fromStdString(p.xlabel));

    /* the label of the vertical axis, along it */
    painter.save();
    painter.translate(metrics.ascent() + 2, (p.iymin + p.iymax) / 2);
    painter.rotate(-90.0);
    painter.drawText(QRect(-(p.iymin - p.iymax) / 2, -metrics.ascent(), p.iymin - p.iymax, metrics.height()), Qt::AlignHCenter,
                     metrics.elidedText(QString::fromStdString(p.ylabel1 + " " + p.ylabel2), Qt::ElideRight, p.iymin - p.iymax));
    painter.restore();

    /* the times along the axis, as many as fit */
    const double dx = (session.plotEndTime() - session.plotStartTime()) / 5;
    const int tick_width = metrics.horizontalAdvance("00:00:00.000") + 8;
    const int every = tick_width > 0 ? std::clamp((tick_width * 5) / std::max(p.ixmax - p.ixmin, 1) + 1, 1, 5) : 1;
    for (int i = 0; i < 6; i++) {
      double x = session.plotStartTime() + i * dx;
      const int ix = p.ixmin + p.xscale * (x - p.xmin);
      x += session.fileStartTime();
      painter.setPen(c.grid);
      painter.drawLine(ix, p.iymin, ix, p.iymin + 5);
      if (i % every != 0)
        continue;
      int time_i[7];
      mb_get_date(0, x, time_i);
      char text[32];
      snprintf(text, sizeof(text), "%2.2d:%2.2d:%2.2d.%3.3d", time_i[3], time_i[4], time_i[5], (int)(0.001 * time_i[6]));
      painter.setPen(c.text);
      /* the first and last labels stay inside the drawing area */
      const int label_x = std::min(ix - tick_width / 2, size.width() - tick_width);
      painter.drawText(QRect(label_x, p.iymin + 6, tick_width, metrics.height()), Qt::AlignHCenter, text);
    }

    /* the range of values, and zero */
    const int value_right = p.ixmin - 4;
    const QRect value_rect(0, 0, value_right, metrics.height());
    painter.setPen(c.text);
    painter.drawText(value_rect.translated(0, p.iymin - metrics.height() / 2), Qt::AlignRight, format_value(p.type, p.ymin));
    painter.drawText(value_rect.translated(0, p.iymax - metrics.height() / 2), Qt::AlignRight, format_value(p.type, p.ymax));
    if ((p.ymax > 0.0 && p.ymin < 0.0) || (p.ymax < 0.0 && p.ymin > 0.0)) {
      const int iyzero = p.iymin - p.yscale * p.ymin;
      if (std::abs(iyzero - p.iymin) > metrics.height() && std::abs(iyzero - p.iymax) > metrics.height())
        painter.drawText(value_rect.translated(0, iyzero - metrics.height() / 2), Qt::AlignRight, format_value(p.type, 0.0));
      painter.setPen(QPen(c.grid, 1.0, Qt::DashLine));
      painter.drawLine(p.ixmin, iyzero, p.ixmax, iyzero);
    }

    /* the frame */
    painter.setPen(QPen(c.grid, 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(p.ixmin, p.iymax, p.ixmax - p.ixmin, p.iymin - p.iymax);
    painter.drawRect(p.ixmin - 1, p.iymax - 1, p.ixmax - p.ixmin + 2, p.iymin - p.iymax + 2);

    /* the lines under the points */
    painter.setRenderHint(QPainter::Antialiasing, count <= ANTIALIAS_UP_TO);
    double Record::*value = nullptr;
    switch (p.type) {
    case PLOT_TINT:
      value = &Record::tint;
      if (show.tint_org)
        draw_series(painter, p, rec, first, count, &Record::tint_org, c.green, &local.line_green);
      break;
    case PLOT_LONGITUDE:
      value = &Record::lon;
      if (show.lon_org)
        draw_series(painter, p, rec, first, count, &Record::lon_org, c.green, &local.line_green);
      if (session.model_mode != MODEL_OFF && show.lon_dr)
        draw_series(painter, p, rec, first, count, &Record::lon_dr, c.blue, &local.line_blue);
      break;
    case PLOT_LATITUDE:
      value = &Record::lat;
      if (show.lat_org)
        draw_series(painter, p, rec, first, count, &Record::lat_org, c.green, &local.line_green);
      if (session.model_mode != MODEL_OFF && show.lat_dr)
        draw_series(painter, p, rec, first, count, &Record::lat_dr, c.blue, &local.line_blue);
      break;
    case PLOT_SPEED:
      value = &Record::speed;
      if (show.speed_org)
        draw_series(painter, p, rec, first, count, &Record::speed_org, c.green, &local.line_green);
      if (show.smg)
        draw_series(painter, p, rec, first, count, &Record::speed_made_good, c.blue, &local.line_blue);
      break;
    case PLOT_HEADING:
      value = &Record::heading;
      if (show.heading_org)
        draw_series(painter, p, rec, first, count, &Record::heading_org, c.green, &local.line_green);
      if (show.cmg)
        draw_series(painter, p, rec, first, count, &Record::course_made_good, c.blue, &local.line_blue);
      break;
    case PLOT_DRAFT:
      value = &Record::draft;
      if (show.draft_org)
        draw_series(painter, p, rec, first, count, &Record::draft_org, c.green, &local.line_green);
      break;
    case PLOT_ROLL:
      draw_series(painter, p, rec, first, count, &Record::roll, c.green, &local.line_green);
      break;
    case PLOT_PITCH:
      draw_series(painter, p, rec, first, count, &Record::pitch, c.green, &local.line_green);
      break;
    default:
      draw_series(painter, p, rec, first, count, &Record::heave, c.green, &local.line_green);
      break;
    }
    painter.setRenderHint(QPainter::Antialiasing, false);
    if (value == nullptr)
      continue;

    /* the points */
    const bool position = p.type == PLOT_LONGITUDE || p.type == PLOT_LATITUDE;
    const double Record::*org = p.type == PLOT_TINT        ? &Record::tint_org
                                : p.type == PLOT_LONGITUDE ? &Record::lon_org
                                : p.type == PLOT_LATITUDE  ? &Record::lat_org
                                : p.type == PLOT_SPEED     ? &Record::speed_org
                                : p.type == PLOT_HEADING   ? &Record::heading_org
                                                           : &Record::draft_org;
    Squares flagged(many), selected(many), changed(many), plain(many);
    for (int i = first; i < first + count; i++) {
      const Record &r = rec[i];
      const int x = r.pixel[2 * p.type], y = r.pixel[2 * p.type + 1];
      if (position && r.lonlat_flag) {
        flagged.add(x, y);
        local.rect_orange++;
      }
      if (r.isSelected(p.type)) {
        selected.add(x, y);
        local.rect_red++;
      }
      else if (position && r.lonlat_flag) {
        continue;
      }
      else if (r.*value != r.*org) {
        changed.add(x, y);
        local.rect_purple++;
      }
      else {
        plain.add(x, y);
        local.fill_black++;
      }
    }
    flagged.outline(painter, c.orange);
    plain.fill(painter, c.text);
    changed.outline(painter, c.purple);
    selected.outline(painter, c.red);
  }
  if (counts != nullptr)
    *counts = local;
}

/*--------------------------------------------------------------------*/
Canvas::Canvas(Session &session, QWidget *parent) : QWidget(parent), session_(session) {
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setMinimumSize(240, MIN_PLOT_HEIGHT);
  updateCursor();
}

void Canvas::setMode(Mode mode) {
  mode_ = mode;
  setIntervalMode(false);
  updateCursor();
}

void Canvas::setIntervalMode(bool on) {
  if (interval_mode_ && !on)
    session_.setInterval(0, 0, 3);
  interval_mode_ = on;
  updateCursor();
  update();
}

void Canvas::updateCursor() {
  if (interval_mode_) {
    setCursor(Qt::SplitHCursor);
    return;
  }
  switch (mode_) {
  case MODE_PICK:
    setCursor(Qt::CrossCursor);
    break;
  case MODE_SELECT:
    setCursor(Qt::PointingHandCursor);
    break;
  case MODE_DESELECT:
    setCursor(Qt::ForbiddenCursor);
    break;
  default:
    setCursor(Qt::ArrowCursor);
    break;
  }
}

void Canvas::layoutChanged() {
  setMinimumHeight(std::max(session_.numberPlots(), 1) * MIN_PLOT_HEIGHT);
  layout_plots(session_, size(), font());
  redraw();
}

void Canvas::redraw() {
  dirty_ = true;
  update();
}

void Canvas::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  layoutChanged();
}

void Canvas::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange)
    redraw();
  QWidget::changeEvent(event);
}

void Canvas::paintEvent(QPaintEvent *) {
  const qreal ratio = devicePixelRatioF();
  const QSize pixels = size() * ratio;
  if (dirty_ || image_.size() != pixels) {
    image_ = QImage(pixels, QImage::Format_RGB32);
    image_.setDevicePixelRatio(ratio);
    QPainter painter(&image_);
    painter.setFont(font());
    draw_plots(painter, session_, size(), palette());
    dirty_ = false;
  }
  QPainter painter(this);
  painter.drawImage(0, 0, image_);

  /* the edges of a Pick Zoom */
  if (interval_mode_ && !session_.plots().empty()) {
    painter.setPen(QPen(colors_for(palette()).red, 1.0, Qt::DashLine));
    for (int which = 0; which < 2; which++) {
      if (!session_.intervalSet(which))
        continue;
      const int x = session_.intervalBound(which);
      for (const Plot &p : session_.plots())
        painter.drawLine(x, p.iymin, x, p.iymax);
    }
  }
}

void Canvas::applyAt(const QPoint &position) {
  switch (mode_) {
  case MODE_PICK:
    session_.mousePick(position.x(), position.y());
    break;
  case MODE_SELECT:
    session_.mouseSelect(position.x(), position.y());
    break;
  case MODE_DESELECT:
    session_.mouseDeselect(position.x(), position.y());
    break;
  case MODE_SELECTALL:
    session_.mouseSelectAll(position.x(), position.y());
    break;
  case MODE_DESELECTALL:
    session_.mouseDeselectAll();
    break;
  }
  redraw();
  emit selectionChanged();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
  const QPoint position = event->position().toPoint();
  if (event->button() == Qt::LeftButton) {
    if (interval_mode_) {
      /* on a Mac with no middle button the second left click sets the other edge */
      const int which = session_.intervalSet(0) && !session_.intervalSet(1) ? 1 : 0;
      session_.setInterval(position.x(), position.y(), which);
      update();
      return;
    }
    applyAt(position);
    dragging_ = mode_ != MODE_PICK;
    last_position_ = position;
  }
  else if (event->button() == Qt::MiddleButton) {
    if (interval_mode_) {
      session_.setInterval(position.x(), position.y(), 1);
      update();
      return;
    }
    session_.step(-session_.data_step_size);
    redraw();
    emit viewChanged();
  }
  else if (event->button() == Qt::RightButton) {
    if (interval_mode_) {
      session_.setInterval(0, 0, 2);
      setIntervalMode(false);
      redraw();
      emit viewChanged();
      return;
    }
    session_.step(session_.data_step_size);
    redraw();
    emit viewChanged();
  }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
  if (!dragging_ || !(event->buttons() & Qt::LeftButton))
    return;
  const QPoint position = event->position().toPoint();
  const QPoint delta = position - last_position_;
  /* a fast drag leaves no gaps */
  const int steps = std::max(1, std::max(std::abs(delta.x()), std::abs(delta.y())) / (SELECT_DISTANCE / 2));
  for (int i = 1; i <= steps; i++)
    applyAt(last_position_ + delta * i / steps);
  last_position_ = position;
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton)
    dragging_ = false;
}

void Canvas::keyPressEvent(QKeyEvent *event) {
  if (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier)) {
    QWidget::keyPressEvent(event);
    return;
  }
  Mode mode;
  switch (event->key()) {
  case Qt::Key_Y:
  case Qt::Key_Q:
    mode = MODE_PICK;
    break;
  case Qt::Key_U:
  case Qt::Key_W:
    mode = MODE_SELECT;
    break;
  case Qt::Key_I:
  case Qt::Key_E:
    mode = MODE_DESELECT;
    break;
  case Qt::Key_O:
  case Qt::Key_R:
    mode = MODE_SELECTALL;
    break;
  case Qt::Key_P:
  case Qt::Key_T:
    mode = MODE_DESELECTALL;
    break;
  default:
    QWidget::keyPressEvent(event);
    return;
  }
  setMode(mode);
  emit modeChanged(mode);
}

}  // namespace mbne
