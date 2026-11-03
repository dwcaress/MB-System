/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_plots.h  2/11/2026
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
 * The three plots of mbvelocitytool-qt, drawn with QPainter and sized by the
 * window: sound velocity profiles (the edit profile is changed here with the
 * mouse), beam residuals and raypaths. The plots do not own the data: the main
 * window keeps it alive and calls update() when it changes.
 */

#ifndef MBVELOCITYTOOL_QT_MBVT_PLOTS_H
#define MBVELOCITYTOOL_QT_MBVT_PLOTS_H

#include <vector>

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QWidget>

#include "mbvt_core.h"

class QMenu;
class QPainter;

namespace mbvt {

/* value <-> pixel conversion of one plot */
struct Axes {
  QRectF area;
  double x0 = 0.0, x1 = 1.0, y0 = 0.0, y1 = 1.0;
  double px(double x) const { return area.left() + (x - x0) * area.width() / (x1 - x0); }
  double py(double y) const { return area.top() + (y - y0) * area.height() / (y1 - y0); }
  double vx(double pixel) const { return x0 + (pixel - area.left()) * (x1 - x0) / area.width(); }
  double vy(double pixel) const { return y0 + (pixel - area.top()) * (y1 - y0) / area.height(); }
};

class PlotBase : public QWidget {
  Q_OBJECT
 public:
  explicit PlotBase(QWidget *parent = nullptr);

 protected:
  QRectF plotArea(int title_lines = 1) const;
  void drawAxes(QPainter &painter, const Axes &axes, const QString &title, const QString &subtitle, const QString &xlabel,
                const QString &ylabel, bool whole_x = false) const;
  QColor foreground() const;
  QColor displayColor(int index) const;  // cycles through six colors in the order the profiles were loaded
  void changeEvent(QEvent *event) override;
};

enum class ProfileAction { Activate, Hide, Remove, Save };

class VelocityPlot : public PlotBase {
  Q_OBJECT
 public:
  explicit VelocityPlot(QWidget *parent = nullptr);
  void setProfiles(Profile *edit, const std::vector<Profile> *display);
  void setScale(double maxdepth, double velrange, double velcenter);
  Axes axes() const;
  int nodeAt(const QPointF &pos) const;  // nearest node of the edit profile close to pos, or -1
  bool addNodeAt(const QPointF &pos);
  bool deleteNode(int node);
  std::vector<int> profilesAt(const QPointF &pos) const;  // the visible display profiles close to pos, nearest first

 signals:
  void edited();
  void profileAction(int index, const QString &name, ProfileAction action);
  void showHiddenProfiles();
  void pointer(bool inside, double velocity, double depth);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void leaveEvent(QEvent *event) override;

 private:
  void dragNode(const QPointF &pos);
  void drawLegend(QPainter &painter, const Axes &axes) const;
  void addProfileActions(QMenu *menu, int index);

  Profile *edit_ = nullptr;
  const std::vector<Profile> *display_ = nullptr;
  double maxdepth_ = 3000.0, velrange_ = 500.0, velcenter_ = 1490.0;
  int active_ = -1;
  int hover_ = -1;
  int menu_profile_ = -1;  // the display profile the open menu is about
};

class ResidualPlot : public PlotBase {
  Q_OBJECT
 public:
  explicit ResidualPlot(QWidget *parent = nullptr);
  /* residuals may be null, nbeams is the number of beams of the file before the first Reprocess */
  void setData(const Residuals *residuals, int nbeams);
  void setRange(double resrange);

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  const Residuals *residuals_ = nullptr;
  int nbeams_ = 100;
  double resrange_ = 200.0;
};

class RaypathPlot : public PlotBase {
  Q_OBJECT
 public:
  explicit RaypathPlot(QWidget *parent = nullptr);
  void setData(const Residuals *residuals);
  void setMaxDepth(double maxdepth);  // the depth range while there are no raypaths

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  const Residuals *residuals_ = nullptr;
  double maxdepth_ = 3000.0;
};

}  // namespace mbvt

#endif
