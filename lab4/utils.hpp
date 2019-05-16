#pragma once
#include <boost/interprocess/managed_mapped_file.hpp>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace bi = boost::interprocess;

using graph_src = std::vector<std::vector<std::pair<size_t, size_t>>>;

using pair_allocator = bi::allocator<std::pair<size_t, size_t>, bi::managed_mapped_file::segment_manager>;

using graph_allocator = bi::allocator<
    std::vector<std::pair<size_t, size_t>, pair_allocator>,
    bi::managed_mapped_file::segment_manager>;

using graph_src_mmap =
    std::vector<std::vector<std::pair<size_t, size_t>>, graph_allocator>;

enum class read_mode { REGULAR, MMAP };

bool gen_write_adjacency_matrix(
    size_t nodes_number, std::string filename, size_t min_border = 500,
    size_t max_border = 1000);

bool generate_adjacency_list(
    size_t nodes_number, graph_src& adj_list, size_t min_border = 500,
    size_t max_border = 1000);

bool gen_adj_list_write_mmap(
    size_t nodes_number, std::string filename, size_t min_border = 500,
    size_t max_border = 1000);

void write_to_file(std::string filename, std::vector<size_t>&& paths);

void read_graph(std::string filename, graph_src& graph);

bool read_graph_from_mmap(std::string filename, graph_src& graph);
