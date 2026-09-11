/*--------------------------------------------------------------------
 *    The MB-system:  mb_xmutil.h
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Shared helpers for MB-System's Motif GUI programs (mbedit, mbnavedit,
 * mbvelocitytool, mbeditviz, mbnavadjust, mbgrdviz). Consolidates logic
 * that was previously reimplemented (and independently bug-fixed, or not)
 * in each program separately.
 */

#ifndef MB_XMUTIL_H_
#define MB_XMUTIL_H_

#include <Xm/Xm.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Copy the text currently in Motif text widget w into str, a caller-owned
 * buffer of len bytes, bounding the copy so it never overflows str and
 * always leaves str null-terminated (including when the widget is empty
 * or XmTextGetString() fails and returns NULL). Frees the Xt-allocated
 * string XmTextGetString() returns. */
void mb_get_text_string(Widget w, String str, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MB_XMUTIL_H_ */
