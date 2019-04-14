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
    tp t0, t1;
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
    t0 = chasiki::now();
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
    t1 = chasiki::now();
    time_utils::print_time_diff(t0, t1);
    return curr;
}

namespace mpi_extension {

inline double chasiki() {
    return MPI_Wtime();
}

// void mpi_jacobi_batches(int argc, char* argv[]) {
//    matrix_data buff;
//    size_t counters[3];
//    int id, size;
//    size_t iteration_num = 100;
//    size_t queue_length, distr_number, cellar;
//    double e = 0.001;
//    double *coefs, *prev, *curr, *curr_temp;
//    size_t* recv_repeater;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &id);
//    MPI_Comm_size(MPI_COMM_WORLD, &size);
//
//    recv_repeater = new size_t[size]();
//
//    if(argc > 1)
//        e = std::stod(argv[1]);
//
//    if(0 == id) {
//        buff = filesystem::read("input_data.txt");
//        counters[0] = buff.counters[0];
//        counters[1] = buff.counters[1];
//        counters[2] = buff.counters[2];
//    }
//
//    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);
//
//    coefs = new double[counters[0]]();
//    prev = new double[counters[1]]();
//    curr_temp = new double[counters[1]]();
//    curr = new double[counters[1]]();
//
//    if(0 == id) {
//        for(size_t k = 0; k < buff.counters[0]; ++k)
//            coefs[k] = buff.pdata[k];
//        // very smart (not)
//        delete[] buff.pdata;
//        distr_number = buff.counters[1] / size;
//        // cellar - how much rows distribute using "static queue"
//        cellar = buff.counters[1] - distr_number * size;
//        distr_number = distr_number * size;
//    }
//
//    MPI_Finalize();
//
//    delete[] recv_repeater;
//    delete[] coefs;
//    delete[] prev;
//    delete[] curr;
//}

void mpi_jacobi_scatter(int argc, char* argv[]) {
    matrix_data buff;
    size_t counters[3];
    int id, size, offset, next_row, precision = -3;
    size_t iteration_num = 100;
    size_t queue_length, final_iteration, distribution_number, matrix_offset,
        cellar;
    size_t* recv_repeater;
    size_t* cellar_repeater;
    double result, norm, norm_temp, e = 0.001;
    double *coefs, *prev, *curr, *curr_temp, *root_time, *received_buffer;
    double t0, t1;

    std::vector<std::pair<size_t, size_t>> distribution_queue;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    root_time = new double[size];
    memset(root_time, 0, size * sizeof(*root_time));

    if(0 == id) {
        if(argc > 1) {
            precision = std::stoi(argv[1]);
            e = decimal_degree(precision);
        }
        buff = filesystem::read("input_data.txt");
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
    }

    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    coefs = new double[counters[0]]();
    prev = new double[counters[1]]();
    curr_temp = new double[counters[1]]();
    // curr = new double[counters[1]]();

    if(0 == id) {
        memcpy(coefs, buff.pdata, counters[0] * sizeof(*coefs));
        delete[] buff.pdata;
    }

    int batch_power = counters[1] / size;
    distribution_number = size * batch_power;
    cellar = counters[1] - distribution_number;

    received_buffer = new double[counters[2] * batch_power];
    cellar_repeater = new size_t[size];

    if(0 == id) {
        size_t cellar_queue_length = cellar;
        for(size_t k = 0; k < cellar_queue_length; ++k) {
            auto send_rank = k - size * (k / size);
            // return constexpr?
            distribution_queue.push_back(
                std::make_pair(send_rank, distribution_number + 1 + k));
            // ++cellar_repeater[send_rank];
        }
    }

    bool flag = true;
    int send_recv_count = counters[2] * batch_power;
    MPI_Scatter(
        coefs, send_recv_count, MPI_DOUBLE, received_buffer, send_recv_count,
        MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // Scatter cellar

    // 0. Send extra rows ???
    // 1. Jacobi calculator
    // NB: distribution_number + cellar_repeater[id] - how much rows we have in
    // curr processor
    size_t current_repeat_counter =
        distribution_number; // + cellar_repeater[id];
    curr = new double[current_repeat_counter];
    for(size_t p = 0; p < current_repeat_counter; ++p) {
        // get last free member
        curr[p] = received_buffer[p * counters[2]];
        // get current offset
        int border = batch_power * size + p;
        for(size_t t = 0; t < counters[1]; ++t) {
            // if != row_id
            if(border != t)
                curr[p] += received_buffer[p * t];
        }
        curr[p] /= curr[border];
    }

    // 2. share cellar through recv_repeater
    // 3 AllGather...

    // do {
    //    int send_recv_count = counters[2] * batch_power;
    //    MPI_Scatter(
    //        coefs, send_recv_count, MPI_DOUBLE, received_buffer,
    //        send_recv_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //    flag = false;
    //} while(flag);

    // debug output
    /*if(0 == id) {
        std::cout << build_json_results(
                         root_time, final_iteration, curr_temp, counters[1],
                         size, precision, false)
                         .rdbuf();
    }*/

    MPI_Finalize();

    //    delete[] recv_repeater;
    delete[] coefs;
    delete[] prev;
    delete[] curr;
    delete[] cellar_repeater;
    delete[] received_buffer;

    return;
} // namespace mpi_extension

void mpi_jacobi(int argc, char* argv[]) {
    matrix_data buff;
    size_t counters[3];
    int id, size, offset, next_row, precision = -3;
    size_t iteration_num = 100;
    size_t queue_length;
    size_t* recv_repeater;
    double result, norm, norm_temp, e = 0.001;
    double *coefs, *prev, *curr, *curr_temp, *root_time;
    double t0, t1;
    bool print_result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    print_result = atoi(argv[4]);

    root_time = new double[size];
    memset(root_time, 0, size * sizeof(*root_time));

    if(0 == id) {
        if(argc > 1) {
            precision = std::stoi(argv[1]);
            e = decimal_degree(precision);
        }
        buff = filesystem::read(argv[2]);
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
    }

    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    coefs = new double[counters[0]]();
    prev = new double[counters[1]]();
    curr_temp = new double[counters[1]]();
    curr = new double[counters[1]]();

    if(0 == id) {
        filesystem::read_first_precision(prev, argv[3]);
        memcpy(coefs, buff.pdata, counters[0] * sizeof(*coefs));
        delete[] buff.pdata;
    }

    // huge
    // but MAY BE better than send row on each occasion
    MPI_Bcast(coefs, counters[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(prev, counters[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    recv_repeater = new size_t[size]();

    // static distribution, chunk = 1 row
    //
    // recv_repeater[id] - how much rows
    // distributed for proc with id
    //
    // Example:
    // 1. matrix 6x6
    // 2. communicator size = 4
    //
    // for each iteration while norm >= e:
    // 1 proc: 1 and 5 rows
    // 2 proc: 2 and 6 rows
    // 3 proc: 3 row
    // 4 proc: 4 row
    //
    // end of iteration for proc 0:
    // curr[ X1, 0, 0, 0, X5, 0 ]
    // where X1 and X5 - calculated values
    //
    // after reduce in all processes:
    // prev[ X1, X2, X3, X4, X5, X6 ]
    // where X1..X6 - calculated values by all
    //
    // Problem:
    // if all proc tasks finished
    // it can't get ready task
    // not according for id in communicator
    //
    // Solution: maintain real queue by root process
    //
    // Expected side-effects: performance degradation
    // because of additional communications
    queue_length = counters[1];
    if(0 == id) {
        for(size_t k = 0; k < queue_length; ++k) {
            // for matrix 10x10; size = 4; proc 1:
            // queue_length = 10
            // k = 1
            // send_rank = 1 - 4 * ( 1 / 4 ) = 1
            // k = 5
            // send_rank = 5 - 4 * ( 5 / 4 ) = 1
            // k = 9
            // send_rank = 9 - 4 * ( 9 / 4) = 1
            // recv_repeater[1] = 3
            auto send_rank = k - size * (k / size);
            ++recv_repeater[send_rank];
        }
    }

    MPI_Bcast(recv_repeater, size, my_MPI_SIZE_T, 0, MPI_COMM_WORLD);

    t0 = MPI_Wtime();
    result = 0;
    offset = 0;
    next_row = 0;
    norm = 0;
    norm_temp = 0;
    size_t final_iteration = iteration_num;
    int exit_flag = 0;
    for(size_t i = 0; i < iteration_num; ++i) {
        memset(curr, 0, counters[1] * sizeof(*curr));
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
                    final_iteration = i + 1;
                    exit_flag = 1;
                }
            }
        }
        MPI_Bcast(&exit_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(exit_flag == 1)
            break;
        memcpy(prev, curr_temp, counters[1] * sizeof(*prev));
    }
    t1 = MPI_Wtime();

    auto time_diff = t1 - t0;

    MPI_Gather(
        &time_diff, 1, MPI_DOUBLE, root_time, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // debug output
    if(0 == id) {
        std::cout << build_json_results(
                         root_time, final_iteration, curr_temp, counters[1],
                         size, precision, print_result)
                         .rdbuf();
    }

    MPI_Finalize();

    delete[] recv_repeater;
    delete[] coefs;
    delete[] prev;
    delete[] curr;
    delete[] curr_temp;
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

    // get epsilon
    if(argc > 1)
        e = std::stod(argv[1]);

    reqs = new MPI_Request[size];
    stats = new MPI_Status[size];
    if(0 == id) {
        buff = filesystem::read("input_data.txt");
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
    }

    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    coefs = new double[counters[0]]();
    received_buff = new double[counters[2]]();
    prev = new double[counters[1]]();
    curr_temp = new double[counters[1]]();
    curr = new double[counters[1]]();

    if(0 == id) {
        for(size_t k = 0; k < buff.counters[0]; ++k)
            coefs[k] = buff.pdata[k];
        // very smart (not)
        delete[] buff.pdata;
        distr_number = buff.counters[1] / size;
        cellar = buff.counters[1] - distr_number * size;
    }

    MPI_Bcast(coefs, counters[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);

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
