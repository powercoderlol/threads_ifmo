#include <iostream>
#include "mt_utils.hpp"

int main() {
    std::cout << "Hello concurrent world!" << std::endl;
    multithread_integral_test(0, 1);

    return 0;
}
