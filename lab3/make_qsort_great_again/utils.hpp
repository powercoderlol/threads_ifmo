#pragma once
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace utils {

constexpr bool mpi_lab_debug_flag = false;

using chasiki = std::chrono::high_resolution_clock;
using tp = chasiki::time_point;

using std::string;
using std::stringstream;

void read_from_file(std::vector<int>& list, string filename) {
    string token;
    stringstream stream;
    stream << std::ifstream(filename).rdbuf();
    while(stream >> token)
        list.push_back(std::stoi(token));
}

void read_from_file(std::list<int>& list, string filename) {
    string token;
    stringstream stream;
    stream << std::ifstream(filename).rdbuf();
    while(stream >> token)
        list.push_back(std::stoi(token));
}

} // namespace utils
