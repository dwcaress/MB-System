#pragma once

#include <algorithm>
#include <functional>
#include <thread>
#include <vector>

namespace mbmesh {

// Calls body(begin, end) on consecutive pieces of [first, last), one per thread, the calling thread taking the first. The
// threads run at the same time and are joined before this returns; with one thread, or little to do, it is a plain call.
// The pieces do not overlap, so a body that writes only to the elements of its own range needs no lock (but not to the
// bits of a std::vector<bool>, which share bytes).
inline void parallel_for_ranges(long first, long last, int threads, const std::function<void(long, long)> &body) {
    const long total = std::max(0L, last - first);
    const long count = std::max(1L, std::min<long>(threads, total));
    if (count <= 1) {
        body(first, last);
        return;
    }
    std::vector<std::thread> helpers;
    helpers.reserve(static_cast<std::size_t>(count) - 1);
    for (long t = 1; t < count; t++) {
        helpers.emplace_back(body, first + total * t / count, first + total * (t + 1) / count);
    }
    body(first, first + total / count);
    for (std::thread &helper : helpers) {
        helper.join();
    }
}

} // namespace mbmesh
