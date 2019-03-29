#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "numeric_traits.h"

// jacobi
// returns vector with result coeffs
// plus number of passed iterations
template<
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
std::vector<T> yakoby_main(
    std::vector<T>& input_matrix, std::vector<T>& free_ch, size_t iteration_num,
    double e = 0) {
    auto sz = free_ch.size();
    std::vector<T> prev;
    prev.resize(sz);
    std::fill(prev.begin(), prev.end(), 0);
    std::vector<T> curr;
    curr.resize(sz);
    std::fill(curr.begin(), curr.end(), 0);
    T norm, buff;
    for(size_t i = 0; i < iteration_num; ++i) {
        prev = curr;
        for(size_t j = 0; j < sz; ++j) {
            curr[j] = free_ch[j];
            for(size_t k = 0; k < sz; ++k) {
                if(j != k)
                    curr[j] -= prev[k] * input_matrix[j * sz + k];
            }
            curr[j] /= input_matrix[j * sz + j];
        }
        if(e != 0) {
            norm = fabs(prev[0] - curr[0]);
            for(size_t k = 0; k < sz; ++k) {
                buff = fabs(prev[k] - curr[k]);
                if(buff > norm)
                    norm = buff;
            }
            if(norm < e) {
                curr.push_back(i + 1);
                break;
            }
        }
    }
    return curr;
}
