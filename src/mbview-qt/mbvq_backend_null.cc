/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_backend_null.cc  11/7/2026
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
 * Null backend: draws nothing. The core keeps glx_init false, so it skips
 * all drawing and finds no picks, while every other part (mouse modes,
 * background work, labels) runs as usual. For tests without a display.
 */

#include <QEvent>
#include <QPainter>
#include <QWidget>
#include <cmath>

#include "mbv_input.h"
#include "mbv_render.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"
#include "mbvq_input.h"

namespace mbvq {
namespace null_backend {

namespace {

class NullMapView final : public QWidget {
 public:
  NullMapView(size_t instance, QWidget *parent) : QWidget(parent), instance_(instance) {
    setFocusPolicy(Qt::StrongFocus);
  }

 protected:
  void resizeEvent(QResizeEvent *) override {
    mbvq_c_resize(instance_, static_cast<int>(std::lround(width() * devicePixelRatioF())),
                  static_cast<int>(std::lround(height() * devicePixelRatioF())));
  }
  void paintEvent(QPaintEvent *) override {
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());
    painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("(no renderer)"));
  }
  bool event(QEvent *qevent) override {
    struct mbv_input_event event;
    if (translate_event(qevent, devicePixelRatioF(), &event)) {
      mbview_input(instance_, &event);
      return true;
    }
    return QWidget::event(qevent);
  }

 private:
  size_t instance_;
};

bool defer_draw(int view, size_t instance, int rez) {
  (void)view;
  (void)instance;
  (void)rez;
  return true;
}

}  // namespace

void install() {
  const struct mbv_render_ops ops = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, defer_draw};
  mbv_render_set_ops(&ops);
}

QWidget *create_map_view(size_t instance, QWidget *parent) { return new NullMapView(instance, parent); }

QWidget *create_profile_view(size_t instance, QWidget *parent) {
  /* nothing is drawn; the size is still passed on as for a drawing */
  auto *view = new QWidget(parent);
  (void)instance;
  return view;
}

}  // namespace null_backend
}  // namespace mbvq
