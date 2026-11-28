/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_input.cc  11/7/2026
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

#include "mbvq_input.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <cmath>

namespace mbvq {

static int button_number(Qt::MouseButton button) {
  switch (button) {
  case Qt::LeftButton:
    return 1;
  case Qt::MiddleButton:
    return 2;
  case Qt::RightButton:
    return 3;
  default:
    return 0;
  }
}

bool translate_event(const QEvent *qevent, double device_pixel_ratio, struct mbv_input_event *event) {
  *event = mbv_input_event{MBV_INPUT_OTHER, 0, 0, 0, 0};
  switch (qevent->type()) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove: {
    const auto *mouse = static_cast<const QMouseEvent *>(qevent);
    if (qevent->type() == QEvent::MouseMove) {
      /* the core gets motion only while a button is down, as from Motif */
      if (mouse->buttons() == Qt::NoButton)
        return false;
      event->type = MBV_INPUT_MOTION;
    }
    else {
      event->button = button_number(mouse->button());
      if (event->button == 0)
        return false;
      event->type = qevent->type() == QEvent::MouseButtonPress ? MBV_INPUT_PRESS : MBV_INPUT_RELEASE;
    }
    event->x = static_cast<int>(std::lround(mouse->position().x() * device_pixel_ratio));
    event->y = static_cast<int>(std::lround(mouse->position().y() * device_pixel_ratio));
    return true;
  }
  case QEvent::KeyPress:
  case QEvent::KeyRelease: {
    /* auto-repeated keys come as press and release pairs, as from X */
    const auto *key = static_cast<const QKeyEvent *>(qevent);
    event->type = qevent->type() == QEvent::KeyPress ? MBV_INPUT_KEYPRESS : MBV_INPUT_KEYRELEASE;
    const QString text = key->text();
    event->key = text.isEmpty() ? 0 : text.at(0).toLatin1();
    return true;
  }
  default:
    return false;
  }
}

}  // namespace mbvq
