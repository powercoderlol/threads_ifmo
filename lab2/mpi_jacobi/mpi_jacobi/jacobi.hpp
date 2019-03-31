#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "numeric_traits.hpp"

namespace algebra {

// jacobi
// returns vector with result coeffs
// plus number of passed iterations
template<
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
std::vector<T> yakoby_main(
    std::vector<T>& input_matrix, std::vector<T>& free_ch, size_t iteration_num,
    double e = 0) {
    auto sz = free_ch.size();
    std::vector<T> prev;
    prev.resize(sz);
    std::fill(prev.begin(), prev.end(), 0);
    std::vector<T> curr;
    curr.resize(sz);
    std::fill(curr.begin(), curr.end(), 0);
    T norm, buff;
    for(size_t i = 0; i < iteration_num; ++i) {
        prev = curr;
        for(size_t j = 0; j < sz; ++j) {
            curr[j] = free_ch[j];
            for(size_t k = 0; k < sz; ++k) {
                if(j != k)
                    curr[j] -= prev[k] * input_matrix[j * sz + k];
            }
            curr[j] /= input_matrix[j * sz + j];
        }
        if(e != 0) {
            norm = fabs(prev[0] - curr[0]);
            for(size_t k = 0; k < sz; ++k) {
                buff = fabs(prev[k] - curr[k]);
                if(buff > norm)
                    norm = buff;
            }
            if(norm < e) {
                curr.push_back(i + 1);
                break;
            }
        }
    }
    return curr;
}

namespace mpi_extension {

void yakoby_mpi_demo(int argc, char* argv[]) {
    int size, id;

    /*double free_ch[4];
    double input_matrix[16];*/
    double free_ch[] = {0.76, 0.08, 1.12, 0.68};
    double input_matrix[] = {0.78,  -0.02, -0.12, -0.14, -0.02, 0.86,
                             -0.04, 0.06,  -0.12, -0.04, 0.72,  -0.08,
                             -0.14, 0.06,  -0.08, 0.74};

    double prev_coefs[] = {0, 0, 0, 0};
    double row[] = {0, 0, 0, 0};
    double received_free_coef;

    size_t iteration_num = 20;
    // double e = 0.001;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Scatter(
        &free_ch, 1, MPI_DOUBLE, &received_free_coef, 1, MPI_DOUBLE, 0,
        MPI_COMM_WORLD);
    MPI_Scatter(
        &input_matrix, 4, MPI_DOUBLE, &row, 4, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double curr;
    for(size_t i = 0; i < iteration_num; ++i) {
        curr = received_free_coef;
        for(size_t k = 0; k < 4; ++k) {
            if(k != id)
                curr -= prev_coefs[k] * row[k];
        }
        curr /= row[id];
        MPI_Allgather(
            &curr, 1, MPI_DOUBLE, &prev_coefs, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    }

    if(0 == id) {
        for(auto val : prev_coefs) {
            std::cout << val << " ";
        }
    }

    MPI_Finalize();
}

} // namespace mpi_extension

} // namespace algebra
