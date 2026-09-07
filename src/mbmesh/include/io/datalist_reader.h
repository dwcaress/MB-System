#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

#include "../data_types/geometry.h"
#include "../data_types/swathfile.h"

struct DatalistReaderOptions {
    GeographicBounds bounds;
    bool use_bounds = false;
    bool reject_flagged_beams = true;
    int verbose = 0;
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
    CollectedPointCloud points;
    CoordinateFrame frame;
    DatalistReaderStats stats;
};

[[nodiscard]] CoordinateFrame coordinate_frame_from_origin(
    const GeodeticPosition &origin);

[[nodiscard]] Vec3 geodetic_to_local(
    const GeodeticPosition &position,
    const CoordinateFrame &frame);

[[nodiscard]] bool read_datalist(
    const std::filesystem::path &path,
    const DatalistReaderOptions &options,
    DatalistReadResult *result,
    std::string *error);
