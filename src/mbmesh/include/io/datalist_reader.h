#pragma once

#include <cstddef>
#include <limits>
#include <filesystem>
#include <string>

#include "../data_types/geometry.h"
#include "../data_types/swathfile.h"
#include "coordinate_frame.h"

struct DatalistReaderOptions {
    GeographicBounds bounds;
    bool use_bounds = false;
    bool reject_flagged_beams = true;
    int verbose = 0;

    // A survey with more accepted soundings than spill_points is not held in memory: the soundings are written, as
    // absolute ECEF, to the file spill_path (see io/point_spill.h) and DatalistReadResult::points is left empty. The
    // default never spills.
    std::size_t spill_points = std::numeric_limits<std::size_t>::max();
    std::filesystem::path spill_path;
};

struct DatalistReaderStats {
    std::size_t files_seen = 0;
    std::size_t files_read = 0;
    std::size_t pings_read = 0;
    std::size_t soundings_read = 0;
    std::size_t soundings_accepted = 0;
    std::size_t soundings_flagged = 0;
    std::size_t soundings_out_of_bounds = 0;
};

struct DatalistReadResult {
    CollectedPointCloud points;   // in the local frame; empty if the soundings were spilled
    bool spilled = false;         // the soundings are in spill_path, as absolute ECEF, and stats.soundings_accepted is their number
    std::filesystem::path spill_path;
    std::vector<TrackPoint> track;   // one per ping that has an accepted sounding, in read order
    std::vector<std::string> files;  // the swath files read, as absolute paths, in read order
    std::vector<int> formats;        // the MBIO formats of those files, each once, ascending
    CoordinateFrame frame;
    DatalistReaderStats stats;
};

// The MB-System version these programs were built with, for example "5.8.3beta25".
[[nodiscard]] std::string mbsystem_version();

[[nodiscard]] bool read_datalist(
    const std::filesystem::path &path,
    const DatalistReaderOptions &options,
    DatalistReadResult *result,
    std::string *error);
