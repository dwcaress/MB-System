/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_state.h  11/7/2026
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
 * The state that mbgrdviz_callbacks.c (Motif) or the Qt program and
 * mbgrdviz_core.c share. The names in the moved code stay as they were:
 * the macros below map them to the shared variables. Include this header
 * only in those C files; it is not for C++ or other code.
 */

#ifndef MBGRDVIZ_MBGRDVIZ_STATE_H
#define MBGRDVIZ_MBGRDVIZ_STATE_H

#include <stdbool.h>

#include "mbview.h"

extern const char mbgrdviz_program_name[];
extern int mbgrdviz_verbose;
extern int mbgrdviz_error;
extern bool mbgrdviz_mbview_id[MBV_MAX_WINDOWS];

#define program_name mbgrdviz_program_name
#define verbose mbgrdviz_verbose
#define error mbgrdviz_error
#define mbview_id mbgrdviz_mbview_id

#endif /* MBGRDVIZ_MBGRDVIZ_STATE_H */
