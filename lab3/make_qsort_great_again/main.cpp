#include "hyperqsort.hpp"

namespace mpi = boost::mpi;

// TODO: read from file

int main(int argc, char* argv[]) {
    mpi::environment env(argc, argv);
    mpi::communicator world;
    std::vector<int> unsorted_list;

    std::string filename = "input.txt";
    if(0 == world.rank()) {
        if(argc > 1) {
            filename = argv[1];
            if(utils::mpi_lab_debug_flag)
                std::cout << filename;
        }
    }

    if(0 == world.rank()) {
        utils::read_from_file(unsorted_list, filename);
        if(utils::mpi_lab_debug_flag)
            for(const auto& val : unsorted_list)
                std::cout << val << " ";
    }

    auto list = algo::mpi_extension::hypersort(std::move(unsorted_list), world);

    unsorted_list.clear();
    std::vector<int> final_sizes_;
    final_sizes_.resize(world.size());
    mpi::gather(world, static_cast<int>(list.size()), final_sizes_, 0);

    std::vector<int> merge_;
    if(0 == world.rank())
        merge_.resize(
            std::accumulate(final_sizes_.begin(), final_sizes_.end(), 0));

    mpi::gatherv(
        world, list.data(), list.size(), merge_.data(), final_sizes_, 0);

    if(0 == world.rank()) {
        for(const auto& val : merge_)
            std::cout << val << " ";
    }

    return 0;
}
