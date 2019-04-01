#include <mpi.h>
#include <iomanip>
#include <iostream>

#include "jacobi.hpp"
#include "utils.hpp"

// 1. Why char**
// 2. Why can't send
void simple_mpi_test(int arg_count, char* arg_vars[]) {
    int rank, size, len, ioerr, received_value;
    char host[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&arg_count, &arg_vars);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(host, &len);

    if(rank == 0) {
        int num = 42;
        for(int i = 1; i < size; ++i) {
            /*
            MPI_Send(
                _In_opt_ const void* buf, _In_range_(>=, 0) int count,
                _In_ MPI_Datatype datatype,
                _In_range_(>=, MPI_PROC_NULL) int dest,
                _In_range_(>=, 0) int tag, _In_ MPI_Comm comm);
            */
            MPI_Send(&num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            std::cout << "Proc 0 send " << num << " to " << i << "!"
                      << std::endl;
        }
    }
    else {
        MPI_Status status;
        // receive from any source
        // MPPI_ANY_SOURCE - or actually 0 - process who send
        /*
        MPI_Recv(
            _Out_opt_ void* buf, _In_range_(>=, 0) int count,
            _In_ MPI_Datatype datatype,
            _In_range_(>=, MPI_ANY_SOURCE) int source,
            _In_range_(>=, MPI_ANY_TAG) int tag, _In_ MPI_Comm comm,
            _Out_ MPI_Status* status);
        */
        ioerr = MPI_Recv(
            &received_value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
        if(ioerr == MPI_SUCCESS)
            std::cout << "Proc " << rank << " get message: " << received_value
                      << std::endl;
        else
            std::cout << "Proc " << rank
                      << "didn't successfully receive a value!" << std::endl;
    }

    ioerr = MPI_Finalize();

    std::cout << "MPI_Finalize: " << ioerr << std::endl;
}

void mpi_ring_network(int argc, char* argv[]) {
    int rank, size, next, prev;
    int flag; // MPI_Testall
    int buf[2];
    MPI_Request reqs[4];
    MPI_Status stats[4];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    next = rank + 1;
    prev = rank - 1;

    if(rank == 0)
        prev = size - 1;
    else if(rank == size - 1)
        next = 0;

    // Unblocking receive
    MPI_Irecv(&buf[0], 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&buf[1], 1, MPI_INT, next, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[1]);

    // MPI_Isend(
    //  _In_opt_ const void* buf, _In_range_(>=, 0) int count,
    //  _In_ MPI_Datatype datatype, _In_range_(>=, MPI_PROC_NULL) int dest,
    //  _In_range_(>=, 0) int tag, _In_ MPI_Comm comm,
    //  _Out_ MPI_Request* request);
    MPI_Isend(&rank, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &reqs[2]);
    MPI_Isend(&rank, 1, MPI_INT, next, 1, MPI_COMM_WORLD, &reqs[3]);

    MPI_Waitall(4, reqs, stats);
    // MPI_Testall(4, reqs, &flag, stats);
    // MPI_Wait(&reqs[0], &stats[0]);

    std::cout << "Proc " << rank << " communicates with " << prev << " and "
              << next << std::endl;
    /*std::cout << "Proc " << rank << " communicates with " << prev << " and "
              << next << " flag: " << flag << std::endl;*/

    MPI_Finalize();
}

// !!! expect system provides buffering

// MPI_Sendrecv(
//    _In_opt_ const void* sendbuf, _In_range_(>=, 0) int sendcount,
//    _In_ MPI_Datatype sendtype, _In_range_(>=, MPI_PROC_NULL) int dest,
//    _In_range_(>=, 0) int sendtag, _Out_opt_ void* recvbuf,
//    _In_range_(>=, 0) int recvcount, _In_ MPI_Datatype recvtype,
//    _In_range_(>=, MPI_ANY_SOURCE) int source,
//    _In_range_(>=, MPI_ANY_TAG) int recvtag, _In_ MPI_Comm comm,
//    _Out_ MPI_Status* status);

// MPI_Bcast(
//    _Pre_opt_valid_ void* buffer, _In_range_(>=, 0) int count,
//    _In_ MPI_Datatype datatype, _mpi_coll_rank_(root) int root,
//    _In_ MPI_Comm comm);

void bcast_mpi_test(int argc, char* argv[]) {
    // test with POD
    int rank, size;
    int num = 0;       // buffer
    int recv_buff = 0; // recv buff
    int num_buff[3];   // array of data
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0)
        num = 42;

    // process buffer
    std::cout << rank << ": buffer before collective communication: " << num
              << std::endl;

    // Use by ANY involved process
    // Also: MPI_Reduce
    auto ioerr = MPI_Bcast(&num, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Reduce(&num, )

    if(rank == 0) {
        std::cout << "Data " << num << " was sent";
    }
    else {
        std::cout << "Data " << num << " was received" << std::endl;
    }

    MPI_Finalize();
}

void mpi_scatter_gather(int argc, char* argv[]) {
    int size, rank;
    int datai[16] = {0, 1, 2, 3, 1, 2, 3, 4,
                     2, 3, 4, 5, 3, 4, 5, 6}; // data to transfer
    int receive_datai[4] = {0, 0, 0, 0};      // data to receive
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // process buffer
    std::cout << rank << ": buffer before collective communication: " << datai
              << std::endl;

    MPI_Scatter(
        &datai, 4, MPI_INT, &receive_datai, 4, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout << "Proc " << rank << " get :" << std::endl;
    for(int i = 0; i < 4; ++i) {
        std::cout << receive_datai[i] << std::endl;
        receive_datai[i] += rank;
    }

    MPI_Gather(
        &receive_datai, 4, MPI_INT, &datai, 4, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        std::cout << "After Gather: " << std::endl;
        for(int i = 0; i < 16; ++i)
            std::cout << datai[i] << std::endl;
    }

    MPI_Finalize();
}

void jacobi_test() {
    std::vector<double> x, input_matrix;
    std::stringstream sstr;
    filesystem::read_from_file("input_data.txt", sstr);
    filesystem::fill_matrix(sstr, input_matrix);

    auto res = algebra::yakoby_main(input_matrix, 20, 0.001);
    std::cout << "result vector" << std::endl;
    for(auto val : res) {
        std::cout << val << " ";
    }
}

int main(int argc, char* argv[]) {
    // 1. First try: MPI_Send and MPI_Recv form main process to other
    // simple_mpi_test(argc, argv);

    // 2. MPI_Bcast: one line sender
    // bcast_mpi_test(argc, argv);

    // 3. Ring network (Non-blocking communication)
    // Send your id to prev and next process
    // mpi_ring_network(argc, argv);

    // 4. Sactter | Gather
    // mpi_scatter_gather(argc, argv);

    // 5. Yakoby test
    // auto t1 = chasiki::now();
    // jacobi_test();
    // auto t2 = chasiki::now();
    // time_utils::print_time_diff(t1, t2);

    // absent of buffer in real world
    // test impl for 4 CPUs
    // algebra::mpi_jacobi(argc, argv);
    // algebra::mpi_extension::yakoby_mpi_demo(argc, argv);
    jacobi_test();

    // TODO: OpenMP multithreading
    // TODO: read from file

    // MPI_Type_contiguous

    return 0;
}
