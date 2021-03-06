#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#include <vector>

namespace utils {

void read_from_file(
    std::string m1_filename, std::string m2_filename, std::stringstream& m1,
    std::stringstream& m2) {
    m1 << std::ifstream(m1_filename).rdbuf();
    m2 << std::ifstream(m2_filename).rdbuf();
}

void fill_matrix(std::stringstream& buf, std::vector<uint32_t>& vec) {
    std::string token;
    size_t size, n, m;
    size = m = n = 0;
    while(buf >> token) {
        switch(size) {
        case 0:
            n = std::stoul(token);
            ++size;
            break;
        case 1:
            m = std::stoul(token);
            ++size;
            break;
        case 2:
            vec.reserve(n * m + 2);
            vec.push_back(stoul(token));
            ++size;
            break;
        default:
            vec.push_back(stoul(token));
            break;
        }
    }
    vec.push_back(static_cast<uint32_t>(n));
    vec.push_back(static_cast<uint32_t>(m));
}

} // namespace utils
