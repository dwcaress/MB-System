#include "io/datalist_reader.h"

#include "io/point_spill.h"
#include "util/logger.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <system_error>
#include <cstddef>
#include <cstring>
#include <string>
#include <utility>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
}

namespace {

struct ReadArrays {
    char *beam_flags = nullptr;
    double *bathymetry = nullptr;
    double *amplitude = nullptr;
    double *bathymetry_across_track = nullptr;
    double *bathymetry_along_track = nullptr;
    double *sidescan = nullptr;
    double *sidescan_across_track = nullptr;
    double *sidescan_along_track = nullptr;
};

struct ReadContext {
    const DatalistReaderOptions &options;
    DatalistReadResult &result;

    // Soundings are held as absolute ECEF until the frame origin is known.
    // The centroid is accumulated relative to the first point to keep the
    // sum well conditioned.
    // Track: the file being read, and whether the current ping is recorded yet.
    std::size_t file_index = 0;
    bool ping_recorded = false;

    bool has_reference = false;
    Vec3 reference_ecef;
    Vec3 offset_sum;

    // Once more soundings are accepted than the options allow in memory, they go to this file instead.
    PointSpill spill;
    bool spilling = false;
};

void set_error(std::string *error, const std::string &message) {
    if (error != nullptr) {
        *error = message;
    }
}

[[nodiscard]] std::string mbio_error_message(int verbose, int error_code) {
    char *message = nullptr;
    mb_error(verbose, error_code, &message);
    if (message == nullptr) {
        return "MBIO error " + std::to_string(error_code);
    }
    return message;
}

[[nodiscard]] bool is_finite(const GeodeticPosition &position) {
    return std::isfinite(position.longitude) && std::isfinite(position.latitude) && std::isfinite(position.elevation);
}

[[nodiscard]] bool is_valid_record(const Ping &ping, const Sounding &sounding) {
    return std::isfinite(ping.timestamp) && is_finite(ping.sensor_position) && is_finite(sounding.position);
}

[[nodiscard]] CollectedPoint make_collected_point(const Ping &ping, const Sounding &sounding) {
    CollectedPoint collected_point;
    collected_point.point = geodetic_to_ecef(sounding.position);
    collected_point.origin = geodetic_to_ecef(ping.sensor_position);
    return collected_point;
}

bool append_sounding_to_pointcloud(const Ping &ping, const Sounding &sounding, ReadContext *context, std::string *error) {
    if (context == nullptr) {
        set_error(error, "Internal error: datalist read context is null");
        return false;
    }

    if (!is_valid_record(ping, sounding)) {
        set_error(error, "Sounding record contains a non-finite coordinate or timestamp");
        return false;
    }

    DatalistReaderStats &stats = context->result.stats;
    ++stats.soundings_read;

    if (context->options.reject_flagged_beams && !mb_beam_ok(sounding.beam_flag)) {
        ++stats.soundings_flagged;
        return true;
    }

    if (context->options.use_bounds && !context->options.bounds.contains(sounding.position.longitude, sounding.position.latitude)) {
        ++stats.soundings_out_of_bounds;
        return true;
    }

    const CollectedPoint collected_point = make_collected_point(ping, sounding);
    if (!context->ping_recorded) {
        // Held as absolute ECEF, like the points, until the frame is chosen.
        context->result.track.push_back({ping.timestamp, collected_point.origin, context->file_index});
        context->ping_recorded = true;
    }
    if (!context->has_reference) {
        context->reference_ecef = collected_point.point;
        context->has_reference = true;
    }
    context->offset_sum += collected_point.point - context->reference_ecef;
    // The sensor position is the same for all the soundings of a ping, so a spilled sounding keeps the number of its
    // ping, which is the last in the track, instead of the position.
    const std::uint32_t ping_index = static_cast<std::uint32_t>(context->result.track.size() - 1);
    if (context->spilling) {
        if (!context->spill.append(collected_point.point, ping_index, error)) {
            return false;
        }
    } else {
        context->result.points.push_back(collected_point);
        if (context->result.points.size() > context->options.spill_points) {
            // Too many for memory: what has been read so far goes to the file, and so does everything after.
            std::error_code directory_error;
            std::filesystem::create_directories(context->options.spill_path.parent_path(), directory_error);
            if (directory_error) {
                set_error(error, "unable to create the directory of the spilled soundings: " + directory_error.message());
                return false;
            }
            if (!context->spill.open(context->options.spill_path, error)) {
                return false;
            }
            const std::filesystem::space_info space = std::filesystem::space(context->options.spill_path.parent_path(), directory_error);
            if (!directory_error) {
                LOG_INFO("[input]", "spilling the soundings to " + context->options.spill_path.string() + ", " + std::to_string(spill_record_bytes) +
                                        " bytes each; " + std::to_string(space.available / 1000000000) + " GB of disk is free there");
            }
            // The points held so far have their sensor positions in them: find their pings, which are in the track in order.
            std::size_t ping = 0;
            for (const CollectedPoint &held : context->result.points) {
                while (ping < context->result.track.size() && !(context->result.track[ping].position.x == held.origin.x &&
                                                                  context->result.track[ping].position.y == held.origin.y &&
                                                                  context->result.track[ping].position.z == held.origin.z)) {
                    ping++;
                }
                if (ping >= context->result.track.size()) {
                    set_error(error, "internal error: a sounding has a sensor position that is not in the track");
                    return false;
                }
                if (!context->spill.append(held.point, static_cast<std::uint32_t>(ping), error)) {
                    return false;
                }
            }
            context->spilling = true;
            CollectedPointCloud().swap(context->result.points);
        }
    }
    ++stats.soundings_accepted;
    return true;
}

bool register_read_array(int verbose, void *mbio, int memory_type, std::size_t element_size, void **array, const char *array_name, std::string *error) {
    int error_code = MB_ERROR_NO_ERROR;
    if (mb_register_array( verbose, mbio, memory_type, element_size, array, &error_code) == MB_SUCCESS) {
        return true;
    }

    set_error(error, "Unable to allocate MBIO " + std::string(array_name) + " array: " + mbio_error_message(verbose, error_code));
    return false;
}

bool register_read_arrays(int verbose, void *mbio, ReadArrays *arrays, std::string *error) {
    return register_read_array(verbose, mbio, MB_MEM_TYPE_BATHYMETRY, sizeof(char),   reinterpret_cast<void **>(&arrays->beam_flags),              "beam flag",               error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_BATHYMETRY, sizeof(double), reinterpret_cast<void **>(&arrays->bathymetry),              "bathymetry",              error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_AMPLITUDE,  sizeof(double), reinterpret_cast<void **>(&arrays->amplitude),               "amplitude",               error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_BATHYMETRY, sizeof(double), reinterpret_cast<void **>(&arrays->bathymetry_across_track), "bathymetry across-track", error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_BATHYMETRY, sizeof(double), reinterpret_cast<void **>(&arrays->bathymetry_along_track),  "bathymetry along-track",  error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_SIDESCAN,   sizeof(double), reinterpret_cast<void **>(&arrays->sidescan),                "sidescan",                error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_SIDESCAN,   sizeof(double), reinterpret_cast<void **>(&arrays->sidescan_across_track),   "sidescan across-track",   error) &&
           register_read_array(verbose, mbio, MB_MEM_TYPE_SIDESCAN,   sizeof(double), reinterpret_cast<void **>(&arrays->sidescan_along_track),    "sidescan along-track",    error);
}

bool process_ping(int verbose, double timestamp, double navigation_longitude, double navigation_latitude, double heading, double sensor_depth, int beam_count, const ReadArrays &arrays, ReadContext *context, std::string *error) {
    if (!std::isfinite(timestamp) || !std::isfinite(navigation_longitude) || !std::isfinite(navigation_latitude) || !std::isfinite(heading) || !std::isfinite(sensor_depth)) {
        set_error(error, "Survey ping contains non-finite navigation data");
        return false;
    }

    double meters_to_degrees_longitude = 0.0;
    double meters_to_degrees_latitude = 0.0;
    int error_code = MB_ERROR_NO_ERROR;
    if (mb_coor_scale(verbose, navigation_latitude, &meters_to_degrees_longitude, &meters_to_degrees_latitude) != MB_SUCCESS) {
        set_error(error, "Unable to calculate coordinate scale: " + mbio_error_message(verbose, error_code));
        return false;
    }

    constexpr double pi = 3.14159265358979323846;
    constexpr double degrees_to_radians = pi / 180.0;
    const double heading_x = std::sin(heading * degrees_to_radians);
    const double heading_y = std::cos(heading * degrees_to_radians);

    context->ping_recorded = false;

    Ping ping;
    ping.timestamp = timestamp;
    ping.sensor_position = { navigation_longitude, navigation_latitude, -sensor_depth };

    for (int beam_index = 0; beam_index < beam_count; ++beam_index) {
        const double across_track = arrays.bathymetry_across_track[beam_index];
        const double along_track = arrays.bathymetry_along_track[beam_index];

        Sounding sounding;
        sounding.position.longitude = navigation_longitude + heading_y * meters_to_degrees_longitude * across_track + heading_x * meters_to_degrees_longitude * along_track;
        sounding.position.latitude = navigation_latitude - heading_x * meters_to_degrees_latitude * across_track + heading_y * meters_to_degrees_latitude * along_track;
        sounding.position.elevation = -arrays.bathymetry[beam_index];
        sounding.beam_index = static_cast<std::size_t>(beam_index);
        sounding.beam_flag = arrays.beam_flags[beam_index];

        if (!append_sounding_to_pointcloud(ping, sounding, context, error)) {
            return false;
        }
    }

    return true;
}

bool read_swath_file(const char *file, int format, int alternate_navigation_status, char *alternate_navigation_path, ReadContext *context, std::string *error) {
    const int verbose = context->options.verbose;
    constexpr int pings = 1;
    constexpr int longitude_flip = 0;
    double bounds[4] = {-360.0, 360.0, -90.0, 90.0};
    int begin_time[7] = {1930, 1, 1, 0, 0, 0, 0};
    int end_time[7] = {3000, 1, 1, 0, 0, 0, 0};
    constexpr double minimum_speed = 0.0;
    constexpr double time_gap = 1.0e9;

    void *mbio = nullptr;
    double begin_time_d = 0.0;
    double end_time_d = 0.0;
    int maximum_bathymetry_beams = 0;
    int maximum_amplitude_beams = 0;
    int maximum_sidescan_pixels = 0;
    int error_code = MB_ERROR_NO_ERROR;

    if (mb_read_init_altnav(
        verbose,
        const_cast<char *>(file),
        format,
        pings,
        longitude_flip,
        bounds,
        begin_time,
        end_time,
        minimum_speed,
        time_gap,
        alternate_navigation_status,
        alternate_navigation_path,
        &mbio,
        &begin_time_d,
        &end_time_d,
        &maximum_bathymetry_beams,
        &maximum_amplitude_beams,
        &maximum_sidescan_pixels,
        &error_code
    ) != MB_SUCCESS) {
        set_error(error, "Unable to initialize swath file '" + std::string(file) + "': " + mbio_error_message(verbose, error_code));
        return false;
    }

    ReadArrays arrays;
    if (!register_read_arrays(verbose, mbio, &arrays, error)) {
        int close_error = MB_ERROR_NO_ERROR;
        mb_close(verbose, &mbio, &close_error);
        return false;
    }

    bool success = true;
    while (true) {
        void *store = nullptr;
        int kind = MB_DATA_NONE;
        int time_i[7] = {};
        double timestamp = 0.0;
        double navigation_longitude = 0.0;
        double navigation_latitude = 0.0;
        double speed = 0.0;
        double heading = 0.0;
        double distance = 0.0;
        double altitude = 0.0;
        double sensor_depth = 0.0;
        int bathymetry_beams = 0;
        int amplitude_beams = 0;
        int sidescan_pixels = 0;
        char comment[MB_COMMENT_MAXLINE] = {};

        error_code = MB_ERROR_NO_ERROR;
        int status = mb_get_all(
            verbose,
            mbio,
            &store,
            &kind,
            time_i,
            &timestamp,
            &navigation_longitude,
            &navigation_latitude,
            &speed,
            &heading,
            &distance,
            &altitude,
            &sensor_depth,
            &bathymetry_beams,
            &amplitude_beams,
            &sidescan_pixels,
            arrays.beam_flags,
            arrays.bathymetry,
            arrays.amplitude,
            arrays.bathymetry_across_track,
            arrays.bathymetry_along_track,
            arrays.sidescan,
            arrays.sidescan_across_track,
            arrays.sidescan_along_track,
            comment,
            &error_code
        );

        if (error_code == MB_ERROR_TIME_GAP) {
            error_code = MB_ERROR_NO_ERROR;
            status = MB_SUCCESS;
        }

        if (status != MB_SUCCESS) {
            if (error_code == MB_ERROR_EOF) {
                break;
            }
            if (error_code < MB_ERROR_NO_ERROR) {
                continue;
            }

            set_error(error, "Error reading swath file '" + std::string(file) + "': " + mbio_error_message(verbose, error_code));
            success = false;
            break;
        }

        if (kind != MB_DATA_DATA) {
            continue;
        }

        ++context->result.stats.pings_read;
        if (!process_ping(verbose, timestamp, navigation_longitude, navigation_latitude, heading, sensor_depth, bathymetry_beams, arrays, context, error)) {
            success = false;
            break;
        }
    }

    int close_error = MB_ERROR_NO_ERROR;
    if (mb_close(verbose, &mbio, &close_error) != MB_SUCCESS && success) {
        set_error(error, "Unable to close swath file '" + std::string(file) + "': " + mbio_error_message(verbose, close_error));
        success = false;
    }

    return success;
}

} // namespace

bool GeographicBounds::contains(double lon, double lat) const {
    const bool latitude_in_bounds = lat >= degrees_S && lat <= degrees_N;
    const bool longitude_in_bounds = degrees_W <= degrees_E ? lon >= degrees_W && lon <= degrees_E : lon >= degrees_W || lon <= degrees_E;
    return latitude_in_bounds && longitude_in_bounds;
}

std::string mbsystem_version() {
    return MB_VERSION;
}

bool read_datalist(const std::filesystem::path &path, const DatalistReaderOptions &options, DatalistReadResult *result, std::string *error) {
    if (error != nullptr) {
        error->clear();
    }

    if (result == nullptr) {
        set_error(error, "Datalist output result must not be null");
        return false;
    }

    if (path.empty()) {
        set_error(error, "Input datalist path is empty");
        return false;
    }

    std::string datalist_path = path.string();
    if (datalist_path.size() >= MB_PATH_MAXLINE) {
        set_error(error, "Input datalist path is too long: " + datalist_path);
        return false;
    }

    const int verbose = options.verbose;
    void *datalist = nullptr;
    int error_code = MB_ERROR_NO_ERROR;
    if (mb_datalist_open(verbose, &datalist, datalist_path.data(), MB_DATALIST_LOOK_UNSET, &error_code) != MB_SUCCESS) {
        set_error(error, "Unable to open datalist '" + datalist_path + "': " + mbio_error_message(verbose, error_code));
        return false;
    }

    DatalistReadResult read_result;
    ReadContext context{options, read_result};
    bool success = true;

    while (true) {
        int processed_status = MB_PROCESSED_NONE;
        int alternate_navigation_status = MB_ALTNAV_NONE;
        mb_path raw_path = {};
        mb_path processed_path = {};
        mb_path alternate_navigation_path = {};
        mb_path data_path = {};
        int format = 0;
        double file_weight = 1.0;

        error_code = MB_ERROR_NO_ERROR;
        const int status = mb_datalist_read3(
            verbose,
            datalist,
            &processed_status,
            raw_path,
            processed_path,
            &alternate_navigation_status,
            alternate_navigation_path,
            data_path,
            &format,
            &file_weight,
            &error_code);

        if (status != MB_SUCCESS) {
            if (error_code != MB_ERROR_EOF && error_code != MB_ERROR_NO_ERROR) {
                set_error(error, "Error reading datalist '" + datalist_path + "': " + mbio_error_message(verbose, error_code));
                success = false;
            }
            break;
        }

        if (format <= 0) {
            continue;
        }

        ++read_result.stats.files_seen;
        char *file = processed_status == MB_PROCESSED_USE ? processed_path : raw_path;

        context.file_index = read_result.stats.files_read;
        if (!read_swath_file(file, format, alternate_navigation_status, alternate_navigation_path, &context, error)) {
            success = false;
            break;
        }

        {
            std::error_code path_error;
            const std::filesystem::path absolute_file = std::filesystem::absolute(file, path_error);
            read_result.files.push_back(path_error ? std::string(file) : absolute_file.lexically_normal().string());
            if (std::find(read_result.formats.begin(), read_result.formats.end(), format) == read_result.formats.end()) {
                read_result.formats.push_back(format);
            }
        }

        ++read_result.stats.files_read;
    }

    int close_error = MB_ERROR_NO_ERROR;
    if (mb_datalist_close(verbose, &datalist, &close_error) != MB_SUCCESS && success) {
        set_error(error, "Unable to close datalist '" + datalist_path + "': " + mbio_error_message(verbose, close_error));
        success = false;
    }

    if (!success) {
        return false;
    }

    if (read_result.stats.soundings_accepted == 0) {
        set_error(error, "Datalist produced no accepted bathymetry soundings: " + datalist_path);
        return false;
    }
    if (context.spilling) {
        if (!context.spill.finish(error)) {
            return false;
        }
        read_result.spilled = true;
        read_result.spill_path = context.spill.path();
    }

    // Choose the reconstruction frame origin now that all soundings have been
    // seen: the point on the ellipsoid below the centroid. This internal frame
    // is independent of --geoorigin, which only sets the output offset.
    const double count = static_cast<double>(read_result.stats.soundings_accepted);
    const Vec3 centroid_ecef = context.reference_ecef + context.offset_sum * (1.0 / count);
    GeodeticPosition origin = ecef_to_geodetic(centroid_ecef);
    origin.elevation = 0.0;

    // Round so the same survey always yields the same, readable origin.
    constexpr double origin_resolution_degrees = 1.0e-6;
    origin.longitude = std::round(origin.longitude / origin_resolution_degrees) * origin_resolution_degrees;
    origin.latitude = std::round(origin.latitude / origin_resolution_degrees) * origin_resolution_degrees;
    read_result.frame = coordinate_frame_from_origin(origin);

    for (CollectedPoint &collected_point : read_result.points) {
        collected_point.point = ecef_to_local(collected_point.point, read_result.frame);
        collected_point.origin = ecef_to_local(collected_point.origin, read_result.frame);
    }

    for (TrackPoint &track_point : read_result.track) {
        track_point.position = ecef_to_local(track_point.position, read_result.frame);
    }

    std::sort(read_result.formats.begin(), read_result.formats.end());

    *result = std::move(read_result);
    return true;
}
