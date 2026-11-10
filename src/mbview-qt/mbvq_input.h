/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_input.h  11/7/2026
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
 * The one translation of Qt mouse and key events into the toolkit-neutral
 * events of mbv_input.h, shared by all backends.
 */

#ifndef MBVIEW_QT_MBVQ_INPUT_H
#define MBVIEW_QT_MBVQ_INPUT_H

#include "mbv_input.h"

class QEvent;

namespace mbvq {

/* Translate a mouse or key event of a view whose pixels are scaled by
   device_pixel_ratio. Positions become physical pixels from the top left,
   as the core expects. Returns false for events the core does not take:
   other event types and buttons other than left, middle and right. */
bool translate_event(const QEvent *qevent, double device_pixel_ratio, struct mbv_input_event *event);

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_INPUT_H */
