#pragma once

#include <chrono>

#include <fstream>
#include <iostream>
#include <sstream>

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

namespace filesystem {

void read_from_file(std::string m1_filename, std::stringstream& m1) {
    m1 << std::ifstream(m1_filename).rdbuf();
}

template<
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
void fill_matrix(std::stringstream& buf, std::vector<T>& vec) {
    std::string token;
    size_t size, n, m;
    size = m = n = 0;
    while(buf >> token) {
        switch(size) {
        case 0:
            n = std::stoul(token);
            ++size;
            break;
        case 1:
            m = std::stoul(token);
            ++size;
            break;
        case 2:
            vec.reserve(n * m + 2);
            vec.push_back(stod(token));
            ++size;
            break;
        default:
            vec.push_back(std::stod(token));
            break;
        }
    }
    vec.push_back(static_cast<T>(n));
    vec.push_back(static_cast<T>(m));
}
} // namespace filesystem
