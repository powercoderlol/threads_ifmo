#include "hyperqsort.hpp"

namespace mpi = boost::mpi;

using utils::chasiki;
using utils::tp;

int main(int argc, char* argv[]) {
    mpi::environment env(argc, argv);
    mpi::communicator world;
    std::vector<int> unsorted_list;
    std::list<int> unsorted_list_;
    mpi::timer timer;

    std::string filename = "input.txt";
    if(0 == world.rank()) {
        if(argc > 1) {
            filename = argv[1];
            if(utils::mpi_lab_debug_flag)
                std::cout << filename;
        }
    }

    auto opt = std::stoi(argv[2]);
    if(0 == world.rank()) {
        if(1 == opt) {
            utils::read_from_file(unsorted_list_, filename);
            auto res_list = algo::qsort(unsorted_list_);
            for(const auto& val : res_list)
                std::cout << val << " ";
            return 0;
        }
        utils::read_from_file(unsorted_list, filename);
        if(utils::mpi_lab_debug_flag)
            for(const auto& val : unsorted_list)
                std::cout << val << " ";
    }
    else if(1 == opt)
        return 0;

    if(1 == world.size()) {
        if(0 == world.rank()) {
            std::sort(unsorted_list.begin(), unsorted_list.end());
            for(const auto& val : unsorted_list)
                std::cout << val << " ";
        }
        return 0;
    }

    auto t1 = timer.elapsed();
    double tfin = 0;
    auto list = std::move(
        algo::mpi_extension::hypersort(std::move(unsorted_list), world));
    auto t2 = timer.elapsed();
    t1 = t2 - t1;    

    mpi::reduce(world, t1, tfin, mpi::maximum<double>(), 0);

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
        std::cout << "Execution time: " << tfin << std::endl;
        for(const auto& val : merge_)
            std::cout << val << " ";
    }

    return 0;
}
