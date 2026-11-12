/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_prog.c  9/11/2026
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
 * Compiles the unchanged src/mbnavadjust/mbnavadjust_prog.c for mbnavadjust-qt.
 * That file is the mbnavadjust program logic, shared with the Motif program so
 * the two cannot drift apart. It has five global variables whose names other
 * tools also use, so those are renamed here. The headers come first, so that
 * only that one file sees the renames. X11/Intrinsic.h is the stand-in in the
 * x11 directory.
 */

#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include <X11/Intrinsic.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
#include "mbsys_ldeoih.h"
#include "mbview.h"

#define error mbnaq_prog_error
#define message mbnaq_prog_message
#define ping mbnaq_prog_ping
#define ncolors mbnaq_prog_ncolors
#define pixel_values mbnaq_prog_pixel_values

#include "mbnavadjust_prog.c"
