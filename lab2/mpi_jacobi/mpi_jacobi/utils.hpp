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

struct matrix_data {
    size_t counters[3];
    double* pdata;
    matrix_data() {
        memset(counters, 0, sizeof(counters));
    }
};

namespace filesystem {

matrix_data read(std::string filename) {
    std::string token;
    matrix_data data;
    std::stringstream stream;
    stream << std::ifstream(filename).rdbuf();
    while(stream >> token) {
        switch(data.counters[0]) {
        case 0:
            data.counters[1] = std::stoul(token);
            ++data.counters[0];
            break;
        case 1:
            data.counters[2] = std::stoul(token);
            ++data.counters[0];
            break;
        case 2:
            /*data.pdata = (double*)malloc(
                sizeof(double) * (data.counters[1] * data.counters[2]));*/
            data.pdata = new double[data.counters[1] * data.counters[2]];
            data.pdata[data.counters[0] - 2] = stod(token);
            ++data.counters[0];
            break;
        default:
            data.pdata[data.counters[0] - 2] = stod(token);
            ++data.counters[0];
            break;
        }
    }
    data.counters[0] = data.counters[0] - 2;
    /*data.pdata[data.counters[0] - 2] = data.counters[1];
    data.pdata[data.counters[0] - 1] = data.counters[2];
    data.pdata[data.counters[0]] = data.counters[0] - 2;*/
    return data;
}

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
