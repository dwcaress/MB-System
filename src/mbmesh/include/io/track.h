#pragma once

#include <vector>

#include "../data_types/geometry.h"

/*
 * The sensor track: one position per ping, in the order read.
 *
 * Joining every consecutive pair of pings would draw false straight lines across
 * the turns between survey lines and across gaps where nothing was logged. The
 * track is therefore split into polylines wherever the swath file changes, the
 * time runs backwards, or the time between two pings is more than gap_factor
 * times the median time between pings in that file.
 *
 * Polylines with fewer than two points are dropped.
 */
[[nodiscard]] std::vector<std::vector<Vec3>> split_track(
    const std::vector<TrackPoint> &track,
    double gap_factor = 10.0);
