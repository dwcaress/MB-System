#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace mbmesh {

// For example "12.3 s", or "2 min 5.1 s" once it passes a minute.
[[nodiscard]] inline std::string format_duration(double total) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1);
    if (total >= 60.0) {
        const long minutes = static_cast<long>(total / 60.0);
        stream << minutes << " min " << (total - 60.0 * static_cast<double>(minutes)) << " s";
    } else {
        stream << total << " s";
    }
    return stream.str();
}

// Minutes with one decimal, padded to a width of five (up to 999.9) so that the
// decimal points of the durations in successive log lines line up: "  3.2 min".
[[nodiscard]] inline std::string format_minutes(double total_seconds) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << std::setw(5) << total_seconds / 60.0 << " min";
    return stream.str();
}

// Wall-clock timer for log messages.
class Stopwatch {
public:
    Stopwatch() : start_(std::chrono::steady_clock::now()) {}

    [[nodiscard]] double seconds() const {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
    }

    [[nodiscard]] std::string text() const {
        return format_duration(seconds());
    }

private:
    std::chrono::steady_clock::time_point start_;
};

} // namespace mbmesh
