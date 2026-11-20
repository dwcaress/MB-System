#include "settings.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace {

bool nearly_equal(double a, double b, double tolerance = 1.0e-9) {
    return std::fabs(a - b) <= tolerance;
}

bool check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        return false;
    }
    return true;
}

} // namespace

int main() {
    DatalistMetadata metadata;
    metadata.spacing.average = 0.20;
    metadata.bounds = Bounds3D(Vec3(0.0, 0.0, -1.0), Vec3(5.0, 3.0, 1.0));

    Options automatic_options;
    apply_spacing_driven_defaults(automatic_options, &metadata);

    if (!check(nearly_equal(automatic_options.level_of_detail, 0.40),
               "Automatic LOD should be twice the estimated average point spacing")) {
        return 1;
    }
    if (!check(!automatic_options.decimation.decimate,
               "Spacing-driven defaults should leave decimation disabled unless requested")) {
        return 1;
    }
    if (!check(automatic_options.normals.search_radius > 0.0,
               "Normal-estimation radius should be initialized")) {
        return 1;
    }
    if (!check(automatic_options.poisson.cell_size > 0.0,
               "Poisson cell size should be initialized")) {
        return 1;
    }
    if (!check(automatic_options.trimming.enabled,
               "Support trimming should be enabled by default")) {
        return 1;
    }

    Options requested_options;
    requested_options.level_of_detail_requested = true;
    requested_options.level_of_detail = 0.80;
    requested_options.decimation_requested = true;
    requested_options.decimation.decimate = true;
    requested_options.decimation.cell_size = 0.25;

    DatalistMetadata requested_metadata;
    requested_metadata.spacing.average = 0.20;
    requested_metadata.bounds = metadata.bounds;
    apply_spacing_driven_defaults(requested_options, &requested_metadata);

    if (!check(nearly_equal(requested_options.level_of_detail, 0.80),
               "Explicitly requested LOD should be preserved within the supported range")) {
        return 1;
    }
    if (!check(requested_options.decimation.decimate &&
                   nearly_equal(requested_options.decimation.cell_size, 0.25),
               "Requested decimation settings should be preserved")) {
        return 1;
    }
    if (!check(requested_metadata.lod_supported_by_spacing,
               "Requested LOD should be marked supported when it is large enough for point spacing")) {
        return 1;
    }

    std::string error;
    if (!check(adjust_poisson_grid_to_budget(requested_options, requested_metadata, &error),
               "Small test bounds should fit within the Poisson grid budget")) {
        return 1;
    }

    Options oversized_options = requested_options;
    oversized_options.poisson.cell_size = 0.01;
    oversized_options.poisson.padding = 0.0;

    DatalistMetadata oversized_metadata = requested_metadata;
    oversized_metadata.bounds = Bounds3D(Vec3(0.0), Vec3(1000.0));

    error.clear();
    if (!check(!adjust_poisson_grid_to_budget(oversized_options, oversized_metadata, &error),
               "Oversized grid should be rejected by the Poisson grid budget")) {
        return 1;
    }
    if (!check(!error.empty(), "Grid-budget rejection should explain the failure")) {
        return 1;
    }

    // --geoorigin resolution follows mbgrd2gltf: none by default, the given
    // lon,lat,elev when specified, otherwise the bounds center and mean elevation.
    DatalistMetadata geo_metadata;
    geo_metadata.geodetic_bounds = {-122.4, -122.2, 36.5, 36.7};
    geo_metadata.mean_elevation = -1500.0;

    Options geo_options;
    GeodeticPosition geo_origin = {1.0, 2.0, 3.0};
    if (!check(!resolve_geoorigin(geo_options, geo_metadata, &geo_origin) && geo_origin.longitude == 1.0,
               "No --geoorigin should leave absolute ECEF output and the origin untouched")) {
        return 1;
    }

    geo_options.geoorigin_mode = GeoOriginMode::Auto;
    if (!check(resolve_geoorigin(geo_options, geo_metadata, &geo_origin) &&
                   nearly_equal(geo_origin.longitude, -122.3) && nearly_equal(geo_origin.latitude, 36.6) &&
                   nearly_equal(geo_origin.elevation, -1500.0),
               "Automatic GeoOrigin should be the bounds center at the mean elevation")) {
        return 1;
    }

    geo_options.geoorigin_mode = GeoOriginMode::Specified;
    geo_options.geoorigin = {-121.9, 36.8, -10.0};
    if (!check(resolve_geoorigin(geo_options, geo_metadata, &geo_origin) &&
                   nearly_equal(geo_origin.longitude, -121.9) && nearly_equal(geo_origin.latitude, 36.8) &&
                   nearly_equal(geo_origin.elevation, -10.0),
               "Specified GeoOrigin should be used as given")) {
        return 1;
    }

    // Automatic chunking must produce enough chunks to keep every thread busy. A
    // coarse-celled survey with a modest depth range would otherwise get a few
    // huge chunks, so that extra threads have nothing to do.
    {
        DatalistMetadata big;
        big.spacing.average = 5.0;
        // A 1.2 km survey: the memory budget alone would allow one chunk, or a few,
        // since a chunk may be up to 512 cells (2.5 km) across here.
        big.bounds = Bounds3D(Vec3(-600.0, -600.0, -30.0), Vec3(600.0, 600.0, 0.0));

        Options base;
        apply_spacing_driven_defaults(base, &big);

        std::size_t previous_chunks = 0;
        for (const std::size_t threads : {1, 4, 8}) {
            Options options = base;
            options.threads = threads;
            options.max_chunks = 1000000;   // forces chunking on without fixing the chunk size
            std::string chunk_error;
            if (!check(resolve_chunking(options, big, nullptr, &chunk_error) && options.use_chunking, "Chunking should be on with an automatically chosen chunk size")) {
                return 1;
            }
            if (!check(options.chunking.threads == threads, "Chunking should use the requested thread count")) {
                return 1;
            }

            const double chunk = options.chunking.chunk_size;
            const double chunks_x = std::floor(big.bounds.max.x / chunk) - std::floor(big.bounds.min.x / chunk) + 1.0;
            const double chunks_y = std::floor(big.bounds.max.y / chunk) - std::floor(big.bounds.min.y / chunk) + 1.0;
            const std::size_t chunks = static_cast<std::size_t>(chunks_x * chunks_y);
            std::printf("  %zu threads: chunk %.0f m, %zu chunks\n", threads, chunk, chunks);

            if (!check(chunks >= 2 * threads, "Automatic chunking should give at least two chunks per thread")) {
                return 1;
            }
            if (!check(chunks >= previous_chunks, "Asking for more threads should not give fewer chunks")) {
                return 1;
            }
            previous_chunks = chunks;
        }

        // An explicit --chunk-size is honoured, not adjusted for threads.
        Options explicit_options = base;
        explicit_options.threads = 8;
        explicit_options.chunk_size_requested = true;
        explicit_options.chunk_size = 2500.0;
        std::string explicit_error;
        if (!check(resolve_chunking(explicit_options, big, nullptr, &explicit_error) &&
                       explicit_options.chunking.chunk_size >= 2500.0 &&
                       explicit_options.chunking.chunk_size < 2500.0 + base.poisson.cell_size,
                   "An explicit --chunk-size should be used as given (rounded up to whole cells)")) {
            return 1;
        }
    }

    // The point spacing estimate must not depend on how many points there are. It
    // once used 8192 points spread over the whole dataset, which overstated the
    // spacing by about sqrt(N / 8192): a million points on a 0.1 m grid looked
    // like 1.1 m. Points are in ping order, as they are read: rows of beams.
    for (const int side : {100, 1000}) {
        CollectedPointCloud grid_points;
        grid_points.reserve(static_cast<std::size_t>(side) * side);
        for (int row = 0; row < side; row++) {
            for (int column = 0; column < side; column++) {
                grid_points.push_back({Vec3(0.1 * column, 0.1 * row, 0.0), Vec3(0.1 * column, 0.1 * row, 10.0)});
            }
        }
        const SpacingMetadata spacing = estimate_point_spacing(grid_points);
        std::printf("  %d x %d points on a 0.1 m grid: estimated spacing median %.4f m, average %.4f m\n", side, side, spacing.median, spacing.average);
        if (!check(spacing.median > 0.095 && spacing.median < 0.105,
                   "Point spacing should be the true 0.1 m whatever the number of points")) {
            return 1;
        }
    }

    // The Poisson cell follows the requested level of detail. It is no finer than
    // half the point spacing, but it is not capped at the spacing, so dense data
    // can still be reconstructed at a coarser level of detail.
    {
        DatalistMetadata dense;
        dense.spacing.average = 0.02;
        dense.bounds = Bounds3D(Vec3(0.0, 0.0, -1.0), Vec3(20.0, 20.0, 1.0));

        Options coarse;
        coarse.level_of_detail_requested = true;
        coarse.level_of_detail = 0.25;
        apply_spacing_driven_defaults(coarse, &dense);
        if (!check(nearly_equal(coarse.poisson.cell_size, 0.0625),
                   "A requested level of detail of 0.25 m should give a 0.0625 m cell even when the points are 0.02 m apart")) {
            return 1;
        }

        Options fine;
        fine.level_of_detail_requested = true;
        fine.level_of_detail = 0.01;
        apply_spacing_driven_defaults(fine, &dense);
        if (!check(nearly_equal(fine.poisson.cell_size, 0.01),
                   "The cell should not be finer than half the point spacing")) {
            return 1;
        }
    }

    // With no level of detail given, a dense survey is coarsened to a feasible
    // amount of work; a requested level of detail is honored as given.
    {
        DatalistMetadata dense;
        dense.spacing.average = 0.01;
        dense.bounds = Bounds3D(Vec3(0.0, 0.0, -30.0), Vec3(150.0, 150.0, 0.0));

        Options automatic;
        apply_spacing_driven_defaults(automatic, &dense);
        const double cell = automatic.poisson.cell_size;
        const double cells = (150.0 / cell + 1.0) * (150.0 / cell + 1.0) * (30.0 / cell + 1.0);
        std::printf("  dense survey, automatic: level of detail %.3f m, cell %.4f m, about %.2e cells\n", automatic.level_of_detail, cell, cells);
        if (!check(dense.automatic_lod_before_limit > 0.0 && nearly_equal(dense.automatic_lod_before_limit, 0.02),
                   "The automatic level of detail should record that it was limited, from twice the spacing") ||
            !check(automatic.level_of_detail > 0.02, "A dense survey should get a coarser automatic level of detail") ||
            !check(cells < 6.0e8, "The automatic level of detail should keep the work within the budget")) {
            return 1;
        }

        DatalistMetadata dense_requested = dense;
        Options requested;
        requested.level_of_detail_requested = true;
        requested.level_of_detail = 0.05;
        apply_spacing_driven_defaults(requested, &dense_requested);
        if (!check(dense_requested.automatic_lod_before_limit == 0.0 && nearly_equal(requested.level_of_detail, 0.05) &&
                       nearly_equal(requested.poisson.cell_size, 0.0125),
                   "A requested level of detail should not be limited")) {
            return 1;
        }
    }

    // Chunks are sized from the relief inside a chunk, not the survey's full depth
    // range: on a slope the survey is tall but any one chunk is not.
    {
        // A 3 km by 200 m survey on a 1:10 slope: 300 m of relief overall, but only
        // about 10 m in a chunk 100 m long.
        CollectedPointCloud slope;
        for (int row = 0; row < 100; row++) {
            for (int column = 0; column < 1500; column++) {
                const double x = 2.0 * column, y = 2.0 * row;
                slope.push_back({Vec3(x, y, 0.1 * x), Vec3(x, y, 0.1 * x + 10.0)});
            }
        }
        DatalistMetadata metadata;
        metadata.spacing.average = 2.0;
        metadata.bounds = Bounds3D(Vec3(0.0, 0.0, 0.0), Vec3(2998.0, 198.0, 299.8));

        Options options;
        options.level_of_detail_requested = true;
        options.level_of_detail = 0.2;
        apply_spacing_driven_defaults(options, &metadata);
        options.max_chunks = 1000000;   // force chunking on
        options.threads = 1;            // so the chunk size is set by memory, not by the thread count

        Options from_bounds = options;
        Options from_points = options;
        std::string chunk_error;
        if (!check(resolve_chunking(from_bounds, metadata, nullptr, &chunk_error), "Chunking should resolve from the bounds") ||
            !check(resolve_chunking(from_points, metadata, &slope, &chunk_error), "Chunking should resolve from the points")) {
            return 1;
        }
        // --seam-blend: a quarter of a chunk or 1 m, whichever is less, unless given; zero is allowed.
        if (!check(std::fabs(from_bounds.chunking.seam_blend - std::min(1.0, 0.25 * from_bounds.chunking.chunk_size)) < 1.0e-12 && from_bounds.chunking.seam_blend > 0.0,
                   "the seam blend is chosen from the chunk size")) {
            return 1;
        }
        Options given = options;
        given.seam_blend_requested = true;
        given.seam_blend = 0.0;
        Options wide = options;
        wide.seam_blend_requested = true;
        wide.seam_blend = 2.5;
        if (!check(resolve_chunking(given, metadata, nullptr, &chunk_error) && given.chunking.seam_blend == 0.0, "--seam-blend 0 turns the matching off") ||
            !check(resolve_chunking(wide, metadata, nullptr, &chunk_error) && wide.chunking.seam_blend == 2.5, "--seam-blend sets the width")) {
            return 1;
        }
        std::printf("  sloping survey: chunk %.0f m from the whole depth range, %.0f m from the relief in each chunk\n",
                    from_bounds.chunking.chunk_size, from_points.chunking.chunk_size);
        if (!check(from_points.chunking.chunk_size > from_bounds.chunking.chunk_size,
                   "Measuring the relief inside chunks should allow larger chunks on a slope")) {
            return 1;
        }
    }

    // --chunk: one chunk, ranges, repeats, and what is refused.
    {
        std::set<std::pair<int, int>> chunks;
        std::string error;
        using Chunks = std::set<std::pair<int, int>>;
        if (!check(parse_chunk_selection("2,-10", &chunks, &error) && chunks == Chunks({{2, -10}}), "--chunk 2,-10 is one chunk") ||
            !check(parse_chunk_selection("2..3,-11..-10", &chunks, &error) &&
                       chunks == Chunks({{2, -10}, {2, -11}, {3, -11}, {3, -10}}),
                   "--chunk 2..3,-11..-10 adds the four chunks of the ranges to the one before it") ||
            !check(parse_chunk_selection("-1..-1,5", &chunks, &error) && chunks.count({-1, 5}) == 1 && chunks.size() == 5,
                   "a range of one chunk, with negative numbers, adds that chunk") ||
            !check(parse_chunk_selection("0..2,0", &chunks, &error) && chunks.size() == 8,
                   "repeating a chunk that is already selected does not count it twice")) {
            return 1;
        }
        for (const char *bad : {"", "1", "1,2,3", ",2", "1,", "a,b", "3..1,0", "0,2..1", "1..,0", "..1,0", "1.5,0", "0..100000,0..100000", "99999999999,0"}) {
            std::set<std::pair<int, int>> none;
            if (!check(!parse_chunk_selection(bad, &none, &error) && none.empty() && !error.empty(), (std::string("--chunk ") + bad + " is refused, and selects nothing").c_str())) {
                return 1;
            }
        }
    }

    std::printf("Settings defaults and Poisson budget checks passed\n");
    return 0;
}
