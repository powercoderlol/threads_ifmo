#pragma once
#include <cstdint>

#include <vector>

#include <mutex>
#include <thread>

#include <iostream>

#include "integral.hpp"

std::mutex mutex;
double result;

void thread_integrate(double a, double b, int step_count) {
    double part_integral = integrate(a, b, step_count);
    std::lock_guard<std::mutex> guard(mutex);
    result += part_integral;
}

void multithread_integral_test(
    int a, int b, size_t threads_count = 4,
    size_t iteration_count = std::numeric_limits<size_t>::max()) {
    std::vector<std::thread> threads;
    double step_length = (static_cast<double>(b - a)) / threads_count;

    time_t start_time;
    time_t finish_time;
    time(&start_time);

    for(size_t i = 0; i < threads_count; ++i) {
        threads.push_back(std::thread(
            thread_integrate, a + step_length * i, a + step_length * (i + 1),
            iteration_count / threads_count));
    }

    for(auto it = threads.begin(); it != threads.end(); ++it) {
        if((*it).joinable())
            (*it).join();
    }

    time(&finish_time);

    std::cout << "Result is: " << result
              << " Operation time: " << difftime(finish_time, start_time)
              << std::endl;

    return;
}
