#pragma once
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using graph_src = std::vector<std::vector<std::pair<size_t, size_t>>>;

bool gen_write_adjacency_matrix(
    size_t nodes_number, std::string filename, size_t min_border = 500,
    size_t max_border = 1000);

bool generate_adjacency_list(
    size_t nodes_number, graph_src& adj_list, size_t min_border = 500,
    size_t max_border = 1000);
