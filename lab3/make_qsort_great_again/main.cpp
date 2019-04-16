#include <iostream>
#include <mpi.h>
#include "qsort.hpp"

void test_qsort() {
    std::list<int> listec{5, 3, 6, 2, 9, 1, 10, 7, 8, 4};
    auto result = algo::qsort<int>(listec);
    for(auto& element : result) {
        std::cout << element << " ";
    }
}

int main() {
    test_qsort();
    return 0;
}
