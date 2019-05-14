#pragma once

//*********************************
//* Dijkstra algorithm O(n^2 + m) using OpenMP
//* Input data is adjacency list
//*********************************

#include <omp.h>

#include <algorithm>
#include <vector>

using std::vector;

using graph_src = vector<vector<std::pair<size_t, size_t>>>;

inline void chunk_config(int threshold, int& chunk) {
    auto threads_num = omp_get_max_threads();
    if(threads_num < threshold)
        chunk = threshold / threads_num;
    else {
        chunk = threshold;
        omp_set_num_threads(threshold);
    }
}

namespace parallel {

vector<size_t> dijkstra_shortest_path(
    const graph_src& g, size_t start_node, bool parallel = true) {
    int k, j, chunk;
    size_t path_len, len, to, links_size, curr_vertex;

    auto n = g.size();
    auto INF = std::numeric_limits<size_t>::max();

    chunk_config(n, chunk);

    vector<bool> marked(n, false);
    vector<size_t> distances(n, INF);
    vector<size_t> mins(omp_get_max_threads(), INF);
    vector<size_t> ids(omp_get_max_threads(), n + 1);

    // vector<size_t> parents(n);

    distances[start_node] = 0;

    for(size_t i = 0; i < n; ++i) {
        std::fill(mins.begin(), mins.end(), INF);
        std::fill(ids.begin(), ids.end(), n + 1);
#pragma omp parallel for schedule(dynamic, chunk) \
    shared(marked, distances, mins, ids)          \
        firstprivate(n) private(curr_vertex, j) if(parallel)
        for(j = 0; j < n; ++j) {
            curr_vertex = ids[omp_get_thread_num()];
            if(!marked[j]
               && (curr_vertex == n + 1
                   || distances[j] < distances[curr_vertex])) {
                ids[omp_get_thread_num()] = j;
                mins[omp_get_thread_num()] = distances[j];
            }
        }

        curr_vertex =
            ids[std::min_element(mins.begin(), mins.end()) - mins.begin()];

        if(distances[curr_vertex] == INF)
            break;
        marked[curr_vertex] = true;

        links_size = g[curr_vertex].size();
        chunk_config(links_size, chunk);

#pragma omp parallel for firstprivate(curr_vertex, links_size) private( \
    to, len, path_len, k) schedule(dynamic, chunk)                      \
    shared(g, distances) if(parallel)
        for(k = 0; k < links_size; ++k) {
            to = g[curr_vertex][k].first;
            len = g[curr_vertex][k].second;
            path_len = distances[curr_vertex] + len;
            if(path_len < distances[to]) {
                distances[to] = path_len;
                // parents[to] = curr_vertex;
            }
        }
    }
    // distances.reserve(2 * n);
    // distances.insert(distances.end(), parents.begin(), parents.end());
    return distances;
}
} // namespace parallel

vector<size_t> dijkstra_shortest_path(const graph_src& g, size_t start_node) {
    auto n = g.size();
    auto INF = std::numeric_limits<size_t>::max();

    vector<size_t> distances(n, INF);
    // vector<size_t> parents(n);
    std::vector<bool> marked(n, false);

    distances[start_node] = 0;

    for(size_t i = 0; i < n; ++i) {
        auto curr_vertex = n + 1;
        for(size_t j = 0; j < n; ++j)
            if(!marked[j]
               && (curr_vertex == n + 1
                   || distances[j] < distances[curr_vertex]))
                curr_vertex = j;
        if(distances[curr_vertex] == INF)
            break;
        marked[curr_vertex] = true;

        auto links_size = g[curr_vertex].size();
        for(size_t k = 0; k < links_size; ++k) {
            auto to = g[curr_vertex][k].first;
            auto len = g[curr_vertex][k].second;
            auto path_len = distances[curr_vertex] + len;
            if(path_len < distances[to]) {
                distances[to] = path_len;
                // parents[to] = curr_vertex;
            }
        }
    }
    // distances.reserve(2 * n);
    // distances.insert(distances.end(), parents.begin(), parents.end());
    return distances;
}
