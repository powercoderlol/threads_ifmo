#pragma once
#include <omp.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

template<class T>
using points = std::vector<std::pair<T, T>>;

void write_large_file_clusters(
    std::string filename, const points<double>& clusters);

void read_dataset_v1(std::string filename, points<double>& storage);

void serialize_to_python(
    const points<double>& storage, const std::vector<size_t>& assignments,
    size_t cluster_number, std::string filename);

template<class T>
double square(T value);

template<class T>
double squared_l2_distance(
    const std::pair<T, T>& first, const std::pair<T, T>& second);

points<double> k_means(
    const points<double>& data, std::vector<size_t>& assignments,
    size_t cluster_number, size_t iteration_number);

namespace parallel {
points<double> k_means(
    const points<double>& data, std::vector<size_t>& assignments,
    size_t cluster_number, size_t iteration_number);
}
