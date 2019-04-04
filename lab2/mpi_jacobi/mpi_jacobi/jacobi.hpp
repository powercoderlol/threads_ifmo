#pragma once

#include <limits.h>
#include <stdint.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "numeric_traits.hpp"

#include "utils.hpp"

#if SIZE_MAX == UCHAR_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
#define my_MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#endif

namespace algebra {

// jacobi
// returns vector with result coeffs
// plus number of passed iterations if passed < iteration_num
template<
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
std::vector<T> yakoby_main(
    std::vector<T>& input_matrix, size_t iteration_num, double e = 0) {
    T norm, buff;
    size_t offset, matrix_dimension;
    std::vector<T> prev, curr;
    // get offset
    offset = static_cast<size_t>(input_matrix.back());
    input_matrix.pop_back();
    input_matrix.pop_back();
    // get row length (actually offset - 1)
    // matrix_dimension = static_cast<size_t>(input_matrix.back());
    matrix_dimension = offset - 1;
    // fill initial data
    curr.resize(matrix_dimension, 0);
    prev.resize(matrix_dimension, 0);
    for(size_t i = 0; i < iteration_num; ++i) {
        prev = curr;
        for(size_t j = 0; j < matrix_dimension; ++j) {
            curr[j] = input_matrix[j * offset + matrix_dimension];
            for(size_t k = 0; k < matrix_dimension; ++k) {
                if(j != k)
                    curr[j] -= prev[k] * input_matrix[j * offset + k];
            }
            curr[j] /= input_matrix[j * offset + j];
        }
        if(e != 0) {
            norm = fabs(prev[0] - curr[0]);
            for(size_t k = 0; k < matrix_dimension; ++k) {
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

void mpi_jacobi(int argc, char* argv[]) {
    matrix_data buff;
    size_t counters[3];
    int id, size;
    size_t iteration_num = 100;
    size_t queue_length;
    double e = 0.001;
    double *coefs, *received_buff, *prev, *curr, *curr_temp;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(0 == id) {
        buff = filesystem::read("input_data.txt");
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
    }

    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    coefs = new double[counters[0]];
    received_buff = new double[counters[2]];
    prev = new double[counters[1]];
    curr_temp = new double[counters[1]];
    curr = new double[counters[1]];
    for(size_t k = 0; k < counters[1]; ++k) {
        curr[k] = 0;
        prev[k] = 0;
        curr_temp[k] = 0;
    }

    if(0 == id) {
        for(size_t k = 0; k < buff.counters[0]; ++k)
            coefs[k] = buff.pdata[k];
        delete[] buff.pdata;
    }

    MPI_Bcast(coefs, counters[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    size_t* recv_repeater = new size_t[size];
    for(int sz = 0; sz < size; ++sz)
        recv_repeater[sz] = 0;

    // static distribution, chunk = 1 row
    queue_length = counters[1];
    if(0 == id) {
        for(size_t k = 0; k < queue_length; ++k) {
            auto send_rank = k - size * (k / size);
            ++recv_repeater[send_rank];
        }
    }

    MPI_Bcast(recv_repeater, size, my_MPI_SIZE_T, 0, MPI_COMM_WORLD);

    double result = 0;
    int offset = 0;
    int next_row = 0;
    double norm = 0;
    double norm_temp = 0;
    size_t final_iteration = iteration_num;
    int exit_flag = 0;
    for(size_t i = 0; i < iteration_num; ++i) {
        for(size_t k = 0; k < counters[1]; ++k) {
            curr[k] = 0;
        }
        for(size_t k = 0; k < recv_repeater[id]; ++k) {
            // offset for static distribution
            // 0 - second iteration => 4
            offset = id + size * k;
            // row length * CPU size * distribution count
            next_row = counters[2] * offset;
            result = coefs[next_row + counters[1]];
            for(size_t n = 0; n < counters[1]; ++n) {
                if(n != offset)
                    result -= prev[n] * coefs[next_row + n];
            }
            result /= coefs[next_row + offset];
            curr[offset] = result;
            result = 0;
        }
        MPI_Allreduce(
            curr, curr_temp, counters[1], MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        if(0 == id) {
            if(e != 0) {
                norm = fabs(prev[0] - curr_temp[0]);
                for(size_t k = 0; k < counters[1]; ++k) {
                    norm_temp = fabs(prev[k] - curr_temp[k]);
                    if(norm_temp > norm)
                        norm = norm_temp;
                }
                if(norm < e) {
                    final_iteration = i;
                    exit_flag = 1;
                }
            }
        }
        MPI_Bcast(&exit_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(exit_flag == 1)
            break;
        for(size_t n = 0; n < counters[1]; ++n)
            prev[n] = curr_temp[n];
    }

    // debug output
    // TODO: json output for graphics
    // TODO: add time points
    if(0 == id) {
        std::cout << "final iteration num: " << final_iteration << std::endl;
        std::cout << "final result: " << std::endl;
        for(size_t k = 0; k < counters[1]; ++k)
            std::cout << curr_temp[k] << " ";
    }

    MPI_Finalize();

    delete[] recv_repeater;
    delete[] coefs;
    delete[] received_buff;
    delete[] prev;
    delete[] curr;
}

void fancy_mpi_jacobi(int argc, char* argv[]) {
    matrix_data buff;
    size_t counters[3];
    int id, size;
    size_t iteration_num = 20;
    size_t distr_number, cellar, queue_length;
    double e = 0.001;
    double *coefs, *received_buff, *prev, *curr, *curr_temp;
    MPI_Request* reqs;
    MPI_Status* stats;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    reqs = new MPI_Request[size];
    stats = new MPI_Status[size];

    if(0 == id) {
        buff = filesystem::read("input_data.txt");
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
    }

    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    coefs = new double[counters[0]];
    received_buff = new double[counters[2]];
    prev = new double[counters[1]];
    curr_temp = new double[counters[1]];
    curr = new double[counters[1]];
    // memset(curr, 0, sizeof(curr));
    // memset(prev, 0, sizeof(prev));
    for(size_t k = 0; k < counters[1]; ++k) {
        curr[k] = 0;
        prev[k] = 0;
        curr_temp[k] = 0;
    }

    if(0 == id) {
        for(size_t k = 0; k < buff.counters[0]; ++k)
            coefs[k] = buff.pdata[k];
        // very smart (not)
        delete[] buff.pdata;
        distr_number = buff.counters[1] / size;
        cellar = buff.counters[1] - distr_number * size;
    }

    MPI_Bcast(coefs, counters[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int test_arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t recv_repeater[4] = {0, 0, 0, 0};
    // memset(recv_repeater, 0, sizeof(recv_repeater));

    // static distribution, chunk = 1 row
    queue_length = counters[1];
    if(0 == id) {
        for(size_t k = 0; k < queue_length; ++k) {
            // auto send_rank = (k > size - 1) ? k - size * (k / size) : k;
            auto send_rank = k - size * (k / size);
            ++recv_repeater[send_rank];
        }
    }
    MPI_Bcast(&recv_repeater, 4, my_MPI_SIZE_T, 0, MPI_COMM_WORLD);
    double result = 0;
    double norm = 0;
    double norm_buff = 0;
    int flag = 0;
    // MPI JACOBI CALCULATION USING STATIC DISTRIBUTION "QUEUE"
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for(size_t i = 0; i < 20; ++i) {
        for(size_t k = 0; k < counters[1]; ++k) {
            // prev[k] = curr[k];
            curr[k] = 0;
        }
        if(0 == id) {
            size_t temp = 0;
            for(size_t k = 0; k < queue_length; ++k) {
                auto send_rank =
                    ((int)k > size - 1) ? k - size * (k / size) : k;
                if(send_rank != id)
                    MPI_Send(
                        coefs + k * counters[2], counters[2], MPI_DOUBLE,
                        send_rank, 0, MPI_COMM_WORLD);
                else {
                    for(size_t j = 0; j < counters[2]; ++j) {
                        received_buff[j] = coefs[k * counters[2] + j];
                    }
                    result = received_buff[counters[1]];
                    // 4 - number of CPUs (Processes)
                    auto offset = id + 4 * temp;
                    for(size_t j = 0; j < counters[1]; ++j) {
                        if(j != offset)
                            result -= received_buff[j] * prev[j];
                    }
                    result /= received_buff[offset];
                    curr[offset] = result;
                    if(temp < recv_repeater[id])
                        ++temp;
                }
            }
        }

        for(size_t k = 0; k < recv_repeater[id]; ++k) {
            if(0 != id) {
                MPI_Recv(
                    received_buff, counters[2], MPI_DOUBLE, 0, MPI_ANY_TAG,
                    MPI_COMM_WORLD, &stats[0]);
                result = received_buff[counters[1]];
                // 4 - number of CPUs (Processes)
                auto offset = id + 4 * k;
                for(size_t j = 0; j < counters[1]; ++j) {
                    if(j != offset)
                        result -= received_buff[j] * prev[j];
                }
                result /= received_buff[offset];
                curr[offset] = result;
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Allreduce(
            curr, prev, counters[1], MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(flag == 1) {
            break;
        }
        // MPI_Bcast(curr, counters[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // MPI JACOBI CALCULATION USING STATIC DISTRIBUTION "QUEUE"
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // debug output
    if(0 == id) {
        std::cout << "final result: " << std::endl;
        for(size_t k = 0; k < counters[1]; ++k)
            std::cout << prev[k] << " ";
    }

    MPI_Finalize();

    delete[] coefs;
    delete[] received_buff;
    delete[] prev;
    delete[] curr;
}
} // namespace mpi_extension

} // namespace algebra
