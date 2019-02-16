#pragma once

#include <iostream>

#include <omp.h>

#include <random>

#include <chrono>

using Time = std::chrono::high_resolution_clock;

namespace openmp_matrix {

namespace utils {

template<class Ò>
std::string to_string(const Ò&t) {
    std::ostringstream os;
    os << t;
    return os.str();
}

void generate_data(std::string filename, int x, int y) {
    int ij, i, j;
    std::vector<double> vec;
    vec.resize(static_cast<size_t>(x * y));
    std::string str;
	int matrix_scale = x * y;

#pragma omp parallel for schedule(static, 2) \
    firstprivate(matrix_scale, x, y) private(ij, i, j)
    for(ij = 0; ij < matrix_scale; ++ij) {
        j = ij / x;
        i = ij % y;
        vec[i + j * x] = 1.0 * rand() / RAND_MAX;
    }
    std::stringstream sstr;
    sstr << to_string(x) << " " << to_string(y) << std::endl;
    uint8_t counter = 0;
	uint8_t border = x - 1;
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

template<class T>
void omp_matrix_test_vector(
    std::vector<T> &matrix_n, std::vector<T> &matrix_m) {
    int ij, i, j, n, k;
    std::vector<T> result;

    int size_n = static_cast<int>(matrix_n.back());
    matrix_n.pop_back();
    int size_m = static_cast<int>(matrix_n.back());
    matrix_n.pop_back();
    matrix_m.pop_back();
    matrix_m.pop_back();

    auto multisize = size_n * size_m;
    result.resize(multisize);

    auto t0 = Time::now();

    omp_set_dynamic(0);
    omp_set_num_threads(8);
#pragma omp parallel for schedule(static, 2) firstprivate( \
    size_m, size_n, multisize) private(ij, i, j, k) if(size_n * size_m > 150)
    for(ij = 0; ij < multisize; ++ij) {
        j = ij / size_n;
        i = ij % size_m;
        T total = 0;
        for(k = 0; k < size_n; ++k) {
            total += matrix_m[i * size_m + k] * matrix_n[k * size_n + j];
        }
        result[i * size_n + j] = total;
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

void omp_matrix_test() {
    int ij, i, j, n, k;
    double *A, *B, *C;

    n = 3;

    A = (double *)malloc(sizeof(double) * n * n);
    B = (double *)malloc(sizeof(double) * n * n);
    C = (double *)malloc(sizeof(double) * n * n);
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

void omp_matrix_test_new() {
    int ij, i, j, n, k;
    double **A, **B, **C;

    n = 2000;

    A = new double *[n];
    B = new double *[n];
    C = new double *[n];
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

} // namespace openmp_matrix
