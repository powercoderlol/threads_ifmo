#pragma once
#include <vector>

using std::vector;

using graph_src = vector<vector<std::pair<size_t, size_t>>>;
using marks = vector<bool>;

vector<size_t> dijkstra_shortest_path(const graph_src& g, size_t start_node) {
    auto n = g.size();
    auto INF = std::numeric_limits<size_t>::max();

    vector<size_t> distances(n, INF);
    // vector<size_t> parents(n);
    marks marked(n, false);

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
