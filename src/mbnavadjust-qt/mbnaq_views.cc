/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_views.cc  10/11/2026
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

#include "mbnaq_views.h"

#include <QMouseEvent>
#include <QPainter>

#include "mbnaq_core.h"
#include "mbnaq_plots.h"

namespace mbnaq {

PlotView::PlotView(PlotSet &plots, QWidget *parent) : QWidget(parent), plots_(plots) {
  setCursor(Qt::CrossCursor);
  setAttribute(Qt::WA_OpaquePaintEvent);
  connect(&plots_, &PlotSet::changed, this, QOverload<>::of(&QWidget::update));
}

int PlotView::buttonNumber(const QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton ||
      (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier)))
    return 2;
  if (event->button() == Qt::RightButton)
    return 3;
  return event->button() == Qt::LeftButton ? 1 : 0;
}

/*--------------------------------------------------------------------*/

ContourView::ContourView(PlotSet &plots, QWidget *parent) : PlotView(plots, parent) { setMinimumSize(200, 200); }

QSize ContourView::sizeHint() const { return QSize(601, 601); }

void ContourView::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  plots_.contour().paint(painter, plots_.session());
}

void ContourView::resizeEvent(QResizeEvent *) { plots_.setContourSize(width(), height()); }

void ContourView::mousePressEvent(QMouseEvent *event) {
  plots_.session().contourPress(buttonNumber(event), event->position().toPoint().x(), event->position().toPoint().y());
}

void ContourView::mouseMoveEvent(QMouseEvent *event) {
  plots_.session().contourDrag(event->position().toPoint().x(), event->position().toPoint().y());
}

void ContourView::mouseReleaseEvent(QMouseEvent *event) {
  plots_.session().contourRelease(buttonNumber(event), event->position().toPoint().x(), event->position().toPoint().y());
  /* the layer was moved by whole pixels while dragging */
  plots_.contour().markStale();
  update();
}

/*--------------------------------------------------------------------*/

MisfitView::MisfitView(PlotSet &plots, QWidget *parent) : PlotView(plots, parent) { setMinimumSize(100, 100); }

QSize MisfitView::sizeHint() const { return QSize(302, 302); }

void MisfitView::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.drawImage(0, 0, plots_.misfitImage());
}

void MisfitView::resizeEvent(QResizeEvent *) { plots_.setMisfitSize(width(), height()); }

void MisfitView::mousePressEvent(QMouseEvent *event) {
  if (buttonNumber(event) == 1)
    plots_.session().misfitPress(event->position().toPoint().x(), event->position().toPoint().y());
}

void MisfitView::mouseMoveEvent(QMouseEvent *event) {
  plots_.session().misfitDrag(event->position().toPoint().x(), event->position().toPoint().y());
}

void MisfitView::mouseReleaseEvent(QMouseEvent *event) {
  if (buttonNumber(event) == 1) {
    plots_.session().misfitRelease();
    plots_.contour().markStale();
    update();
  }
}

/*--------------------------------------------------------------------*/

ZOffsetView::ZOffsetView(PlotSet &plots, QWidget *parent) : PlotView(plots, parent) { setMinimumSize(100, 40); }

QSize ZOffsetView::sizeHint() const { return QSize(301, 61); }

void ZOffsetView::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.drawImage(0, 0, plots_.zOffsetImage());
}

void ZOffsetView::resizeEvent(QResizeEvent *) { plots_.setZOffsetSize(width(), height()); }

void ZOffsetView::mousePressEvent(QMouseEvent *event) {
  if (buttonNumber(event) == 1)
    plots_.session().zOffsetPress(event->position().toPoint().x(), event->position().toPoint().y());
}

void ZOffsetView::mouseMoveEvent(QMouseEvent *event) {
  plots_.session().zOffsetDrag(event->position().toPoint().x(), event->position().toPoint().y());
}

void ZOffsetView::mouseReleaseEvent(QMouseEvent *event) {
  if (buttonNumber(event) == 1)
    plots_.session().zOffsetRelease();
}

/*--------------------------------------------------------------------*/

ModelPlotView::ModelPlotView(PlotSet &plots, QWidget *parent) : PlotView(plots, parent) { setMinimumSize(300, 200); }

QSize ModelPlotView::sizeHint() const { return QSize(1000, 600); }

void ModelPlotView::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.drawImage(0, 0, plots_.modelImage());
}

void ModelPlotView::resizeEvent(QResizeEvent *) { plots_.setModelSize(width(), height()); }

void ModelPlotView::mousePressEvent(QMouseEvent *event) {
  plots_.session().modelPlotPress(buttonNumber(event), event->position().toPoint().x(), event->position().toPoint().y());
}

void ModelPlotView::mouseMoveEvent(QMouseEvent *event) {
  plots_.session().modelPlotDrag(event->position().toPoint().x(), event->position().toPoint().y());
}

void ModelPlotView::mouseReleaseEvent(QMouseEvent *event) {
  plots_.session().modelPlotRelease(buttonNumber(event), event->position().toPoint().x(), event->position().toPoint().y());
}

}  // namespace mbnaq
