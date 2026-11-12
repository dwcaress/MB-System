/*--------------------------------------------------------------------
 *    The MB-system:  wcd_view_gl.h  12/11/2026
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

#ifndef MBWCDVIEW_WCD_VIEW_GL_H
#define MBWCDVIEW_WCD_VIEW_GL_H

#include <memory>

#include <QOpenGLWidget>

#include "WcdView.h"
#include "wcd_plot.h"

namespace wcd {

/* the swath drawn with OpenGL, and the axes with QPainter in the same widget */
class GlSwathView : public QOpenGLWidget, public SwathDrawing {
  Q_OBJECT

 public:
  explicit GlSwathView(WcdPlot &plot, QWidget *parent = nullptr);
  ~GlSwathView() override;
  void redraw() override { update(); }

 signals:
  void clicked(double across, double depth);

 protected:
  void initializeGL() override;
  void paintGL() override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  void releaseGL();
  WcdPlot &plot_;
  std::unique_ptr<wcd::SwathGL> gl_;
};

}  // namespace wcd

#endif
