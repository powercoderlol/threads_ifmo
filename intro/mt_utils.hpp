#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "integral.hpp"

using Time = std::chrono::high_resolution_clock;

std::mutex mutex;
double result;

void thread_integrate(double a, double b, int step_count) {
    double part_integral = integrate(a, b, step_count);
    std::lock_guard<std::mutex> guard(mutex);
    result += part_integral;
}

void multithread_integral_test(
    int a, int b, uint32_t threads_count = 4,
    uint64_t iteration_count = std::numeric_limits<uint64_t>::max()) {
    int step_count = static_cast<int>(iteration_count / threads_count);
    std::vector<std::thread> threads;
    double step_length = (static_cast<double>(b - a)) / threads_count;

    auto t0 = Time::now();

    for(size_t i = 0; i < threads_count; ++i) {
        threads.push_back(std::thread(
            thread_integrate, a + step_length * i, a + step_length * (i + 1),
            step_count));
    }

    std::for_each(
        threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    auto t1 = Time::now();
    auto duration = t1 - t0;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Result is: " << result << " Execution time: " << seconds
              << " sec (" << milliseconds << " ms)" << std::endl;

    return;
}
