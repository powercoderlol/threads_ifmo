#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "dijkstra.hpp"
#include "utils.hpp"

using chasiki = std::chrono::high_resolution_clock;

void read_graph(std::string filename, graph_src& graph) {
    std::string token;
    std::stringstream stream;
    std::vector<size_t> matrix;
    std::fstream fstr;
    size_t counter = 0;
    auto inf = std::numeric_limits<size_t>::max();

    fstr.open(filename);
    stream << fstr.rdbuf();
    fstr.close();

    while(stream >> token) {
        if(0 == counter) {
            counter = static_cast<size_t>(std::stoi(token));
            graph.resize(counter);
            continue;
        }
        if("inf" == token) {
            matrix.push_back(inf);
            continue;
        }
        matrix.push_back(static_cast<size_t>(std::stoi(token)));
    }

    for(size_t i = 0; i < counter; ++i) {
        for(size_t j = 0; j < counter; ++j) {
            auto idx = counter * i + j;
            if(matrix[idx] != inf)
                graph[i].push_back(std::move(std::make_pair(j, matrix[idx])));
        }
    }
}

void write_to_file(std::string filename, std::vector<size_t>&& paths) {
    std::ofstream fstr;
    fstr.open(filename);
    for(const auto val : paths)
        fstr << val << " ";
    fstr.close();
}

int main(int argc, char* argv[]) {
    bool read_from_file;
    int threads_num;
    size_t start_node;
    std::vector<size_t> result;
    std::string input_filename, output_filename;
    graph_src graph;

    if(3 == argc) {
        start_node = static_cast<size_t>(std::stoi(argv[1]));
        input_filename = argv[2];
        auto ok = gen_write_adjacency_matrix(start_node, input_filename);
        return 0;
    }

    read_from_file = std::stoi(argv[1]);
    auto i = 0;
    if(!read_from_file)
        i = std::stoi(argv[3]);
    else
        input_filename = argv[2];
    start_node = static_cast<size_t>(std::stoi(argv[i + 3]));
    output_filename = argv[i + 4];
    auto parallel = std::stoi(argv[i + 5]);
    if(parallel)
        threads_num = parallel;

    omp_set_dynamic(0);
    omp_set_num_threads(threads_num);

    if(read_from_file)
        read_graph(input_filename, graph);
    else
        generate_adjacency_list(threads_num * 5, graph);

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
