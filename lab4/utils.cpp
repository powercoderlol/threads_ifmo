#include "utils.hpp"

constexpr int MMAP_FILE_SIZE = 2147483648;

bool gen_write_adjacency_matrix(
    size_t nodes_number, std::string filename, size_t min_border,
    size_t max_border) {
    std::ofstream fstr;
    bool ok = false;

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<size_t> distr(min_border, max_border);
    std::uniform_int_distribution<size_t> probability(0, 1);

    std::vector<std::vector<size_t>> matrix(nodes_number);
    for(auto& intra : matrix)
        intra.resize(nodes_number);

    auto inf = std::numeric_limits<size_t>::max();

    for(size_t i = 0; i < nodes_number; ++i) {
        for(size_t j = i; j < nodes_number; ++j) {
            if(j == i) {
                matrix[i][j] = 0;
                continue;
            }
            auto prob = probability(generator);
            auto path_len = inf;
            if(prob)
                path_len = distr(generator);
            matrix[i][j] = matrix[j][i] = path_len;
        }
    }

    fstr.open(filename);
    if(fstr) {
        ok = true;
        fstr << nodes_number << std::endl;
        for(const auto& inner : matrix) {
            for(auto val : inner) {
                if(val == inf) {
                    fstr << "inf ";
                    continue;
                }
                fstr << val << " ";
            }
            fstr << std::endl;
        }
        fstr.close();
    }

    return ok;
}

bool generate_adjacency_list(
    size_t nodes_number, graph_src& adj_list, size_t min_border,
    size_t max_border) {
    auto inf = std::numeric_limits<size_t>::max();

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<size_t> distr(min_border, max_border);
    std::uniform_int_distribution<size_t> probability(0, 1);

    adj_list.clear();
    adj_list.resize(nodes_number);

    for(size_t i = 0; i < nodes_number; ++i) {
        for(size_t j = i; j < nodes_number; ++j) {
            if(j == i) {
                adj_list[i].push_back(std::make_pair(i, 0));
                continue;
            }
            auto prob = probability(generator);
            auto path_len = inf;
            if(prob) {
                path_len = distr(generator);
            }
            adj_list[i].push_back(std::make_pair(j, path_len));
            adj_list[j].push_back(std::make_pair(i, path_len));
        }
    }

    return false;
}

bool gen_adj_list_write_mmap(
    size_t nodes_number, std::string filename, size_t min_border,
    size_t max_border) {
    auto inf = std::numeric_limits<size_t>::max();

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<size_t> distr(min_border, max_border);
    std::uniform_int_distribution<size_t> probability(0, 1);

    // let it be 2GB
    bi::managed_mapped_file graph_file_storage(
        bi::open_or_create, filename.c_str(), 1000);

    graph_src_mmap* mapped_graph =
        graph_file_storage.find_or_construct<graph_src_mmap>("adj_graph")(
            graph_file_storage.get_segment_manager());

    mapped_graph->resize(nodes_number);

    for(size_t i = 0; i < nodes_number; ++i) {
        for(size_t j = i; j < nodes_number; ++j) {
            if(j == i) {
                mapped_graph->at(i).push_back(std::make_pair(i, 0));
                continue;
            }
            auto prob = probability(generator);
            auto path_len = inf;
            if(prob) {
                path_len = distr(generator);
            }
            mapped_graph->at(i).push_back(std::make_pair(j, path_len));
            mapped_graph->at(j).push_back(std::make_pair(i, path_len));
        }
    }

    return true;
}

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

bool read_graph_from_mmap(std::string filename, graph_src& graph) {
    bi::managed_mapped_file graph_file_storage(
        bi::open_or_create, filename.c_str(), MMAP_FILE_SIZE);

    auto graph_new = graph_file_storage.find_or_construct<graph_src_mmap>(
        "adj_graph")(graph_file_storage.get_segment_manager());

    std::cout << graph_new->size();

    for(size_t k = 0; k < 10; ++k) {
        auto internal = graph_new->at(k);
        std::cout << "[ " << k << " ] = ";
        for(auto para : internal)
            std::cout << "< " << para.first << ", " << para.second << " > ";
        std::cout << std::endl;
    }

    return true;
}

void write_to_file(std::string filename, std::vector<size_t>&& paths) {
    std::ofstream fstr;
    fstr.open(filename);
    for(const auto val : paths)
        fstr << val << " ";
    fstr.close();
}
