#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "dijkstra.hpp"

void read_graph(std::string filename, graph_src& graph) {
    std::string token;
    std::stringstream stream;
    std::vector<size_t> matrix;
    size_t counter = 0;
    auto inf = std::numeric_limits<size_t>::max();

    auto fstr = std::ifstream(filename);
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
    auto fstr = std::ofstream(filename);
    for(const auto val : paths)
        fstr << val << " ";
    fstr.close();
}

int main(int argc, char* argv[]) {
    graph_src graph;
    std::vector<size_t> result;

    if(argc < 4)
        return 0;

    std::string input_filename = argv[1];
    size_t start_node = static_cast<size_t>(std::stoi(argv[2]));
    std::string output_filename = argv[3];

    read_graph(input_filename, graph);

    try {
        graph.at(start_node);
        result = dijkstra_shortest_path(graph, start_node);
    }
    catch(std::out_of_range& e) {
        std::cout << e.what()
                  << ": start node MUST BE\na) less than or equal to graph "
                     "size.\nb) Be positive";
        return 0;
    }

    write_to_file(
        output_filename, std::move(result));
    //    output_filename, {result.begin(), result.begin() + graph.size()});

    return 0;
}
