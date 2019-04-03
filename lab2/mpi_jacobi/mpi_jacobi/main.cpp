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

int main(int argc, char* argv[]) {
    algebra::mpi_extension::fancy_mpi_jacobi(argc, argv);
    return 0;
}
