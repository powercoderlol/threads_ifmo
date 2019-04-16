#pragma once

#include <boost/mpi.hpp>
#include <boost/mpi/allocator.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>

#include "qsort.hpp"

#define _ITERATOR_DEBUG_LEVEL 0

namespace algo {
namespace mpi_extension {

namespace mpi = boost::mpi;

std::vector<int> hypersort_internal(
    std::vector<int> list, mpi::communicator comm) {
    int pivot = 0;

    if(1 == comm.size()) {
        return list;
    }

    if(0 == comm.rank())
        pivot = *list.begin();

    mpi::broadcast(comm, pivot, 0);

    auto divider = std::partition(
        list.begin(), list.end(),
        [&](const int& value) { return value < pivot; });

    std::vector<int> low, up;
    low.insert(low.begin(), list.begin(), divider);
    up.insert(up.begin(), divider, list.end());

    auto partner = (comm.rank() + comm.size() / 2) % comm.size();

    // it works...
    std::vector<int> c;
    size_t recv_size;
    if(comm.rank() < partner) {
        comm.recv(partner, 0, recv_size);
        c.resize(recv_size);
        comm.recv(partner, 0, c.data(), recv_size);
        comm.send(partner, 0, up.size());
        comm.send(partner, 0, up.data(), up.size());
        list = std::move(low);
        list.insert(list.end(), c.begin(), c.end());
    }
    else {
        comm.send(partner, 0, low.size());
        comm.send(partner, 0, low.data(), low.size());
        comm.recv(partner, 0, recv_size);
        c.resize(recv_size);
        comm.recv(partner, 0, c.data(), recv_size);
        list = std::move(up);
        list.insert(list.begin(), c.begin(), c.end());
    }

    std::sort(list.begin(), list.end());

    mpi::communicator new_comm;
    if(partner > comm.rank())
        new_comm = comm.split(0, comm.rank());
    else
        new_comm = comm.split(1, comm.rank());

    return hypersort_internal(std::move(list), new_comm);
}

std::vector<int> hypersort(
    std::vector<int> unsorted_list, mpi::communicator comm) {
    std::vector<int> sizes_;
    sizes_.resize(comm.size() - 1);

    auto list_size = unsorted_list.size();
    auto distribution_number = list_size / comm.size();
    auto cellar = list_size % comm.size();

    std::fill(sizes_.begin(), sizes_.end(), distribution_number);
    sizes_.push_back(cellar + distribution_number);

    std::vector<int> list;

    if(comm.size() - 1 != comm.rank())
        list.resize(distribution_number);
    else
        list.resize(distribution_number + cellar);

    mpi::scatterv(comm, unsorted_list.data(), sizes_, list.data(), 0);

    return hypersort_internal(std::move(list), comm);
}

} // namespace mpi_extension
} // namespace algo
