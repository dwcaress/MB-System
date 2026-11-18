#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../algorithms/coverage_footprint.h"
#include "../data_types/geometry.h"

/*******************************************************************************
 * Soundings on disk, for surveys that are too large to hold in memory.
 *
 * Two files are involved. The reader writes each accepted sounding to a spill
 * file as it is read (PointSpill), so that memory does not grow with the number
 * of soundings. Once the reconstruction frame, the point spacing and the chunk
 * size are known, the points are read back in pieces and sorted into one file
 * per reconstruction chunk (ChunkBins), each chunk file holding the points of the
 * chunk and of the halo around it, so that a chunk is reconstructed from its own
 * file and only the chunks being worked on are in memory.
 *
 * The files are temporary and private to one run, in the host byte order. A chunk
 * file record is the bytes of a CollectedPoint (two Vec3 of doubles, 48 bytes).
 * A spill file record is smaller, 28 bytes: the position of the sounding as
 * three doubles and the number of its ping, since the sensor position is the
 * same for all the soundings of a ping and is kept once, in the track.
 ******************************************************************************/

static_assert(std::is_trivially_copyable<CollectedPoint>::value, "CollectedPoint is written to disk as raw bytes");

// Bytes of one record in a chunk file, and in the spill file.
constexpr std::size_t spilled_point_bytes = sizeof(CollectedPoint);
constexpr std::size_t spill_record_bytes = 3 * sizeof(double) + sizeof(std::uint32_t);

// Sequential writer of the spill file.
class PointSpill {
public:
    PointSpill() = default;
    PointSpill(const PointSpill &) = delete;
    PointSpill &operator=(const PointSpill &) = delete;
    ~PointSpill();

    // Creates (or empties) the file.
    [[nodiscard]] bool open(const std::filesystem::path &path, std::string *error);
    [[nodiscard]] bool is_open() const { return file_ != nullptr; }

    // ping is the index of the sounding's ping in the list of sensor positions that the points are read back with.
    [[nodiscard]] bool append(const Vec3 &point, std::uint32_t ping, std::string *error);

    // Writes what is buffered and closes the file.
    [[nodiscard]] bool finish(std::string *error);

    [[nodiscard]] std::size_t count() const { return count_; }
    [[nodiscard]] const std::filesystem::path &path() const { return path_; }

private:
    bool flush(std::string *error);

    std::FILE *file_ = nullptr;
    std::filesystem::path path_;
    std::vector<unsigned char> buffer_;   // whole records
    std::size_t count_ = 0;
};

// Reads a spill file in pieces of at most piece_points points, calling the callback with each piece and the index in
// the file of its first point. The point of each is as it was written, and its origin is origins[ping]. The callback may
// change the piece. It returns false to stop, which is not an error.
using SpillPieceCallback = std::function<bool(CollectedPointCloud &piece, std::size_t first_index)>;
[[nodiscard]] bool read_spill_pieces(const std::filesystem::path &path, const std::vector<Vec3> &origins, std::size_t piece_points,
                                     const SpillPieceCallback &callback, std::string *error);

// Reads count points starting at index first (fewer, with an error, if the file is shorter).
[[nodiscard]] bool read_spill_range(const std::filesystem::path &path, const std::vector<Vec3> &origins, std::size_t first, std::size_t count,
                                    CollectedPointCloud *points, std::string *error);

// Calls visit(i, j, owned) for each chunk of this size whose points include a point at x, y: the chunk that owns it
// and every neighbor whose halo reaches it. Chunk (i, j) owns x in [i*chunk, (i+1)*chunk) and y in [j*chunk, (j+1)*chunk).
// The in-memory chunker and the on-disk one both use this, so that they put every point in the same chunks.
template <typename Visit>
void for_each_chunk_of_point(double x, double y, double chunk, double halo, Visit &&visit) {
    const int i_first = static_cast<int>(std::floor((x - halo) / chunk)) - 1;
    const int i_last = static_cast<int>(std::floor((x + halo) / chunk)) + 1;
    const int j_first = static_cast<int>(std::floor((y - halo) / chunk)) - 1;
    const int j_last = static_cast<int>(std::floor((y + halo) / chunk)) + 1;

    for (int i = i_first; i <= i_last; i++) {
        if (x < i * chunk - halo || x >= (i + 1) * chunk + halo) {
            continue;
        }
        for (int j = j_first; j <= j_last; j++) {
            if (y < j * chunk - halo || y >= (j + 1) * chunk + halo) {
                continue;
            }
            const bool owned = x >= i * chunk && x < (i + 1) * chunk && y >= j * chunk && y < (j + 1) * chunk;
            visit(i, j, owned);
        }
    }
}

// How many points a chunk file holds.
struct ChunkBinCount {
    std::size_t owned = 0;   // inside the chunk
    std::size_t total = 0;   // with the halo
};

// One file of points for each reconstruction chunk.
class ChunkBins {
public:
    // The files go in directory. If keep_only_chunks() is called, only those chunks are kept (--chunk, for testing).
    // memory_points bounds how many points are buffered in memory before they are appended to the files.
    ChunkBins(const std::filesystem::path &directory, double chunk_size, double halo, std::size_t memory_points = 4000000);
    ChunkBins(const ChunkBins &) = delete;
    ChunkBins &operator=(const ChunkBins &) = delete;
    ~ChunkBins();

    // Only these chunks are kept (--chunk, for testing); all of them if the set is empty.
    void keep_only_chunks(const std::set<std::pair<int, int>> &chunks) { only_chunks_ = chunks; }

    // Adds every point to the footprint (which must outlive the adds), whether or not
    // keep_only_chunks() drops it.
    void set_footprint(CoverageFootprint *footprint) { footprint_ = footprint; }

    // Creates the directory, and empties it of earlier chunk files.
    [[nodiscard]] bool open(std::string *error);
    [[nodiscard]] bool add(const CollectedPoint &point, std::string *error);
    // Writes what is buffered. No more points can be added.
    [[nodiscard]] bool finish(std::string *error);

    [[nodiscard]] double chunk_size() const { return chunk_size_; }
    [[nodiscard]] double halo() const { return halo_; }
    [[nodiscard]] const std::map<std::pair<int, int>, ChunkBinCount> &chunks() const { return counts_; }
    // Points owned by chunks: each point of the survey once.
    [[nodiscard]] std::size_t owned_point_count() const;

    // The points of chunk (i, j), its halo included, in the order they were added. Safe to call from several threads.
    [[nodiscard]] bool read_chunk(int i, int j, CollectedPointCloud *points, std::string *error) const;

    // Calls the callback with the points each chunk owns, one chunk at a time, so that every point is visited once.
    // It returns false to stop, which is not an error.
    [[nodiscard]] bool for_each_owned(const std::function<bool(const CollectedPointCloud &)> &callback, std::string *error) const;

    // Deletes the chunk files and the directory.
    void remove();

private:
    [[nodiscard]] std::filesystem::path chunk_path(int i, int j) const;
    [[nodiscard]] bool flush(std::string *error);

    std::filesystem::path directory_;
    double chunk_size_;
    double halo_;
    std::size_t memory_points_;
    std::set<std::pair<int, int>> only_chunks_;
    CoverageFootprint *footprint_ = nullptr;
    std::map<std::pair<int, int>, ChunkBinCount> counts_;
    std::map<std::pair<int, int>, CollectedPointCloud> buffers_;
    std::size_t buffered_ = 0;
};
