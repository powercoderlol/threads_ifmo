#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "dijkstra.hpp"
#include "utils.hpp"

using chasiki = std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    read_mode read_mode_;
    int threads_num;
    size_t start_node;
    std::vector<size_t> result;
    std::string input_filename, output_filename;
    graph_src graph;

    read_mode_ = static_cast<read_mode>(std::stoi(argv[1]));
    input_filename = argv[2];

    if(4 == argc) {        
        start_node = static_cast<size_t>(std::stoi(argv[3]));
        if(read_mode::REGULAR == read_mode_)
            gen_write_adjacency_matrix(start_node, input_filename);
        if(read_mode::MMAP == read_mode_)
            gen_adj_list_write_mmap(start_node, input_filename);
        return 0;
    }

    output_filename = argv[3];
    start_node = static_cast<size_t>(std::stoi(argv[4]));

    auto parallel = std::stoi(argv[5]);

    if(parallel) {
        threads_num = parallel;
        omp_set_dynamic(0);
        omp_set_num_threads(threads_num);
    }

    if(read_mode::REGULAR == read_mode_) {
        read_graph(input_filename, graph);
    }

    if(read_mode::MMAP == read_mode_) {
        read_graph_from_mmap(input_filename, graph);
        return 0;
    }

    try {
        graph.at(start_node);
        auto t1 = chasiki::now();
        if(parallel)
            result = parallel::dijkstra_shortest_path(graph, start_node);
        else
            result = dijkstra_shortest_path(graph, start_node);
        auto t2 = chasiki::now();

        auto duration = t2 - t1;
        auto seconds =
            std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                .count();
        std::cout << "Execution time: " << seconds << " sec (" << milliseconds
                  << " ms)" << std::endl;
    }
    catch(std::out_of_range& e) {
        std::cout << e.what()
                  << ": start node MUST BE\na) less than or equal to graph "
                     "size.\nb) Be positive";
        return 0;
    }

    write_to_file(output_filename, std::move(result));
    //    output_filename, {result.begin(), result.begin() + graph.size()});

    return 0;
}
