#include <mpi.h>
#include <time.h>

#include <fstream>
#include <iostream>

int* read(char* filename) {
    int numbers;
    int* buff;
    std::ifstream file(filename);
    if(file.is_open()) {
        file >> numbers;
        buff = (int*)malloc(sizeof(int) * numbers);
        for(size_t k = 0; k < (size_t)numbers; ++k)
            file >> buff[k + 1];
        buff[0] = numbers;
    }
    return buff;
}

template<class T>
void qsort(T* a, int N) {
    long i = 0, j = N;
    T temp, p;

    p = a[0];

    do {
        while(a[i] < p)
            i++;
        while(a[j] > p)
            j--;

        if(i <= j) {
            temp = a[i];
            a[i] = a[j];
            a[j] = temp;
            i++;
            j--;
        }
    } while(i <= j);

    if(j > 0)
        qsort(a, j);
    if(N > i)
        qsort(a + i, N - i);
}

inline void swap(int* l, int* r) {
    *l = (*r + *l);
    *r = *l - *r;
    *l = *l - *r;
}

int partition(int* a, int pivot, int r) {
    int i = 0;
    int j = r - 1;
    while(i <= j) {
        while(a[i] < pivot)
            i++;
        while(a[j] > pivot)
            j--;
        if(i <= j) {
            int temp = a[i];
            a[i] = a[j];
            a[j] = temp;
            i++;
            j--;
        }
    }
    return (i > r ? r : i);
}

int* distribution_deduction(MPI_Comm comm, int elements_number) {
    int size;
    int* sizes;
    MPI_Comm_size(comm, &size);

    sizes = (int*)malloc(sizeof(int) * size);
    memset(sizes, 0, sizeof(int) * size);

    int distribution_number = elements_number / size;
    int cellar = elements_number % size;

    while(cellar) {
        for(size_t k = 0; k < size; ++k) {
            if(cellar) {
                ++sizes[k];
                --cellar;
            }
            else
                break;
        }
    }

    if(distribution_number > 0) {
        for(size_t k = 0; k < size; ++k)
            sizes[k] += distribution_number;
    }

    return sizes;
}

void concat_arrays(int* one, int* two) {
}

int* hypersort(int* before_partition_array, MPI_Comm comm) {
    int id, size;
    int partner;
    int pivot;
    int uber_part_length = 0;
    int low_part_length = 0;
    int recv_uber_part = 0;
    int recv_low_part = 0;
    int new_size = 0;

    int* my_new_array = NULL;
    int* low = NULL;
    int* uber = NULL;
    int* recv_low = NULL;
    int* recv_uber = NULL;

    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &id);

    if(size == 1) {
        return before_partition_array;
    }

    MPI_Request* request = new MPI_Request[size];
    MPI_Status* status = new MPI_Status[size];

    int array_length = before_partition_array[0];
    ++before_partition_array;

    if(0 == id) {
        if(array_length > 0)
            pivot = before_partition_array[0];
        else
            pivot = INT_MIN;
    }
    MPI_Bcast(&pivot, 1, MPI_INT, 0, comm);

    int offset = partition(before_partition_array, pivot, array_length);

    partner = (id + size / 2) % size;

    if(offset > 0) {
        low = before_partition_array;
        low_part_length = offset;
        uber = before_partition_array + offset;
        uber_part_length = array_length - offset;
    }
    else if(offset == 0) {
        low = NULL;
        low_part_length = 0;
        uber = before_partition_array;
        uber_part_length = array_length;
    }

    if(id < partner) {
        MPI_Send(&uber_part_length, 1, MPI_INT, partner, 0, comm);
        MPI_Send(uber, uber_part_length, MPI_INT, partner, 0, comm);
        MPI_Recv(&recv_uber_part, 1, MPI_INT, partner, 0, comm, &status[id]);
        recv_uber = (int*)malloc(sizeof(int) * recv_uber_part);
        MPI_Recv(
            recv_uber, recv_uber_part, MPI_INT, partner, 0, comm, &status[id]);

        new_size = low_part_length + recv_uber_part;
        my_new_array = (int*)malloc(sizeof(int) * (new_size + 1));
        ++my_new_array;
        memcpy(my_new_array, low, sizeof(int) * low_part_length);
        memcpy(
            my_new_array + low_part_length, recv_uber,
            sizeof(int) * recv_uber_part);
        --my_new_array;
        my_new_array[0] = new_size;
    }
    else {
        MPI_Recv(&recv_low_part, 1, MPI_INT, partner, 0, comm, &status[id]);
        recv_low = (int*)malloc(sizeof(int) * recv_low_part);
        ++recv_low;
        MPI_Recv(
            recv_low, recv_low_part, MPI_INT, partner, 0, comm, &status[id]);
        MPI_Send(&low_part_length, 1, MPI_INT, partner, 0, comm);
        MPI_Send(low, low_part_length, MPI_INT, partner, 0, comm);

        new_size = uber_part_length + recv_low_part;
        my_new_array = (int*)malloc(sizeof(int) * (new_size + 1));
        ++my_new_array;
        memcpy(my_new_array, uber, sizeof(int) * uber_part_length);
        memcpy(
            my_new_array + uber_part_length, recv_low,
            sizeof(int) * recv_low_part);
        --my_new_array;
        my_new_array[0] = new_size;
    }

    MPI_Comm new_communicator;

    if(partner > id) {
        MPI_Comm_split(comm, 0, id, &new_communicator);
    }
    else {
        MPI_Comm_split(comm, 1, id, &new_communicator);
    }

    return hypersort(my_new_array, new_communicator);
}

int main(int argc, char* argv[]) {
    int id, size;
    int pivot = 0;
    int my_initial_part_size = 0;
    int initial_data_size = 0;
    int accumulator = 0;

    int parallel = 1;
    int suppress_output = 0;

    double long_start = 0;
    double long_finish = 0;

    int* a = NULL;
    int* before_partition = NULL;
    int* low = NULL;
    int* uber = NULL;
    int* sizes = NULL;
    int* displs = NULL;
    int* final_sizes = NULL;
    int* merge = NULL;

    char* filename = "data.dat";

    size_t int_bytes = sizeof(int);

    MPI_Init(&argc, &argv);

    if(argc > 2) {
        filename = argv[1];
        parallel = atoi(argv[2]);
    }

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &id);

    if((size & (size - 1)) != 0) {
        if(0 == id)
            std::cout << "Aborted: communicator size MUST BE power of two"
                      << std::endl;
        MPI_Finalize();
        return 0;
    }

    if(0 == id) {
        a = read(filename);
        initial_data_size = a[0];
        ++a;
    }

    if(!parallel) {
        if(0 == id) {
            auto t1 = MPI_Wtime();
            qsort(a, initial_data_size - 1);
            auto t2 = MPI_Wtime();
            for(size_t i = 0; i < initial_data_size; ++i)
                std::cout << a[i] << " ";
            std::cout << std::endl << "Elapsed time sequential: " << t2 - t1 << std::endl;
        }
        MPI_Barrier(comm);

        MPI_Finalize();
        return 0;
    }

    MPI_Bcast(&initial_data_size, 1, MPI_INT, 0, comm);

    displs = (int*)malloc(sizeof(int) * size);
    memset(displs, 0, sizeof(int) * size);
    if(0 == id) {
        sizes = distribution_deduction(comm, initial_data_size);
        displs[0] = 0;
        int counter = sizes[0];
        for(size_t k = 1; k < size; ++k) {
            if(sizes[k] == 0) {
                break;
            }
            displs[k] = counter;
            counter += sizes[k];
        }
    }
    else {
        a = (int*)malloc(sizeof(int) * initial_data_size);
        sizes = (int*)malloc(sizeof(int) * size);
    }

    MPI_Scatter(sizes, 1, MPI_INT, &my_initial_part_size, 1, MPI_INT, 0, comm);

    before_partition = (int*)malloc(sizeof(int) * my_initial_part_size + 1);

    MPI_Scatterv(
        a, sizes, displs, MPI_INT, ++before_partition, my_initial_part_size,
        MPI_INT, 0, comm);
    MPI_Barrier(comm);

    --before_partition;
    before_partition[0] = my_initial_part_size;

    int* sorted_part = hypersort(before_partition, comm);
    auto sz = sorted_part[0];
    ++sorted_part;

    auto t1 = MPI_Wtime();
    qsort(sorted_part, sz - 1);
    auto t2 = MPI_Wtime();

    MPI_Reduce(&t1, &long_start, 1, MPI_DOUBLE, MPI_MIN, 0, comm);
    MPI_Reduce(&t2, &long_finish, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

    merge = (int*)malloc(sizeof(int) * initial_data_size);
    final_sizes = (int*)malloc(sizeof(int) * size);
    MPI_Gather(&sz, 1, MPI_INT, final_sizes, 1, MPI_INT, 0, comm);
    int counter = final_sizes[0];
    for(size_t k = 1; k < size; ++k) {
        if(final_sizes[k] == 0) {
            displs[k] = -1;
            continue;
        }
        displs[k] = counter;
        counter += final_sizes[k];
    }
    MPI_Gatherv(
        sorted_part, sz, MPI_INT, merge, final_sizes, displs, MPI_INT, 0, comm);

    MPI_Barrier(comm);
    if(0 == id) {
        for(size_t i = 0; i < initial_data_size; ++i)
            std::cout << merge[i] << " ";
        std::cout << std::endl
                  << "Elapsed time parallel: " << long_finish - long_start
                  << std::endl;
    }

    MPI_Finalize();

    return 0;
}
