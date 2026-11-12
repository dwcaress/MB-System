/*--------------------------------------------------------------------
 *    The MB-system:  wcd_view_rhi.cc  12/11/2026
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

#include "wcd_view_rhi.h"

#include <vector>

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

#include "wcd_swath_rhi.h"

namespace wcd {

namespace {

/* the axes, colour bar, soundings and bottom line, drawn with QPainter over the swath */
class Overlay : public QWidget {
 public:
  Overlay(const WcdPlot &plot, QWidget *parent) : QWidget(parent), plot_(plot) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
  }

 protected:
  void paintEvent(QPaintEvent *) override {
    QPainter painter(this);
    plot_.drawOverlay(painter, size());
  }

 private:
  const WcdPlot &plot_;
};

}  // namespace

RhiSwathView::RhiSwathView(WcdPlot &plot, QWidget *parent)
    : QRhiWidget(parent), plot_(plot), overlay_(new Overlay(plot, this)) {}

RhiSwathView::~RhiSwathView() {
  /* Up to Qt 6.12.0 (QTBUG-151834), a view moved to another window by floating
     or docking its dock leaves the cleanup callback of QRhiWidget in the QRhi of
     the window it left, which would then call this deleted widget when that
     window closes. Remove it from every earlier QRhi still alive, together with
     this view's own callbacks. */
  for (QRhi *r : std::as_const(seen_)) {
    r->removeCleanupCallback(&seen_);
    if (r != rhi())
      r->removeCleanupCallback(static_cast<QRhiWidget *>(this));
  }
  swath_.reset();
}

void RhiSwathView::redraw() {
  update();
  overlay_->update();
}

void RhiSwathView::releaseResources() { swath_.reset(); }

void RhiSwathView::initialize(QRhiCommandBuffer *) {
  /* called for the first frame, after a resize, and in another window: the
     resources of a QRhi left behind went with it or are dropped here */
  if (rhi() != rhi_) {
    swath_.reset();
    rhi_ = rhi();
    if (rhi_ != nullptr && !seen_.contains(rhi_)) {
      seen_.insert(rhi_);
      rhi_->addCleanupCallback(&seen_, [this](QRhi *dying) {
        seen_.remove(dying);
        if (dying == rhi_) {
          swath_.reset();
          rhi_ = nullptr;
        }
      });
    }
  }
  if (swath_ == nullptr && rhi_ != nullptr) {
    swath_ = std::make_unique<SwathRhi>(rhi_);
    if (!swath_->init(renderTarget()->renderPassDescriptor(), renderTarget()->sampleCount()))
      swath_.reset();
    plot_.requestMesh();
  }
}

void RhiSwathView::render(QRhiCommandBuffer *cb) {
  QRhiResourceUpdateBatch *updates = rhi()->nextResourceUpdateBatch();
  if (swath_ != nullptr) {
    if (plot_.meshChanged()) {
      std::vector<float> vertices;
      std::vector<unsigned int> indices;
      plot_.buildMesh(vertices, indices);
      swath_->upload(updates, vertices, indices);
    }
    swath_->update(updates, plot_);
  }
  cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
  if (swath_ != nullptr)
    swath_->draw(cb, plot_.deviceViewport(size(), devicePixelRatioF()));
  cb->endPass();
}

void RhiSwathView::mousePressEvent(QMouseEvent *event) {
  double across, depth;
  if (event->button() == Qt::LeftButton && plot_.toData(event->position(), size(), across, depth))
    emit clicked(across, depth);
  QRhiWidget::mousePressEvent(event);
}

void RhiSwathView::resizeEvent(QResizeEvent *event) {
  QRhiWidget::resizeEvent(event);
  overlay_->setGeometry(rect());
}

}  // namespace wcd
