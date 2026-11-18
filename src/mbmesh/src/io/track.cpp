#include "io/track.h"

#include <algorithm>
#include <cstddef>
#include <vector>

std::vector<std::vector<Vec3>> split_track(const std::vector<TrackPoint> &track, double gap_factor) {
    std::vector<std::vector<Vec3>> polylines;

    std::size_t first = 0;
    while (first < track.size()) {
        // The pings of one swath file are consecutive in the track.
        std::size_t last = first + 1;
        while (last < track.size() && track[last].segment == track[first].segment) {
            last++;
        }

        // The usual time between pings in this file.
        std::vector<double> intervals;
        for (std::size_t i = first + 1; i < last; i++) {
            const double interval = track[i].time - track[i - 1].time;
            if (interval > 0.0) {
                intervals.push_back(interval);
            }
        }
        double median = 0.0;
        if (!intervals.empty()) {
            std::nth_element(intervals.begin(), intervals.begin() + intervals.size() / 2, intervals.end());
            median = intervals[intervals.size() / 2];
        }

        std::vector<Vec3> current;
        for (std::size_t i = first; i < last; i++) {
            if (i > first) {
                const double interval = track[i].time - track[i - 1].time;
                const bool backwards = interval < 0.0;
                const bool gap = median > 0.0 && interval > gap_factor * median;
                if (backwards || gap) {
                    if (current.size() >= 2) {
                        polylines.push_back(std::move(current));
                    }
                    current.clear();
                }
            }
            current.push_back(track[i].position);
        }
        if (current.size() >= 2) {
            polylines.push_back(std::move(current));
        }

        first = last;
    }
    return polylines;
}
