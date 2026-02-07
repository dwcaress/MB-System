/*--------------------------------------------------------------------
 *    The MB-system:  logger.cpp  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "logger.h"

namespace mbmesh {

LogLevel Logger::current_level_ = LogLevel::INFO;

}  // namespace mbmesh
