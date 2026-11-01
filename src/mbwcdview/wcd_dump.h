/*--------------------------------------------------------------------
 *    The MB-system:  wcd_dump.h  1/11/2026
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

#ifndef MBWCDVIEW_WCD_DUMP_H
#define MBWCDVIEW_WCD_DUMP_H

#include <string>

namespace wcd {

/* print a summary of the line of path (mbwcdview --dump); returns the exit code */
int dump(const std::string &path, int format, int ping_index);

}  // namespace wcd

#endif
