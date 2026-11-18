#include "io/glb_writer.h"
#include "io/track.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

TrackPoint ping(double time, double x, std::size_t segment) {
    TrackPoint point;
    point.time = time;
    point.position = Vec3(x, 2.0 * x, -100.0);
    point.segment = segment;
    return point;
}

std::vector<std::size_t> sizes(const std::vector<std::vector<Vec3>> &polylines) {
    std::vector<std::size_t> result;
    for (const auto &polyline : polylines) {
        result.push_back(polyline.size());
    }
    return result;
}

// Reads the JSON chunk of a GLB file.
std::string glb_json(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (bytes.size() < 20 || std::memcmp(bytes.data(), "glTF", 4) != 0) {
        return "";
    }
    std::uint32_t length = 0;
    std::memcpy(&length, bytes.data() + 12, 4);
    return bytes.substr(20, length);
}

// Reads the three numbers after a key such as "max":[ in the JSON.
bool read_triplet(const std::string &json, const std::string &key, double *values) {
    const std::size_t at = json.find(key);
    if (at == std::string::npos) {
        return false;
    }
    return std::sscanf(json.c_str() + at + key.size(), "%lf,%lf,%lf", &values[0], &values[1], &values[2]) == 3;
}

} // namespace

int main() {
    bool ok = true;

    // ---- splitting the track
    {
        std::vector<TrackPoint> track;
        // File 0: ten pings one second apart, then a 91 s gap, then five more.
        for (int i = 0; i < 10; i++) {
            track.push_back(ping(i, i, 0));
        }
        for (int i = 0; i < 5; i++) {
            track.push_back(ping(100 + i, 10 + i, 0));
        }
        // File 1: three pings. File 2: a single ping, which is not a line.
        for (int i = 0; i < 3; i++) {
            track.push_back(ping(200 + i, 20 + i, 1));
        }
        track.push_back(ping(300, 30, 2));

        const std::vector<std::size_t> expected = {10, 5, 3};
        ok &= check(sizes(split_track(track)) == expected,
                    "the track is split at a time gap and at a change of file, and a single ping is dropped");

        const auto polylines = split_track(track);
        ok &= check(polylines.size() == 3 && polylines[0][0].x == 0.0 && polylines[0][9].x == 9.0 &&
                        polylines[1][0].x == 10.0 && polylines[2][0].x == 20.0,
                    "polylines keep the order and the positions of the pings");
    }

    {
        // Time running backwards, as when two passes are in one file, starts a new line.
        std::vector<TrackPoint> track;
        for (const double time : {5.0, 6.0, 7.0, 3.0, 4.0, 5.0}) {
            track.push_back(ping(time, time, 0));
        }
        const std::vector<std::size_t> expected = {3, 3};
        ok &= check(sizes(split_track(track)) == expected, "the track is split where the time runs backwards");
    }

    {
        // A change in the ping rate of a few times is not a gap.
        std::vector<TrackPoint> track;
        double time = 0.0;
        for (int i = 0; i < 20; i++) {
            time += (i % 5 == 0) ? 3.0 : 1.0;
            track.push_back(ping(time, i, 0));
        }
        ok &= check(split_track(track).size() == 1, "an uneven ping interval is not treated as a gap");
        ok &= check(split_track({}).empty(), "an empty track gives no polylines");
    }

    // ---- writing the track as lines
    {
        const std::vector<std::vector<Vec3>> polylines = {
            {Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0), Vec3(1.0, 1.0, 0.0)},
            {Vec3(5.0, 5.0, 5.0), Vec3(6.0, 5.0, 5.0)},
        };
        const std::string path = (std::filesystem::temp_directory_path() / "mbmesh_test_track.glb").string();
        std::string error;
        ok &= check(write_track_glb_file(path, polylines, &error), "writing the track succeeds: " + error);

        const std::string json = glb_json(path);
        ok &= check(!json.empty(), "the track file is a GLB");
        // 2 + 1 segments, each two vertices
        ok &= check(json.find("\"count\":6") != std::string::npos, "the track holds one vertex pair per segment");
        ok &= check(json.find("\"mode\":1") != std::string::npos, "the track is drawn as lines");
        double color[3] = {1, 1, 1};
        ok &= check(read_triplet(json, "\"baseColorFactor\":[", color) && color[0] == 0.0 && color[1] == 0.0 && color[2] == 0.0, "the track is black");
        double minimum[3] = {1, 1, 1}, maximum[3] = {0, 0, 0};
        ok &= check(read_triplet(json, "\"min\":[", minimum) && read_triplet(json, "\"max\":[", maximum) &&
                        minimum[0] == 0.0 && minimum[1] == 0.0 && minimum[2] == 0.0 &&
                        maximum[0] == 6.0 && maximum[1] == 5.0 && maximum[2] == 5.0,
                    "the position bounds cover the track");
        std::filesystem::remove(path);

        ok &= check(!write_track_glb_file(path, {}, &error) && !error.empty(), "an empty track is an error with a message");
        ok &= check(!write_track_glb_file(path, {{Vec3(0.0), Vec3(1.0e39, 0.0, 0.0)}}, &error) && !error.empty(),
                    "a position outside the float range is an error");
    }

    std::printf(ok ? "track tests passed\n" : "track tests FAILED\n");
    return ok ? 0 : 1;
}
