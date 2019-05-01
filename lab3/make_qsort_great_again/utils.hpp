#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define _ITERATOR_DEBUG_LEVEL 0

namespace utils {

constexpr bool mpi_lab_debug_flag = false;

using std::string;
using std::stringstream;

void read_from_file(std::vector<int>& list, string filename) {
    string token;
    stringstream stream;
    stream << std::ifstream(filename).rdbuf();
    while(stream >> token)
        list.push_back(std::stoi(token));
}

} // namespace utils
