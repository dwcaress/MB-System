/*--------------------------------------------------------------------
 *    The MB-system:  WcdView.cc  1/11/2026
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

#include "WcdView.h"

#include <QVBoxLayout>

#ifdef MBWCD_HAVE_RHI
#include "wcd_view_rhi.h"
#endif
#ifdef MBWCD_HAVE_GL
#include "wcd_view_gl.h"
#endif

#if !defined(MBWCD_HAVE_RHI) && !defined(MBWCD_HAVE_GL)
#error "mbwcdview needs QRhi (MBWCD_HAVE_RHI) or OpenGL (MBWCD_HAVE_GL) to draw the swath"
#endif

namespace wcd {

WcdView::WcdView(QWidget *parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
#ifdef MBWCD_HAVE_RHI
  if (use_rhi()) {
    auto *view = new RhiSwathView(plot_);
    connect(view, &RhiSwathView::clicked, this, &WcdView::clicked);
    drawing_widget_ = view;
    drawing_ = view;
    rhi_ = true;
  }
#endif
#ifdef MBWCD_HAVE_GL
  if (drawing_ == nullptr) {
    auto *view = new GlSwathView(plot_);
    connect(view, &GlSwathView::clicked, this, &WcdView::clicked);
    drawing_widget_ = view;
    drawing_ = view;
  }
#endif
  layout->addWidget(drawing_widget_);
}

/* the drawing widget uses the plot: delete it first */
WcdView::~WcdView() { delete drawing_widget_; }

void WcdView::redraw() { drawing_->redraw(); }

}  // namespace wcd
