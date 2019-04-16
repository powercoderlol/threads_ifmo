#pragma once
#include <iostream>
namespace utils {

constexpr bool mpi_debug = true;

void print_array(int arr[], size_t size) {
    if(mpi_debug) {
        for(size_t k = 0; k < size; ++k) {
            std::cout << arr[k] << " ";
        }
        std::cout << std::endl;
    }
}
} // namespace utils
