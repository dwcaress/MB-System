/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_views.h  10/11/2026
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
 * The plots of mbnavadjust-qt as widgets. Each shows one plot of a PlotSet and
 * sends the mouse to the Session. A host can place them separately.
 */

#ifndef MBNAQ_VIEWS_H_
#define MBNAQ_VIEWS_H_

#include <QWidget>

class QMouseEvent;

namespace mbnaq {

class PlotSet;

class PlotView : public QWidget {
 public:
  explicit PlotView(PlotSet &plots, QWidget *parent = nullptr);

 protected:
  /* 1 left, 2 middle, 3 right; Alt with the left button counts as middle */
  static int buttonNumber(const QMouseEvent *event);

  PlotSet &plots_;
};

/* The contours of the two sections. Left drags the section, middle zooms, right picks tie points. */
class ContourView : public PlotView {
 public:
  explicit ContourView(PlotSet &plots, QWidget *parent = nullptr);
  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
};

/* The misfit in the horizontal offset. The left button sets the offset. */
class MisfitView : public PlotView {
 public:
  explicit MisfitView(PlotSet &plots, QWidget *parent = nullptr);
  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
};

/* The misfit in the vertical offset. The left button sets the offset. */
class ZOffsetView : public PlotView {
 public:
  explicit ZOffsetView(PlotSet &plots, QWidget *parent = nullptr);
  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
};

/* The inversion results. Left picks a tie, middle picks an untied crossing, right zooms. */
class ModelPlotView : public PlotView {
 public:
  explicit ModelPlotView(PlotSet &plots, QWidget *parent = nullptr);
  QSize sizeHint() const override;

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
};

}  // namespace mbnaq

#endif
