/*--------------------------------------------------------------------
 *    The MB-system:  WcdView.h  1/11/2026
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

#ifndef MBWCDVIEW_WCDVIEW_H
#define MBWCDVIEW_WCDVIEW_H

#include <QWidget>

#include "wcd_plot.h"

namespace wcd {

/* the widget that draws the swath: with QRhi (wcd_view_rhi.h) or with OpenGL (wcd_view_gl.h) */
class SwathDrawing {
 public:
  virtual ~SwathDrawing() = default;
  virtual void redraw() = 0;
};

/* the water column swath plot in the window: the plot and the widget that draws it,
   with QRhi where it is built (unless MB_RENDERER=legacy-gl) and with OpenGL otherwise */
class WcdView : public QWidget {
  Q_OBJECT

 public:
  explicit WcdView(QWidget *parent = nullptr);
  ~WcdView() override;
  wcd::WcdPlot &plot() { return plot_; }
  /* draw again after the plot changed */
  void redraw();
  bool usesRhi() const { return rhi_; }

 signals:
  void clicked(double across, double depth);

 private:
  wcd::WcdPlot plot_;
  QWidget *drawing_widget_ = nullptr;
  SwathDrawing *drawing_ = nullptr;
  bool rhi_ = false;
};

}  // namespace wcd

#endif
