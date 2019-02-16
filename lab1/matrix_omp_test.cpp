#include "filesystem.hpp"
#include "matrix.hpp"

int main(int argc, char* argv[]) {
    // read from file
    std::stringstream m1_buf, m2_buf;
    if(argc == 6) {
        utils::read_from_file(argv[1], argv[2], m1_buf, m2_buf);
        std::string token;
        std::vector<double> matrix_n, matrix_m;
        utils::fill_matrix(m1_buf, matrix_n);
        utils::fill_matrix(m2_buf, matrix_m);
        auto schedule_t =
            static_cast<openmp_matrix::utils::schedule_type>(atoi(argv[3]));
        openmp_matrix::omp_matrix_test_vector<double>(
            matrix_n, matrix_m, schedule_t, atoi(argv[4]), atoi(argv[5]));
    }
    else if(argc == 4)
        openmp_matrix::utils::generate_data(
            argv[1], atoi(argv[2]), atoi(argv[3]));

    return 0;
}
