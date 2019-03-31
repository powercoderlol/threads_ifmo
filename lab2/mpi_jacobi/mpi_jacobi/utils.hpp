#pragma once

#include <chrono>
#include <iostream>

using chasiki = std::chrono::high_resolution_clock;
using tp = chasiki::time_point;

namespace time_utils {
void print_time_diff(tp& t0, tp& t1) {
    auto duration = t1 - t0;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Execution time: " << seconds << " sec (" << milliseconds
              << " ms)" << std::endl;
}
} // namespace time_utils
