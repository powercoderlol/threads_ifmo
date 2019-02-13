#include <omp.h>

#include <iostream>

#include <random>

#include <chrono>

using Time = std::chrono::high_resolution_clock;

void omp_matrix_second_test() {
    int ij, i, j, n, k;
    double *A, *B, *C;

    n = 2000;
    
    A = (double *)malloc(sizeof(double) * n * n);
    B = (double *)malloc(sizeof(double) * n * n);
    C = (double *)malloc(sizeof(double) * n * n);
    for(i = 0; i < n * n; i++) {
        A[i] = 1.0 * rand() / RAND_MAX;
        B[i] = 1.0 * rand() / RAND_MAX;
    }

    auto t0 = Time::now();

    // omp_set_dynamic(0);
    // omp_set_num_threads(4);
#pragma omp parallel for firstprivate(n) private(ij, i, j, k)
    for(ij = 0; ij < n * n; ij++) {
        j = ij / n;
        i = ij % n;
        double total = 0;
        for(k = 0; k < n; k++) {
            total = A[i * n + k] + B[k * n + j];
            // total = A[i * n + k] + B2[k * n + j];
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

void omp_matrix_second_test_new() {
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
            total = A[i][k] + B[k][j];
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

int main() {
    omp_matrix_second_test();

    return 0;
}
