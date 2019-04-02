#include <mpi.h>
#include <iomanip>
#include <iostream>

#include "jacobi.hpp"
#include "utils.hpp"

#include <Windows.h>

void jacobi_test() {
    std::vector<double> input_matrix;
    std::stringstream sstr;
    filesystem::read_from_file("input_data.txt", sstr);
    filesystem::fill_matrix(sstr, input_matrix);

    auto res = algebra::yakoby_main(input_matrix, 20, 0.001);
    std::cout << "result vector" << std::endl;
    for(auto val : res) {
        std::cout << val << " ";
    }
}

void test() {
    char buffer[MAX_PATH];
    int bytes = GetModuleFileName(NULL, buffer, MAX_PATH);
    std::cout << buffer;
}

int main(int argc, char* argv[]) {
    matrix_data buff;
    int counters[3];
    int id, size;
    size_t iteration_num = 20;
    double e = 0.001;
    double *coefs, *received_buff, *prev, *curr;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(0 == id) {
        buff = filesystem::read("input_data.txt");
        counters[0] = buff.counters[0];
        counters[1] = buff.counters[1];
        counters[2] = buff.counters[2];
        // size_t platform depenent
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    // MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&counters, 3, MPI_INT, 0, MPI_COMM_WORLD);

    /*std::cout << "proc " << id << " received: " << std::endl;
    for(size_t k = 0; k < 3; ++k)
        std::cout << counters[k] << " ";*/

    /*coefs = (double*)malloc(sizeof(double) * counters[0]);
received_buff = (double*)malloc(sizeof(double) * counters[0]);*/

    coefs = new double[counters[0]];
    received_buff = new double[counters[0]];

    if(0 == id) {
        for(size_t k = 0; k < buff.counters[0]; ++k)
            coefs[k] = buff.pdata[k];
    }

    // MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(coefs, counters[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // MPI_Barrier(MPI_COMM_WORLD);

    std::cout << "proc " << id << " received: " << std::endl;
    for(size_t k = 0; k < counters[0]; ++k)
        std::cout << coefs[k] << " ";

    MPI_Finalize();

    return 0;
}
