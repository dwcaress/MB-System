#include "io/point_spill.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

// Points whose coordinates say which one they are: x is the index, y a spread over several chunks.
CollectedPoint make_point(std::size_t n) {
    const double x = static_cast<double>(n % 97) * 0.37 - 5.0;
    const double y = static_cast<double>((n * 31) % 101) * 0.29 - 7.0;
    return {Vec3(x, y, static_cast<double>(n)), Vec3(x, y, static_cast<double>(n) + 20.0)};
}

} // namespace

int main() {
    bool ok = true;
    const std::filesystem::path directory = std::filesystem::temp_directory_path() / ("mbmesh_test_point_spill_" + std::to_string(getpid()));
    std::filesystem::create_directories(directory);
    std::string error;

    constexpr std::size_t count = 25000;

    // ---- the spill file
    {
        // 50 pings, each with its own sensor position; sounding n is from ping n % 50.
        std::vector<Vec3> origins;
        for (int ping = 0; ping < 50; ping++) {
            origins.push_back(Vec3(100.0 + ping, 200.0 - ping, 300.0 + 0.5 * ping));
        }
        PointSpill spill;
        ok &= check(spill.open(directory / "soundings.bin", &error), "the spill file opens: " + error);
        for (std::size_t n = 0; n < count; n++) {
            ok &= spill.append(make_point(n).point, static_cast<std::uint32_t>(n % 50), &error);
        }
        ok &= check(spill.finish(&error), "the spill file closes: " + error);
        ok &= check(spill.count() == count, "the spill counts its points");
        ok &= check(spill_record_bytes == 28 && std::filesystem::file_size(directory / "soundings.bin") == count * spill_record_bytes,
                    "a spilled sounding takes 28 bytes: three doubles and a ping number");

        std::size_t seen = 0;
        std::size_t pieces = 0;
        bool in_order = true;
        ok &= check(read_spill_pieces(directory / "soundings.bin", origins, 7000, [&](CollectedPointCloud &piece, std::size_t first) {
                        pieces++;
                        in_order = in_order && first == seen && piece.size() <= 7000;
                        for (std::size_t n = 0; n < piece.size(); n++) {
                            const CollectedPoint expected = make_point(first + n);
                            const Vec3 &origin = origins[(first + n) % 50];
                            in_order = in_order && piece[n].point.z == expected.point.z && piece[n].point.y == expected.point.y &&
                                       piece[n].point.x == expected.point.x && piece[n].origin.x == origin.x &&
                                       piece[n].origin.y == origin.y && piece[n].origin.z == origin.z;
                        }
                        seen += piece.size();
                        return true;
                    }, &error), "the spill file is read in pieces: " + error);
        ok &= check(seen == count && pieces == 4 && in_order, "the pieces hold every point, in order, with their sensor positions and first indices");

        pieces = 0;
        ok &= check(read_spill_pieces(directory / "soundings.bin", origins, 7000, [&](CollectedPointCloud &, std::size_t) {
                        pieces++;
                        return false;
                    }, &error) && pieces == 1, "a callback that returns false stops the reading");

        CollectedPointCloud range;
        ok &= check(read_spill_range(directory / "soundings.bin", origins, 12345, 100, &range, &error) && range.size() == 100 &&
                        range[0].point.z == 12345.0 && range[99].point.z == 12444.0 && range[0].origin.x == origins[12345 % 50].x,
                    "a range of the spill file is read by index");
        ok &= check(!read_spill_range(directory / "soundings.bin", origins, count - 10, 100, &range, &error), "a range past the end is an error");
        ok &= check(!read_spill_pieces(directory / "soundings.bin", std::vector<Vec3>(10), 100, [](CollectedPointCloud &, std::size_t) { return true; }, &error) &&
                        error.find("ping") != std::string::npos,
                    "a ping that the track does not have is an error");
        ok &= check(!read_spill_pieces(directory / "missing.bin", origins, 10, [](CollectedPointCloud &, std::size_t) { return true; }, &error),
                    "a missing spill file is an error");
    }

    // ---- the chunk files
    {
        constexpr double chunk = 4.0;
        constexpr double halo = 0.5;
        ChunkBins bins(directory / "chunks", chunk, halo, 3000);   // flushed several times
        ok &= check(bins.open(&error), "the chunk directory opens: " + error);
        std::map<std::pair<int, int>, std::vector<std::size_t>> expected;   // chunk -> indices, in order
        std::map<std::pair<int, int>, std::size_t> expected_owned;
        for (std::size_t n = 0; n < count; n++) {
            const CollectedPoint point = make_point(n);
            ok &= bins.add(point, &error);
            for_each_chunk_of_point(point.point.x, point.point.y, chunk, halo, [&](int i, int j, bool owned) {
                expected[{i, j}].push_back(n);
                if (owned) {
                    expected_owned[{i, j}]++;
                }
            });
        }
        ok &= check(bins.finish(&error), "the chunk files are finished: " + error);
        ok &= check(bins.chunks().size() == expected.size(), "the chunk files are the chunks the points fall in");
        ok &= check(bins.owned_point_count() == count, "every point is owned by exactly one chunk");

        bool counts_match = true;
        bool contents_match = true;
        for (const auto &entry : expected) {
            const auto found = bins.chunks().find(entry.first);
            counts_match = counts_match && found != bins.chunks().end() && found->second.total == entry.second.size() &&
                           found->second.owned == expected_owned[entry.first];
            CollectedPointCloud points;
            if (!bins.read_chunk(entry.first.first, entry.first.second, &points, &error) || points.size() != entry.second.size()) {
                contents_match = false;
                continue;
            }
            for (std::size_t k = 0; k < points.size(); k++) {
                contents_match = contents_match && points[k].point.z == static_cast<double>(entry.second[k]);
            }
        }
        ok &= check(counts_match, "each chunk file counts its owned and halo points");
        ok &= check(contents_match, "each chunk file holds its points in the order they were added");

        // A point within the halo of a chunk edge is in both chunks; one in the middle of a chunk is in one.
        std::size_t shared = 0;
        for (const auto &entry : expected) {
            shared += entry.second.size();
        }
        ok &= check(shared > count, "points near chunk edges are in more than one chunk file");

        std::set<double> owned_seen;
        std::size_t owned_total = 0;
        ok &= check(bins.for_each_owned([&](const CollectedPointCloud &owned) {
                        for (const CollectedPoint &point : owned) {
                            owned_seen.insert(point.point.z);
                        }
                        owned_total += owned.size();
                        return true;
                    }, &error), "the owned points are visited: " + error);
        ok &= check(owned_total == count && owned_seen.size() == count, "each point is visited once, by the chunk that owns it");

        bins.remove();
        ok &= check(!std::filesystem::exists(directory / "chunks"), "removing the chunk files removes their directory");
    }

    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
    std::printf(ok ? "point spill tests passed\n" : "point spill tests FAILED\n");
    return ok ? 0 : 1;
}
