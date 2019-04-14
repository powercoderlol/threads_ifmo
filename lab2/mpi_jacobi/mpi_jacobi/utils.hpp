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

template<
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
std::stringstream build_json_results(
    double* times, size_t& passed_iteration_count, T* results, size_t& length,
    int& comm_size, int precision, bool write_result = false,
    size_t output_precision = 0) {
    bool first;
    std::stringstream json;
    json << "{\"data_type\": \"" << typeid(T).name() << "\"";
    json << ", \"matrix_scale\": " << std::to_string(length);
    json << ", \"communicator_size\": " << std::to_string(comm_size);
    json << ", \"passed_iteration_count\": "
         << std::to_string(passed_iteration_count);
    json << ", \"decimal_precision_power\": " << std::to_string(precision);

    first = true;
    json << ", \"execution_times\": [";
    for(int k = 0; k < comm_size; ++k) {
        if(!first) {
            json << ", ";
        }
        json << times[k];
        if(first)
            first = false;
    }
    json << "]";

    if(write_result) {
        first = true;
        json << ", \"result_vector\": [";
        if(output_precision != 0)
            json << std::setprecision(output_precision);
        for(size_t k = 0; k < length; ++k) {
            if(!first) {
                json << ", ";
            }
            json << results[k];
            if(first)
                first = false;
        }
        json << "]";
    }
    json << "}" << std::endl;
    return json;
}

namespace filesystem {

void read_first_precision(double* input_data, std::string filename) {
    std::stringstream stream;
    std::string token;
    stream << std::ifstream(filename).rdbuf();
    size_t iterator = 0;
    while(stream >> token) {
        input_data[iterator] = stod(token);
        ++iterator;
    }
}

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
