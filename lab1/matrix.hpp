#pragma once

#include <omp.h>

#include <iostream>

#include <random>

#include <chrono>

#include <array>

using Time = std::chrono::high_resolution_clock;

namespace openmp_matrix {

namespace utils {

template<class Enum>
constexpr typename std::underlying_type<Enum>::type enum_to_int(
    Enum v) noexcept {
    return static_cast<typename std::underlying_type<Enum>::type>(v);
}

enum class schedule_type { static_t, dynamic_t, guided_t };
static std::array<std::string, 3> schedule_traits{"static", "dynamic",
                                                  "guided"};

template<class T>
std::string to_string(const T& t) {
    std::ostringstream os;
    os << t;
    return os.str();
}

void generate_data(std::string filename, int x, int y) {
    int ij, i, j;
    std::vector<uint32_t> vec;
    vec.resize(static_cast<size_t>(x * y));
    std::string str;
    int matrix_scale = x * y;

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<uint32_t> distr(1, 500);

#pragma omp parallel for schedule(static, 1) \
    firstprivate(matrix_scale, x, y) private(ij, i, j)
    for(ij = 0; ij < matrix_scale; ++ij) {
        j = ij / y;
        i = ij % y;
        vec[i * x + j] = distr(generator);
    }
    std::stringstream sstr;
    sstr << to_string(x) << " " << to_string(y) << std::endl;
    uint8_t counter = 0;
    uint8_t border = y - 1;
    for(auto num : vec) {
        sstr << to_string(num);
        if(counter == border) {
            sstr << std::endl;
            counter = 0;
        }
        else {
            sstr << " ";
            ++counter;
        }
    }

    std::ofstream file;
    file.open(filename);
    file << sstr.rdbuf();
    file.close();
}
} // namespace utils

void omp_matrix_test_vector(
    std::vector<uint32_t>& matrix_n, std::vector<uint32_t>& matrix_m,
    utils::schedule_type schedule_t =
        openmp_matrix::utils::schedule_type::static_t,
    int chunks = 1, int threads_num = omp_get_max_threads(),
    bool write_result = false) {
    omp_set_dynamic(0);
    omp_set_num_threads(threads_num);
    int ij, i, j, k;
    std::vector<uint32_t> result;

    int columns_n = static_cast<int>(matrix_n.back());
    matrix_n.pop_back();
    int rows_n = static_cast<int>(matrix_n.back());
    matrix_n.pop_back();
    int columns_m = static_cast<int>(matrix_m.back());
    matrix_m.pop_back();
    int rows_m = static_cast<int>(matrix_m.back());
    matrix_m.pop_back();

    if(columns_n != rows_m) {
        std::cout << "can't multiple matrices" << std::endl;
        return;
    }

    auto multisize = rows_n * columns_m;
    result.resize(multisize);

    /*
    for(ij = 0; ij < multisize; ++ij) {
        j = ij / rows_n;
        i = ij % rows_n;
        uint32_t total = 0;
        for(k = 0; k < columns_n; ++k) {
            total += matrix_n[i * columns_n + k] * matrix_m[k * columns_m + j];
        }
        result[i * columns_m + j] = total;
    }
    */

    auto t0 = Time::now();

    if(schedule_t == utils::schedule_type::static_t) {
#pragma omp parallel for schedule(static, chunks)                          \
    firstprivate(rows_m, rows_n, columns_m, columns_n, multisize) private( \
        ij, i, j, k) if(multisize >= 22500)
        for(ij = 0; ij < multisize; ++ij) {
            j = ij / rows_n;
            i = ij % rows_n;
            uint32_t total = 0;
            for(k = 0; k < columns_n; ++k) {
                total +=
                    matrix_n[i * columns_n + k] * matrix_m[k * columns_m + j];
            }
            result[i * columns_m + j] = total;
        }
    }

    if(schedule_t == utils::schedule_type::dynamic_t) {
#pragma omp parallel for schedule(dynamic, chunks)                         \
    firstprivate(rows_m, rows_n, columns_m, columns_n, multisize) private( \
        ij, i, j, k) if(multisize >= 22500)
        for(ij = 0; ij < multisize; ++ij) {
            j = ij / rows_n;
            i = ij % rows_n;
            uint32_t total = 0;
            for(k = 0; k < columns_n; ++k) {
                total +=
                    matrix_n[i * columns_n + k] * matrix_m[k * columns_m + j];
            }
            result[i * columns_m + j] = total;
        }
    }

    if(schedule_t == utils::schedule_type::guided_t) {
#pragma omp parallel for schedule(guided, chunks)                          \
    firstprivate(rows_m, rows_n, columns_m, columns_n, multisize) private( \
        ij, i, j, k) if(multisize >= 22500)
        for(ij = 0; ij < multisize; ++ij) {
            j = ij / rows_n;
            i = ij % rows_n;
            uint32_t total = 0;
            for(k = 0; k < columns_n; ++k) {
                total +=
                    matrix_n[i * columns_n + k] * matrix_m[k * columns_m + j];
            }
            result[i * columns_m + j] = total;
        }
    }

    auto t1 = Time::now();

    auto duration = t1 - t0;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::string json;
    json.append("{\"number_of_threads\": " + std::to_string(threads_num));
    json.append(
        ", \"schedule_type\": \""
        + utils::schedule_traits[enum_to_int(schedule_t)]);
    // json.append(
    //    ", \"schedule_type\": "
    //    + std::to_string(utils::enum_to_int(schedule_t)));
    json.append("\", \"number_of_chunks\": " + std::to_string(chunks));
    json.append(", \"execution_time\": " + std::to_string(milliseconds));
    json.append(
        ", \"columns\": " + std::to_string(columns_m)
        + ", \"rows\": " + std::to_string(rows_n) + ", \"result\": [");
    if(write_result) {
        bool first = true;
        for(auto num : result) {
            if(!first)
                json.append(", ");
            json.append(std::to_string(num));
            if(first)
                first = false;
        }
    }
    json.append("]}");

    std::cout << json << std::endl;
}

void omp_matrix_test() {
    int ij, i, j, n, k;
    double *A, *B, *C;

    n = 3;

    A = (double*)malloc(sizeof(double) * n * n);
    B = (double*)malloc(sizeof(double) * n * n);
    C = (double*)malloc(sizeof(double) * n * n);
    for(i = 0; i < n * n; i++) {
        A[i] = 1.0 * rand() / RAND_MAX;
        B[i] = 1.0 * rand() / RAND_MAX;
    }

    auto t0 = Time::now();

    omp_set_dynamic(0);
    omp_set_num_threads(8);
#pragma omp parallel for schedule(static, 2) \
    firstprivate(n) private(ij, i, j, k) if(n > 150)
    for(ij = 0; ij < n * n; ij++) {
        j = ij / n;
        i = ij % n;
        double total = 0;
        for(k = 0; k < n; k++) {
            total += A[i * n + k] * B[k * n + j];
        }
        C[i * n + j] = total;
    }

    auto t1 = Time::now();

    auto duration = t1 - t0;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Execution time: " << seconds << " sec (" << milliseconds
              << " ms)" << std::endl;
}

/*
void omp_matrix_test_new() {
    int ij, i, j, n, k;
    double **A, **B, **C;

    n = 2000;

    A = new double*[n];
    B = new double*[n];
    C = new double*[n];
    for(size_t i = 0; i < n; ++i) {
        A[i] = new double[n];
        B[i] = new double[n];
        C[i] = new double[n];
    }

    for(size_t i = 0; i < n; ++i) {
        for(size_t j = 0; j < n; ++j) {
            A[i][j] = 1.0 * rand() / RAND_MAX;
            B[i][j] = 1.0 * rand() / RAND_MAX;
        }
    }

    auto t0 = Time::now();

#pragma omp parallel for firstprivate(n) private(ij, i, j, k)
    for(ij = 0; ij < n * n; ij++) {
        j = ij / n;
        i = ij % n;
        double total = 0;
        for(k = 0; k < n; k++) {
            total += A[i][k] * B[k][j];
        }
        C[i][j] = total;
    }
    auto t1 = Time::now();

    auto duration = t1 - t0;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Execution time: " << seconds << " sec (" << milliseconds
              << " ms)" << std::endl;
}
*/

} // namespace openmp_matrix
