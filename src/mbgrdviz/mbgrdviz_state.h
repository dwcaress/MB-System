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
extern int mbgrdviz_working_route;
extern int mbgrdviz_survey_instance;
extern int mbgrdviz_survey_mode;
extern int mbgrdviz_survey_platform;
extern int mbgrdviz_survey_interleaving;
extern int mbgrdviz_survey_direction;
extern bool mbgrdviz_survey_crosslines_last;
extern int mbgrdviz_survey_crosslines;
extern int mbgrdviz_survey_linespacing;
extern int mbgrdviz_survey_swathwidth;
extern int mbgrdviz_survey_depth;
extern int mbgrdviz_survey_altitude;
extern int mbgrdviz_survey_color;
extern char mbgrdviz_survey_name[MB_PATH_MAXLINE];

#define program_name mbgrdviz_program_name
#define verbose mbgrdviz_verbose
#define error mbgrdviz_error
#define mbview_id mbgrdviz_mbview_id
#define working_route mbgrdviz_working_route
#define survey_instance mbgrdviz_survey_instance
#define survey_mode mbgrdviz_survey_mode
#define survey_platform mbgrdviz_survey_platform
#define survey_interleaving mbgrdviz_survey_interleaving
#define survey_direction mbgrdviz_survey_direction
#define survey_crosslines_last mbgrdviz_survey_crosslines_last
#define survey_crosslines mbgrdviz_survey_crosslines
#define survey_linespacing mbgrdviz_survey_linespacing
#define survey_swathwidth mbgrdviz_survey_swathwidth
#define survey_depth mbgrdviz_survey_depth
#define survey_altitude mbgrdviz_survey_altitude
#define survey_color mbgrdviz_survey_color
#define survey_name mbgrdviz_survey_name

#endif /* MBGRDVIZ_MBGRDVIZ_STATE_H */
