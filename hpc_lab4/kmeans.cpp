#include "utils.hpp"

template<class T>
inline double square(T value) {
    return value * value;
}

template<class T>
inline double squared_l2_distance(
    const std::pair<T, T>& x, const std::pair<T, T>& y) {
    return sqrt(square(x.first - y.first) + square(x.second - y.second));
}

// TODO: move to template
points<double> k_means(
    const points<double>& data, std::vector<size_t>& assignments,
    size_t cluster_number, size_t iteration_number) {
    assignments.clear();
    assignments.resize(data.size());

    size_t closest_cluster = 0;
    points<double> clusters(cluster_number);
    std::vector<size_t> counters(cluster_number);

    // to get bolee luchshii random vector offset
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<size_t> distr(0, data.size() - 1);

    // set cluster as random point from dataset
    for(auto& cluster : clusters) {
        cluster = data[distr(generator)];
    }

    for(size_t iteration = 0; iteration < iteration_number; ++iteration) {
        for(size_t point_id = 0; point_id < data.size(); ++point_id) {
            auto closest_distance = std::numeric_limits<double>::max();
            closest_cluster = 0;
            for(size_t cluster_id = 0; cluster_id < cluster_number;
                ++cluster_id) {
                auto current_distance = squared_l2_distance<double>(
                    data[point_id], clusters[cluster_id]);
                if(current_distance < closest_distance) {
                    closest_distance = current_distance;
                    closest_cluster = cluster_id;
                }
            }
            assignments[point_id] = closest_cluster;
        }

        std::fill(counters.begin(), counters.end(), 0);
        points<double> new_clusters(cluster_number);

        for(size_t point_id = 0; point_id < data.size(); ++point_id) {
            auto cluster_id = assignments[point_id];
            new_clusters[cluster_id].first += data[point_id].first;
            new_clusters[cluster_id].second += data[point_id].second;
            ++counters[cluster_id];
        }

        for(size_t cluster_id = 0; cluster_id < cluster_number; ++cluster_id) {
            auto counter = std::max<size_t>(1, counters[cluster_id]);
            clusters[cluster_id].first =
                new_clusters[cluster_id].first / counter;
            clusters[cluster_id].second =
                new_clusters[cluster_id].second / counter;
        }
    }

    return clusters;
}

inline void chunk_config(int threshold, int& chunk) {
    auto threads_num = omp_get_max_threads();
    if(threads_num < threshold)
        chunk = threshold / threads_num;
    else {
        chunk = 1;
        // omp_set_num_threads(threshold);
    }
}

points<double> parallel::k_means(
    const points<double>& data, std::vector<size_t>& assignments,
    size_t cluster_number, size_t iteration_number) {
    int omp_cluster_number = static_cast<int>(cluster_number);
    int omp_storage_size = 0;
    int omp_loop = 0;
    int omp_for = 0;
    int omp_i = 0;
    int omp_j = 0;
    int chunk = 1;
    size_t closest_cluster = 0;
    size_t cluster_id = 0;
    size_t counter = 0;
    double closest_distance = 0;
    bool mode_controller = false;

    assignments.clear();
    assignments.resize(data.size());

    points<double> clusters(cluster_number);
    std::vector<size_t> counters(cluster_number);

    // to get bolee luchshii random vector offset
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<size_t> distr(0, data.size() - 1);

    // set cluster as random point from dataset
    chunk_config(omp_cluster_number, chunk);
#pragma omp parallel for schedule(dynamic, chunk) \
    firstprivate(omp_cluster_number) private(omp_for) shared(clusters)
    for(omp_for = 0; omp_for < omp_cluster_number; ++omp_for) {
        clusters[omp_for] = data[distr(generator)];
    }

    omp_storage_size = static_cast<int>(data.size());
    chunk_config(omp_storage_size, chunk);
    for(size_t iteration = 0; iteration < iteration_number; ++iteration) {
#pragma omp parallel for schedule(dynamic, chunk) firstprivate(           \
    omp_storage_size) private(closest_distance, closest_cluster, omp_for) \
    shared(assignments, data, clusters)
        for(omp_for = 0; omp_for < omp_storage_size; ++omp_for) {
            closest_distance = std::numeric_limits<double>::max();
            closest_cluster = 0;
            for(size_t cluster_id = 0; cluster_id < cluster_number;
                ++cluster_id) {
                auto current_distance = squared_l2_distance<double>(
                    data[omp_for], clusters[cluster_id]);
                if(current_distance < closest_distance) {
                    closest_distance = current_distance;
                    closest_cluster = cluster_id;
                }
            }
            assignments[omp_for] = closest_cluster;
        }

        std::fill(counters.begin(), counters.end(), 0);
        points<double> new_clusters(cluster_number);

        for(size_t point_id = 0; point_id < data.size(); ++point_id) {
            auto cluster_id = assignments[point_id];
            new_clusters[cluster_id].first += data[point_id].first;
            new_clusters[cluster_id].second += data[point_id].second;
            ++counters[cluster_id];
        }

        for(size_t cluster_id = 0; cluster_id < cluster_number; ++cluster_id) {
            auto counter = std::max<size_t>(1, counters[cluster_id]);
            clusters[cluster_id].first =
                new_clusters[cluster_id].first / counter;
            clusters[cluster_id].second =
                new_clusters[cluster_id].second / counter;
        }

        //#pragma omp parallel for schedule(dynamic, chunk)               \
//    firstprivate(omp_storage_size) private(cluster_id, omp_for) \
//        shared(assignments, counters)
        //        for(omp_for = 0; omp_for < omp_storage_size; ++omp_for) {
        //            cluster_id = assignments[omp_for];
        //            new_clusters[cluster_id].first += data[omp_for].first;
        //            new_clusters[cluster_id].second += data[omp_for].second;
        //            ++counters[cluster_id];
        //        }
        //
        //        chunk_config(omp_loop, chunk);
        //#pragma omp parallel for schedule(dynamic, chunk)              \
//    firstprivate(omp_cluster_number) private(omp_for, counter) \
//        shared(clusters)
        //        for(omp_for = 0; omp_for < omp_cluster_number; ++omp_for) {
        //            counter = std::max<size_t>(1, counters[omp_for]);
        //            clusters[omp_for].first = new_clusters[omp_for].first /
        //            counter; clusters[omp_for].second =
        //            new_clusters[omp_for].second / counter;
        //        }
    }

    return clusters;
}
