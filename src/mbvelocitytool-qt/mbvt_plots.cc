/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_plots.cc  2/11/2026
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

#include "mbvt_plots.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <QContextMenuEvent>
#include <QEvent>
#include <QFileInfo>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QPolygonF>
#include <QPointF>
#include <QStringList>

namespace mbvt {

namespace {

constexpr double NODE_HIT_PIXELS = 10.0;   // the mouse takes a node this close
constexpr double NODE_DEPTH_GAP = 0.01;    // a dragged node stays this far from its neighbours, m
constexpr double NODE_SIZE = 5.0;
constexpr double LINE_HIT_PIXELS = 6.0;    // a right click takes a display profile this close

/* a step of 1, 2 or 5 x 10^n with ticks at least min_pixels apart */
double nice_step(double range, double pixels, double min_pixels, bool whole) {
  if (!(range > 0.0) || !(pixels > 0.0))
    return 1.0;
  const double raw = range * min_pixels / pixels;
  const double magnitude = std::pow(10.0, std::floor(std::log10(raw)));
  const double ratio = raw / magnitude;
  const double step = magnitude * (ratio <= 1.0 ? 1.0 : ratio <= 2.0 ? 2.0 : ratio <= 5.0 ? 5.0 : 10.0);
  return whole ? std::max(step, 1.0) : step;
}

double segment_distance(const QPointF &p, const QPointF &a, const QPointF &b) {
  const QPointF ab = b - a;
  const double length2 = QPointF::dotProduct(ab, ab);
  const double t = length2 > 0.0 ? std::clamp(QPointF::dotProduct(p - a, ab) / length2, 0.0, 1.0) : 0.0;
  const QPointF d = p - (a + t * ab);
  return std::sqrt(QPointF::dotProduct(d, d));
}

int step_decimals(double step) { return step >= 1.0 ? 0 : (int)std::ceil(-std::log10(step) - 1.0e-9); }

/* a line through many points closer together than a pixel draws the same with fewer of them */
QPolygonF thin_line(const Axes &axes, const std::vector<double> &x, const std::vector<double> &y) {
  QPolygonF line;
  line.reserve((int)x.size());
  for (size_t i = 0; i < x.size(); i++) {
    const QPointF p(axes.px(x[i]), axes.py(y[i]));
    if (line.isEmpty() || i + 1 == x.size() || std::abs(p.x() - line.last().x()) >= 1.0 || std::abs(p.y() - line.last().y()) >= 1.0)
      line.append(p);
  }
  return line;
}

}  // namespace

/*--------------------------------------------------------------------*/
PlotBase::PlotBase(QWidget *parent) : QWidget(parent) {
  setMinimumSize(240, 160);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

QRectF PlotBase::plotArea(int title_lines) const {
  const QFontMetrics fm(font());
  const double left = fm.horizontalAdvance("-00000.0") + fm.height() + 10.0;
  const double top = (title_lines + 0.5) * fm.height();
  const double bottom = 2.0 * fm.height() + 8.0;
  return QRectF(left, top, std::max(width() - left - 24.0, 10.0), std::max(height() - top - bottom, 10.0));
}

QColor PlotBase::foreground() const { return palette().color(QPalette::Text); }

QColor PlotBase::displayColor(int index) const {
  const bool dark = palette().color(QPalette::Base).lightness() < 128;
  static const QColor light_colors[] = {QColor(220, 0, 0),    QColor(0, 150, 0),   QColor(0, 0, 230),
                                        QColor(225, 120, 0), QColor(150, 0, 170), QColor(0, 145, 150)};
  static const QColor dark_colors[] = {QColor(255, 110, 110), QColor(100, 215, 100), QColor(120, 170, 255),
                                       QColor(255, 175, 80),  QColor(220, 130, 240), QColor(80, 215, 215)};
  return (dark ? dark_colors : light_colors)[index % 6];
}

void PlotBase::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange || event->type() == QEvent::FontChange)
    update();
  QWidget::changeEvent(event);
}

void PlotBase::drawAxes(QPainter &painter, const Axes &axes, const QString &title, const QString &subtitle, const QString &xlabel,
                        const QString &ylabel, bool whole_x) const {
  const QFontMetrics fm(font());
  const QRectF &area = axes.area;
  QColor grid = foreground();
  grid.setAlpha(70);
  const QPen grid_pen(grid, 1.0, Qt::DashLine);
  const QPen text_pen(foreground());

  const double xstep = nice_step(std::abs(axes.x1 - axes.x0), area.width(), fm.horizontalAdvance("-000000") + 16.0, whole_x);
  const int xdecimals = step_decimals(xstep);
  for (long k = (long)std::ceil(axes.x0 / xstep - 1.0e-9); k <= (long)std::floor(axes.x1 / xstep + 1.0e-9); k++) {
    const double x = axes.px(k * xstep);
    painter.setPen(grid_pen);
    painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
    painter.setPen(text_pen);
    const QString label = QString::number(k * xstep, 'f', xdecimals);
    painter.drawText(QPointF(x - fm.horizontalAdvance(label) / 2.0, area.bottom() + fm.ascent() + 3.0), label);
  }
  const double ystep = nice_step(std::abs(axes.y1 - axes.y0), area.height(), 2.0 * fm.height(), false);
  const int ydecimals = step_decimals(ystep);
  for (long k = (long)std::ceil(axes.y0 / ystep - 1.0e-9); k <= (long)std::floor(axes.y1 / ystep + 1.0e-9); k++) {
    const double y = axes.py(k * ystep);
    painter.setPen(grid_pen);
    painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    painter.setPen(text_pen);
    const QString label = QString::number(k * ystep, 'f', ydecimals);
    painter.drawText(QPointF(area.left() - fm.horizontalAdvance(label) - 4.0, y + fm.ascent() / 2.5), label);
  }

  painter.setPen(text_pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(area);
  double y = fm.ascent() + 2.0;
  if (!subtitle.isEmpty()) {
    painter.drawText(QPointF(area.center().x() - fm.horizontalAdvance(subtitle) / 2.0, y), subtitle);
    y += fm.height();
  }
  QFont bold = painter.font();
  bold.setBold(true);
  painter.setFont(bold);
  painter.drawText(QPointF(area.center().x() - QFontMetrics(bold).horizontalAdvance(title) / 2.0, y), title);
  painter.setFont(font());
  painter.drawText(QPointF(area.center().x() - fm.horizontalAdvance(xlabel) / 2.0, area.bottom() + 2.0 * fm.height() + 2.0), xlabel);
  painter.save();
  painter.translate(fm.ascent() + 1.0, area.center().y() + fm.horizontalAdvance(ylabel) / 2.0);
  painter.rotate(-90.0);
  painter.drawText(QPointF(0.0, 0.0), ylabel);
  painter.restore();
}

/*--------------------------------------------------------------------*/
VelocityPlot::VelocityPlot(QWidget *parent) : PlotBase(parent) {
  setMouseTracking(true);
  setToolTip("Drag a node of the editable profile to move it.\n"
             "Double-click or middle-click to add a node.\n"
             "Right-click a node to delete it, or a display profile to edit, hide or remove it.");
}

void VelocityPlot::setProfiles(Profile *edit, const std::vector<Profile> *display) {
  edit_ = edit;
  display_ = display;
  active_ = hover_ = -1;
  update();
}

void VelocityPlot::setScale(double maxdepth, double velrange, double velcenter) {
  maxdepth_ = maxdepth;
  velrange_ = velrange;
  velcenter_ = velcenter;
  update();
}

Axes VelocityPlot::axes() const {
  Axes a;
  a.area = plotArea();
  a.x0 = velcenter_ - velrange_ / 2.0;
  a.x1 = velcenter_ + velrange_ / 2.0;
  a.y0 = 0.0;
  a.y1 = maxdepth_;
  return a;
}

void VelocityPlot::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  const Axes a = axes();
  drawAxes(painter, a, "Water Sound Velocity Profiles", QString(), "Water Sound Velocity (m/s)", "Depth (m)");

  painter.setClipRect(a.area);
  if (display_ != nullptr)
    for (size_t i = 0; i < display_->size(); i++) {
      const Profile &p = (*display_)[i];
      if (p.hidden)
        continue;
      painter.setPen(QPen(displayColor((int)i), (int)i == menu_profile_ ? 3.0 : 1.0));
      painter.drawPolyline(thin_line(a, p.velocity, p.depth));
    }
  if (edit_ != nullptr) {
    painter.setPen(QPen(foreground(), 1.5));
    painter.drawPolyline(thin_line(a, edit_->velocity, edit_->depth));
    for (int i = 0; i < edit_->size(); i++) {
      const QPointF p(a.px(edit_->velocity[i]), a.py(edit_->depth[i]));
      const bool lit = i == active_ || i == hover_;
      const double size = lit ? NODE_SIZE + 3.0 : NODE_SIZE;
      painter.fillRect(QRectF(p.x() - size / 2.0, p.y() - size / 2.0, size, size),
                       lit ? palette().color(QPalette::Highlight) : foreground());
    }
  }
  drawLegend(painter, a);
}

void VelocityPlot::drawLegend(QPainter &painter, const Axes &axes) const {
  if (display_ == nullptr || display_->empty())
    return;
  std::vector<int> shown;
  for (int i = 0; i < (int)display_->size(); i++)
    if (!(*display_)[i].hidden)
      shown.push_back(i);
  if (shown.empty())
    return;

  const QFontMetrics fm(font());
  const double margin = 6.0, swatch = 16.0;
  const int rows = std::min((int)shown.size(), std::max((int)((axes.area.height() - 2.0 * margin) / fm.height()), 1));
  const bool all_shown = rows == (int)shown.size();
  const int max_text = std::max((int)(0.6 * axes.area.width() - swatch - 2.0 * margin), 40);

  QStringList labels;
  int text_width = 0;
  for (int i = 0; i < rows; i++) {
    const Profile &p = (*display_)[shown[i]];
    const QString name = QString::fromStdString(p.name);
    const bool is_file = p.source == ProfileSource::File || p.source == ProfileSource::SwathListing || p.source == ProfileSource::Edited;
    QString label = is_file ? QFileInfo(name).fileName() : name;
    if (i == rows - 1 && !all_shown)
      label = QString("and %1 more").arg((int)shown.size() - rows + 1);
    label = fm.elidedText(label, Qt::ElideMiddle, max_text);
    text_width = std::max(text_width, fm.horizontalAdvance(label));
    labels << label;
  }

  const double width = swatch + text_width + 2.0 * margin;
  /* lower left: sound speed rises with depth, so the profiles leave this corner free */
  const double height = rows * fm.height() + margin;
  const QRectF box(axes.area.left() + margin, axes.area.bottom() - height - margin, width, height);
  QColor background = palette().color(QPalette::Base);
  background.setAlpha(120);
  painter.fillRect(box, background);
  for (int i = 0; i < rows; i++) {
    const double y = box.top() + margin / 2.0 + i * fm.height();
    if (all_shown || i < rows - 1) {
      painter.setPen(QPen(displayColor(shown[i]), 2.0));
      painter.drawLine(QPointF(box.left() + margin / 2.0, y + fm.height() / 2.0),
                       QPointF(box.left() + swatch, y + fm.height() / 2.0));
    }
    painter.setPen(foreground());
    painter.drawText(QPointF(box.left() + swatch + margin, y + fm.ascent()), labels[i]);
  }
}

int VelocityPlot::nodeAt(const QPointF &pos) const {
  if (edit_ == nullptr)
    return -1;
  const Axes a = axes();
  int node = -1;
  double nearest = NODE_HIT_PIXELS * NODE_HIT_PIXELS;
  for (int i = 0; i < edit_->size(); i++) {
    const double dx = a.px(edit_->velocity[i]) - pos.x(), dy = a.py(edit_->depth[i]) - pos.y();
    const double distance = dx * dx + dy * dy;
    if (distance < nearest) {
      nearest = distance;
      node = i;
    }
  }
  return node;
}

std::vector<int> VelocityPlot::profilesAt(const QPointF &pos) const {
  std::vector<std::pair<double, int>> found;
  const Axes a = axes();
  if (display_ == nullptr || !a.area.contains(pos))
    return {};
  for (int i = 0; i < (int)display_->size(); i++) {
    const Profile &p = (*display_)[i];
    if (p.hidden)
      continue;
    double nearest = LINE_HIT_PIXELS;
    bool hit = false;
    for (int j = 1; j < p.size(); j++) {
      const double distance = segment_distance(pos, QPointF(a.px(p.velocity[j - 1]), a.py(p.depth[j - 1])),
                                                QPointF(a.px(p.velocity[j]), a.py(p.depth[j])));
      if (distance <= nearest) {
        nearest = distance;
        hit = true;
      }
    }
    if (hit)
      found.push_back({nearest, i});
  }
  std::sort(found.begin(), found.end());
  std::vector<int> profiles;
  for (const auto &entry : found)
    profiles.push_back(entry.second);
  return profiles;
}

void VelocityPlot::dragNode(const QPointF &pos) {
  const Axes a = axes();
  const int last = edit_->size() - 1;
  edit_->velocity[active_] = a.vx(std::clamp(pos.x(), a.area.left(), a.area.right()));
  /* the first node keeps its depth, the others stay between their neighbours */
  if (active_ > 0) {
    const double low = std::max(edit_->depth[active_ - 1] + NODE_DEPTH_GAP, a.y0);
    const double high = active_ < last ? std::min(edit_->depth[active_ + 1] - NODE_DEPTH_GAP, a.y1) : a.y1;
    if (low <= high)
      edit_->depth[active_] = std::clamp(a.vy(pos.y()), low, high);
  }
  update();
  emit edited();
}

bool VelocityPlot::addNodeAt(const QPointF &pos) {
  const Axes a = axes();
  if (edit_ == nullptr || !a.area.contains(pos))
    return false;
  const double depth = a.vy(pos.y());
  int place = 0;
  while (place < edit_->size() && edit_->depth[place] <= depth)
    place++;
  /* never two nodes at one depth */
  if ((place > 0 && depth - edit_->depth[place - 1] < NODE_DEPTH_GAP) ||
      (place < edit_->size() && edit_->depth[place] - depth < NODE_DEPTH_GAP))
    return false;
  edit_->depth.insert(edit_->depth.begin() + place, depth);
  edit_->velocity.insert(edit_->velocity.begin() + place, a.vx(pos.x()));
  active_ = hover_ = -1;
  update();
  emit edited();
  return true;
}

bool VelocityPlot::deleteNode(int node) {
  if (edit_ == nullptr || node < 0 || node >= edit_->size() || edit_->size() <= 2)
    return false;
  edit_->depth.erase(edit_->depth.begin() + node);
  edit_->velocity.erase(edit_->velocity.begin() + node);
  active_ = hover_ = -1;
  update();
  emit edited();
  return true;
}

void VelocityPlot::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    active_ = nodeAt(event->position());
    if (active_ >= 0)
      setCursor(Qt::ClosedHandCursor);
    update();
  }
  else if (event->button() == Qt::MiddleButton) {
    addNodeAt(event->position());
  }
}

void VelocityPlot::mouseMoveEvent(QMouseEvent *event) {
  const Axes a = axes();
  const QPointF pos = event->position();
  if (active_ >= 0 && (event->buttons() & Qt::LeftButton) != 0) {
    dragNode(pos);
    emit pointer(true, edit_->velocity[active_], edit_->depth[active_]);
    return;
  }
  const int node = nodeAt(pos);
  if (node != hover_) {
    hover_ = node;
    if (node >= 0)
      setCursor(Qt::OpenHandCursor);
    else
      unsetCursor();
    update();
  }
  emit pointer(a.area.contains(pos), a.vx(pos.x()), a.vy(pos.y()));
}

void VelocityPlot::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton && active_ >= 0) {
    active_ = -1;
    if (hover_ >= 0)
      setCursor(Qt::OpenHandCursor);
    else
      unsetCursor();
    update();
  }
}

void VelocityPlot::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton && nodeAt(event->position()) < 0)
    addNodeAt(event->position());
}

void VelocityPlot::addProfileActions(QMenu *menu, int index) {
  const QString name = QString::fromStdString((*display_)[index].name);
  const auto add = [this, menu, index, name](const QString &text, ProfileAction action) {
    connect(menu->addAction(text), &QAction::triggered, this, [this, index, name, action] { emit profileAction(index, name, action); });
  };
  add("Activate as Editing Profile", ProfileAction::Activate);
  add("Hide", ProfileAction::Hide);
  add("Remove", ProfileAction::Remove);
  add("Save Profile As...", ProfileAction::Save);
}

void VelocityPlot::contextMenuEvent(QContextMenuEvent *event) {
  const QPointF pos = event->pos();
  if (!axes().area.contains(pos))
    return;
  QMenu menu(this);

  /* profiles often lie on top of each other, so each one close to the click is offered by name */
  const std::vector<int> profiles = profilesAt(pos);
  if (profiles.size() == 1) {
    const QString name = QFontMetrics(menu.font()).elidedText(QString::fromStdString((*display_)[profiles[0]].name), Qt::ElideMiddle, 320);
    menu.addAction(name)->setEnabled(false);
    addProfileActions(&menu, profiles[0]);
    menu_profile_ = profiles[0];
  }
  else {
    for (const int index : profiles) {
      const QString name = QFontMetrics(menu.font()).elidedText(QString::fromStdString((*display_)[index].name), Qt::ElideMiddle, 320);
      QMenu *submenu = menu.addMenu(name);
      addProfileActions(submenu, index);
      connect(submenu, &QMenu::aboutToShow, this, [this, index] {
        menu_profile_ = index;
        update();
      });
    }
  }

  if (edit_ != nullptr) {
    if (!menu.isEmpty())
      menu.addSeparator();
    const int node = nodeAt(pos);
    if (node >= 0) {
      QAction *remove = menu.addAction("Delete Node");
      remove->setEnabled(edit_->size() > 2);
      connect(remove, &QAction::triggered, this, [this, node] { deleteNode(node); });
    }
    else {
      connect(menu.addAction("Add Node Here"), &QAction::triggered, this, [this, pos] { addNodeAt(pos); });
    }
  }

  if (display_ != nullptr && std::any_of(display_->begin(), display_->end(), [](const Profile &p) { return p.hidden; })) {
    if (!menu.isEmpty())
      menu.addSeparator();
    connect(menu.addAction("Show Hidden Profiles"), &QAction::triggered, this, [this] { emit showHiddenProfiles(); });
  }

  if (menu.isEmpty())
    return;
  update();
  menu.exec(event->globalPos());
  menu_profile_ = -1;
  update();
}

void VelocityPlot::leaveEvent(QEvent *event) {
  emit pointer(false, 0.0, 0.0);
  if (hover_ >= 0) {
    hover_ = -1;
    unsetCursor();
    update();
  }
  PlotBase::leaveEvent(event);
}

/*--------------------------------------------------------------------*/
ResidualPlot::ResidualPlot(QWidget *parent) : PlotBase(parent) {}

void ResidualPlot::setData(const Residuals *residuals, int nbeams) {
  residuals_ = residuals;
  nbeams_ = nbeams;
  update();
}

void ResidualPlot::setRange(double resrange) {
  resrange_ = resrange;
  update();
}

void ResidualPlot::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  const bool traced = residuals_ != nullptr && residuals_->beam_last >= residuals_->beam_first;

  Axes a;
  a.area = plotArea(2);
  a.x0 = (traced ? residuals_->beam_first : 0) - 1.0;
  a.x1 = (traced ? residuals_->beam_last : nbeams_) + 1.0;
  /* shallow is up and deep is down */
  a.y0 = -resrange_;
  a.y1 = resrange_;
  QString subtitle;
  if (traced)
    subtitle = QString("Depth Range: %1 to %2 m")
                   .arg(residuals_->bath_min, 0, 'f', 0)
                   .arg(residuals_->bath_max, 0, 'f', 0);
  drawAxes(painter, a, "Swath Bathymetry Beam Residuals", subtitle, "Bathymetry Beam Number", "Residual (m)", true);
  if (!traced)
    return;

  painter.setClipRect(a.area);
  /* the error bars are pale, so that the mean stands out when the beams are closer than the markers */
  QColor bar = foreground();
  bar.setAlpha(90);
  painter.setPen(QPen(bar, 1.0));
  for (int i = 0; i < residuals_->nbeams; i++)
    if (residuals_->count[i] > 0)
      painter.drawLine(QPointF(a.px(i), a.py(residuals_->residual[i] - residuals_->sigma[i])),
                       QPointF(a.px(i), a.py(residuals_->residual[i] + residuals_->sigma[i])));

  painter.setPen(QPen(foreground(), 1.5));
  const double size = std::clamp(0.8 * a.area.width() / (a.x1 - a.x0), 1.5, NODE_SIZE);
  QPolygonF line;
  for (int i = 0; i < residuals_->nbeams; i++) {
    if (residuals_->count[i] <= 0) {
      painter.drawPolyline(line);
      line.clear();
      continue;
    }
    const QPointF p(a.px(i), a.py(residuals_->residual[i]));
    painter.fillRect(QRectF(p.x() - size / 2.0, p.y() - size / 2.0, size, size), foreground());
    line.append(p);
  }
  painter.drawPolyline(line);
}

/*--------------------------------------------------------------------*/
RaypathPlot::RaypathPlot(QWidget *parent) : PlotBase(parent) {}

void RaypathPlot::setData(const Residuals *residuals) {
  residuals_ = residuals;
  update();
}

void RaypathPlot::setMaxDepth(double maxdepth) {
  maxdepth_ = maxdepth;
  update();
}

void RaypathPlot::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  const bool traced = residuals_ != nullptr && !residuals_->raypaths.empty() && residuals_->ray_depth_max > residuals_->ray_depth_min;
  const double depth_min = traced ? residuals_->ray_depth_min : 0.0;
  const double depth_max = traced ? residuals_->ray_depth_max : maxdepth_;

  /* no vertical exaggeration: the depth range fills the plot unless the rays are then too wide for it */
  Axes a;
  a.area = plotArea();
  a.y0 = depth_min - 0.02 * (depth_max - depth_min);
  a.y1 = depth_max + 0.02 * (depth_max - depth_min);
  double scale = a.area.height() / (a.y1 - a.y0);
  double xmax = a.area.width() / (2.0 * scale);
  if (traced && xmax < residuals_->ray_x_max) {
    xmax = 1.02 * residuals_->ray_x_max;
    scale = a.area.width() / (2.0 * xmax);
    a.y1 = a.y0 + a.area.height() / scale;
  }
  a.x0 = -xmax;
  a.x1 = xmax;
  drawAxes(painter, a, "Raypaths", QString(), "Acrosstrack Distance (m)", "Depth (m)");
  if (!traced)
    return;

  painter.setClipRect(a.area);
  painter.setPen(QPen(foreground(), 0.0));
  for (const RayPath &ray : residuals_->raypaths)
    if (ray.x.size() > 1)
      painter.drawPolyline(thin_line(a, ray.x, ray.z));
  const QColor end = displayColor(0);
  for (const RayPath &ray : residuals_->raypaths)
    if (ray.x.size() > 1)
      painter.fillRect(QRectF(a.px(ray.x.back()) - 2.0, a.py(ray.z.back()) - 2.0, 4.0, 4.0), end);
}

}  // namespace mbvt
