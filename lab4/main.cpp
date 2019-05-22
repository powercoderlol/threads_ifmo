#include <chrono>
#include <iostream>
#include <utility>

#include "dijkstra.hpp"
#include "utils.hpp"

using chasiki = std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    size_t start_node;
    std::vector<size_t> result;
    std::string input_filename, output_filename;
    graph_src graph;

    input_filename = argv[1];
    start_node = static_cast<size_t>(std::stoi(argv[2]));

    if(3 == argc) {
        gen_write_adjacency_matrix(start_node, input_filename);
        return 0;
    }

    output_filename = argv[3];
    auto parallel = std::stoi(argv[4]);
    if(parallel) {
        omp_set_dynamic(0);
        omp_set_num_threads(parallel);
    }
    auto write_to_file_flag = std::stoi(argv[5]);

    read_graph(input_filename, graph);

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

    if(write_to_file_flag)
        write_to_file(output_filename, std::move(result));
    //    output_filename, {result.begin(), result.begin() + graph.size()});

    return 0;
}
