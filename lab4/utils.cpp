#include "utils.hpp"

bool gen_write_adjacency_matrix(
    size_t nodes_number, std::string filename, size_t min_border = 500,
    size_t max_border = 1000) {
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
    size_t nodes_number, graph_src& adj_list, size_t min_border = 500,
    size_t max_border = 1000) {
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
