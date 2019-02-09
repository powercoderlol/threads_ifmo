#include <iostream>
#include "mt_utils.hpp"

int main(int argc, char* argv[]) {
    std::cout << "Hello concurrent world!" << std::endl;
    uint32_t thread_count = 4;
	uint64_t iteration_count = std::numeric_limits<uint64_t>::max();
    if(argc == 2)
        thread_count = static_cast<uint64_t>(atoi(argv[1]));
	else if (argc == 3) {
		thread_count = static_cast<uint64_t>(atoi(argv[1]));
		iteration_count = static_cast<uint64_t>(atoi(argv[2]));
	}

    multithread_integral_test(0, 1, thread_count, iteration_count);

    return 0;
}
