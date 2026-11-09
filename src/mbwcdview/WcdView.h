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

#include <memory>

#include <QOpenGLWidget>

#include "wcd_plot.h"

namespace wcd {

/* the water column swath plot in the window */
class WcdView : public QOpenGLWidget {
  Q_OBJECT

 public:
  explicit WcdView(QWidget *parent = nullptr);
  ~WcdView() override;
  wcd::WcdPlot &plot() { return plot_; }

 signals:
  void clicked(double across, double depth);

 protected:
  void initializeGL() override;
  void paintGL() override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  wcd::WcdPlot plot_;
  std::unique_ptr<wcd::SwathGL> gl_;
};

}  // namespace wcd

#endif
