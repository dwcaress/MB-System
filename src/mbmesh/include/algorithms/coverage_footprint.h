#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

// Where the soundings of a survey are, seen from above, with the lowest and highest height of the soundings in each cell.
//
// The support trimming uses it to tell a void inside the surveyed surface (the sonar returns thin out in a depression of
// the seafloor and under an overhang) from the edge of the survey, and from a sheet of mesh that hangs under the
// surveyed surface: it asks whether there is a sounding near a point, in three dimensions, however far the soundings
// are from the chunk being reconstructed.
//
// The raster is of the whole survey, so a void that crosses the boundary of two reconstruction chunks is seen whole from
// both. It is in the x, y of the local frame, with z up.
class CoverageFootprint {
public:
    // The points are inside [min_x, max_x] x [min_y, max_y]. Cells are cell_size across, and larger if the raster would
    // have more than maximum_cells.
    CoverageFootprint(double min_x, double min_y, double max_x, double max_y, double cell_size, std::size_t maximum_cells = 16000000) {
        const double width = std::max(max_x - min_x, 0.0);
        const double height = std::max(max_y - min_y, 0.0);
        cell_ = std::max(cell_size, 1.0e-3);
        while ((width / cell_ + 1.0) * (height / cell_ + 1.0) > static_cast<double>(maximum_cells)) {
            cell_ *= 1.25;
        }
        origin_x_ = min_x;
        origin_y_ = min_y;
        nx_ = static_cast<int>(std::ceil(width / cell_)) + 1;
        ny_ = static_cast<int>(std::ceil(height / cell_)) + 1;
        const std::size_t cells = static_cast<std::size_t>(nx_) * static_cast<std::size_t>(ny_);
        lowest_.assign(cells, std::numeric_limits<float>::max());
        highest_.assign(cells, std::numeric_limits<float>::lowest());
    }

    // Keeps the lowest and highest z of the soundings in the cell of a sounding. Points outside the raster are ignored.
    void add(double x, double y, double z) {
        const int i = cell_index(x, origin_x_);
        const int j = cell_index(y, origin_y_);
        if (i >= 0 && i < nx_ && j >= 0 && j < ny_) {
            const std::size_t at = static_cast<std::size_t>(j) * nx_ + i;
            lowest_[at] = std::min(lowest_[at], static_cast<float>(z));
            highest_[at] = std::max(highest_[at], static_cast<float>(z));
        }
    }

    // Whether there is a sounding near x, y, z: a cell within the tolerance of x, y, in the horizontal plane, that holds
    // soundings and whose heights, widened by the tolerance, include z. The raster is of the whole survey, so this
    // knows the soundings of the neighboring chunks, and it knows their heights, so that a point under or over the
    // surface of the soundings, as a sheet of the mesh beyond their edge is, is not near them.
    [[nodiscard]] bool has_sounding_near(double x, double y, double z, double tolerance) const {
        const int ci = cell_index(x, origin_x_);
        const int cj = cell_index(y, origin_y_);
        const int reach = static_cast<int>(std::ceil(tolerance / cell_));
        const double limit = (tolerance / cell_ + 0.7072) * (tolerance / cell_ + 0.7072);
        for (int j = std::max(cj - reach, 0); j <= std::min(cj + reach, ny_ - 1); j++) {
            for (int i = std::max(ci - reach, 0); i <= std::min(ci + reach, nx_ - 1); i++) {
                const std::size_t at = static_cast<std::size_t>(j) * nx_ + i;
                if (lowest_[at] > highest_[at] || static_cast<double>((i - ci) * (i - ci) + (j - cj) * (j - cj)) > limit) {
                    continue;
                }
                if (z >= static_cast<double>(lowest_[at]) - tolerance && z <= static_cast<double>(highest_[at]) + tolerance) {
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] double cell_size() const { return cell_; }
    [[nodiscard]] std::size_t cell_count() const { return lowest_.size(); }

private:
    // The cell of a coordinate, or -1 if it is not a number a cell can be found for.
    [[nodiscard]] int cell_index(double value, double origin) const {
        const double cell = std::floor((value - origin) / cell_);
        if (!(cell > -1.0e9 && cell < 1.0e9)) {
            return -1;
        }
        return static_cast<int>(cell);
    }

    double cell_ = 1.0;
    double origin_x_ = 0.0;
    double origin_y_ = 0.0;
    int nx_ = 0;
    int ny_ = 0;
    std::vector<float> lowest_;    // lowest sounding of a cell, greater than highest_ for a cell without soundings
    std::vector<float> highest_;
};
