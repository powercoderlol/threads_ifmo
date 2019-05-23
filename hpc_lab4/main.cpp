#include "utils.hpp"

using chasiki = std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    size_t cluster_numbers = 15;
    size_t iteration_number = 100;
    int parallel = 0;

    points<double> storage;
    points<double> clusters;
    std::vector<size_t> assignments;

    std::string filename = argv[1];    
    parallel = std::stoi(argv[3]);
    cluster_numbers = std::stoi(argv[4]);
    iteration_number = std::stoi(argv[5]);


    read_dataset_v1(filename, storage);

    auto t1 = chasiki::now();
    auto t2 = t1;

    if(parallel) {
        omp_set_dynamic(0);
        omp_set_num_threads(parallel);
        t1 = chasiki::now();
        clusters = parallel::k_means(
            storage, assignments, cluster_numbers, iteration_number);
        t2 = chasiki::now();
    }
    else {
        t1 = chasiki::now();
        clusters = k_means(storage, assignments, cluster_numbers, iteration_number);
        t2 = chasiki::now();
    }

    filename = argv[2];
    serialize_to_python(storage, assignments, cluster_numbers, filename);

    auto duration = t2 - t1;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Execution time: " << seconds << " sec (" << milliseconds
              << " ms)" << std::endl;

    return 0;
}
