/*--------------------------------------------------------------------
 *    The MB-system:  wcd_view_rhi.h  12/11/2026
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

#ifndef MBWCDVIEW_WCD_VIEW_RHI_H
#define MBWCDVIEW_WCD_VIEW_RHI_H

#include <memory>

#include <QRhiWidget>
#include <QSet>

#include "WcdView.h"
#include "wcd_plot.h"

namespace wcd {

class SwathRhi;

/* the swath drawn with QRhi (Metal on macOS, Direct3D 11 on Windows, OpenGL
   elsewhere), and the axes with QPainter in a transparent widget on top */
class RhiSwathView : public QRhiWidget, public SwathDrawing {
  Q_OBJECT

 public:
  explicit RhiSwathView(WcdPlot &plot, QWidget *parent = nullptr);
  ~RhiSwathView() override;
  void redraw() override;

 signals:
  void clicked(double across, double depth);

 protected:
  void initialize(QRhiCommandBuffer *cb) override;
  void render(QRhiCommandBuffer *cb) override;
  void releaseResources() override;
  void mousePressEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

 private:
  WcdPlot &plot_;
  QWidget *overlay_;
  QRhi *rhi_ = nullptr;  // the QRhi of swath_
  QSet<QRhi *> seen_;    // every QRhi this view met that is still alive
  std::unique_ptr<SwathRhi> swath_;
};

}  // namespace wcd

#endif
